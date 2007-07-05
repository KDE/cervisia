/*
 * Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "sshagent.h"

#include <qregexp.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <k3process.h>

#include <stdlib.h>
#include <signal.h>


// initialize static member variables
bool    SshAgent::m_isRunning  = false;
bool    SshAgent::m_isOurAgent = false;
QString SshAgent::m_authSock   = QString::null;
QString SshAgent::m_pid        = QString::null;


SshAgent::SshAgent(QObject* parent, const char* name)
    : QObject(parent, name)
{
}


SshAgent::~SshAgent()
{
}


bool SshAgent::querySshAgent()
{
    kDebug(8051) << "SshAgent::querySshAgent(): ENTER" << endl;

    if( m_isRunning )
        return true;

    // Did the user already start a ssh-agent process?
    char* pid;
    if( (pid = ::getenv("SSH_AGENT_PID")) != 0 )
    {
        kDebug(8051) << "SshAgent::querySshAgent(): ssh-agent already exists"
                      << endl;

        m_pid = QString::fromLocal8Bit(pid);

        char* sock = ::getenv("SSH_AUTH_SOCK");
        if( sock )
            m_authSock = QString::fromLocal8Bit(sock);

        m_isOurAgent = false;
        m_isRunning  = true;
    }
    // We have to start a new ssh-agent process
    else
    {
        kDebug(8051) << "SshAgent::querySshAgent(): start ssh-agent" << endl;

        m_isOurAgent = true;
        m_isRunning  = startSshAgent();
    }

    return m_isRunning;
}


bool SshAgent::addSshIdentities()
{
    kDebug(8051) << "SshAgent::addSshIdentities(): ENTER" << endl;

    if( !m_isRunning || !m_isOurAgent )
        return false;

    // add identities to ssh-agent
    K3Process proc;

    proc.setEnvironment("SSH_AGENT_PID", m_pid);
    proc.setEnvironment("SSH_AUTH_SOCK", m_authSock);
    proc.setEnvironment("SSH_ASKPASS", "cvsaskpass");

    proc << "ssh-add";

    connect(&proc, SIGNAL(receivedStdout(K3Process*, char*, int)),
            SLOT(slotReceivedStdout(K3Process*, char*, int)));
    connect(&proc, SIGNAL(receivedStderr(K3Process*, char*, int)),
            SLOT(slotReceivedStderr(K3Process*, char*, int)));

    proc.start(K3Process::DontCare, K3Process::AllOutput);

    // wait for process to finish
    // TODO CL use timeout?
    proc.wait();

    kDebug(8051) << "SshAgent::slotProcessExited(): added identities" << endl;

    return (proc.normalExit() && proc.exitStatus() == 0);
}


void SshAgent::killSshAgent()
{
    kDebug(8051) << "SshAgent::killSshAgent(): ENTER" << endl;

    if( !m_isRunning || !m_isOurAgent )
        return;

    kill(m_pid.toInt(), SIGTERM);

    kDebug(8051) << "SshAgent::killSshAgent(): killed pid = " << m_pid << endl;
}


void SshAgent::slotProcessExited(K3Process*)
{
    kDebug(8051) << "SshAgent::slotProcessExited(): ENTER" << endl;

    QRegExp cshPidRx("setenv SSH_AGENT_PID (\\d*);");
    QRegExp cshSockRx("setenv SSH_AUTH_SOCK (.*);");

    QRegExp bashPidRx("SSH_AGENT_PID=(\\d*).*");
    QRegExp bashSockRx("SSH_AUTH_SOCK=(.*\\.\\d*);.*");

    QStringList::Iterator it  = m_outputLines.begin();
    QStringList::Iterator end = m_outputLines.end();
    for( ; it != end; ++it )
    {
        if( m_pid.isEmpty() )
        {
            int pos = cshPidRx.search(*it);
            if( pos > -1 )
            {
                m_pid = cshPidRx.cap(1);
                continue;
            }

            pos = bashPidRx.search(*it);
            if( pos > -1 )
            {
                m_pid = bashPidRx.cap(1);
                continue;
            }
        }

        if( m_authSock.isEmpty() )
        {
            int pos = cshSockRx.search(*it);
            if( pos > -1 )
            {
                m_authSock = cshSockRx.cap(1);
                continue;
            }

            pos = bashSockRx.search(*it);
            if( pos > -1 )
            {
                m_authSock = bashSockRx.cap(1);
                continue;
            }
        }
    }

    kDebug(8051) << "SshAgent::slotProcessExited(): pid = " << m_pid
                  << ", socket = " << m_authSock << endl;
}


void SshAgent::slotReceivedStdout(K3Process* proc, char* buffer, int buflen)
{
    Q_UNUSED(proc);

    QString output = QString::fromLocal8Bit(buffer, buflen);
    m_outputLines += QStringList::split("\n", output);

    kDebug(8051) << "SshAgent::slotReceivedStdout(): output = " << output << endl;
}


void SshAgent::slotReceivedStderr(K3Process* proc, char* buffer, int buflen)
{
    Q_UNUSED(proc);

    QString output = QString::fromLocal8Bit(buffer, buflen);
    m_outputLines += QStringList::split("\n", output);

    kDebug(8051) << "SshAgent::slotReceivedStderr(): output = " << output << endl;
}


bool SshAgent::startSshAgent()
{
    kDebug(8051) << "SshAgent::startSshAgent(): ENTER" << endl;

    K3Process proc;

    proc << "ssh-agent";

    connect(&proc, SIGNAL(processExited(K3Process*)),
            SLOT(slotProcessExited(K3Process*)));
    connect(&proc, SIGNAL(receivedStdout(K3Process*, char*, int)),
            SLOT(slotReceivedStdout(K3Process*, char*, int)));
    connect(&proc, SIGNAL(receivedStderr(K3Process*, char*, int)),
            SLOT(slotReceivedStderr(K3Process*, char*, int)) );

    proc.start(K3Process::NotifyOnExit, K3Process::All);

    // wait for process to finish
    // TODO CL use timeout?
    proc.wait();

    return (proc.normalExit() && proc.exitStatus() == 0);
}


#include "sshagent.moc"
