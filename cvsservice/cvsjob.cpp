/*
 * Copyright (c) 2002-2003 Christian Loose <christian.loose@hamburg.de>
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include "cvsjob.h"

#include <qfile.h>
#include <kdebug.h>
#include <kprocess.h>

#include "sshagent.h"


struct CvsJob::Private
{
    Private() : isRunning(false)
    {
        childproc = new KProcess;
        childproc->setUseShell(true, "/bin/sh");
    }
    ~Private() { delete childproc; }

    KProcess*   childproc;
    QString     server;
    QString     rsh;
    QString     directory;
    bool        isRunning;
    QStringList outputLines;
};


CvsJob::CvsJob(unsigned jobNum)
    : QObject()
    , DCOPObject()
    , d(new Private)
{
    QString objId("CvsJob" + QString::number(jobNum));
    setObjId(objId.local8Bit());
}


CvsJob::CvsJob(const QString& objId)
    : QObject()
    , DCOPObject()
    , d(new Private)
{
    setObjId(objId.local8Bit());
}


CvsJob::~CvsJob()
{
    delete d;
}


void CvsJob::clearCvsCommand()
{
    d->childproc->clearArguments();
}


void CvsJob::setRSH(const QString& rsh)
{
    d->rsh = rsh;
}


void CvsJob::setServer(const QString& server)
{
    d->server = server;
}


void CvsJob::setDirectory(const QString& directory)
{
    d->directory = directory;
}


bool CvsJob::isRunning() const
{
    return d->isRunning;
}


CvsJob& CvsJob::operator<<(const QString& arg)
{
    *d->childproc << arg;
    return *this;
}


CvsJob& CvsJob::operator<<(const char* arg)
{
    *d->childproc << arg;
    return *this;
}


CvsJob& CvsJob::operator<<(const QCString& arg)
{
    *d->childproc << arg;
    return *this;
}


CvsJob& CvsJob::operator<<(const QStringList& args)
{
    *d->childproc << args;
    return *this;
}


QString CvsJob::cvsCommand() const
{
    QString command;

    const QValueList<QCString>& args(d->childproc->args());
    for (QValueList<QCString>::const_iterator it = args.begin(), itEnd = args.end();
         it != itEnd; ++it)
    {
        if (!command.isEmpty())
            command += ' ';

        command += QFile::decodeName(*it);
    }

    return command;
}


QStringList CvsJob::output() const
{
    return d->outputLines;
}


bool CvsJob::execute()
{
    // setup job environment to use the ssh-agent (if it is running)
    SshAgent ssh;
    if( !ssh.pid().isEmpty() )
    {
        // kdDebug(8051) << "PID  = " << ssh.pid() << endl;
        // kdDebug(8051) << "SOCK = " << ssh.authSock() << endl;

        d->childproc->setEnvironment("SSH_AGENT_PID", ssh.pid());
        d->childproc->setEnvironment("SSH_AUTH_SOCK", ssh.authSock());
    }
    
    d->childproc->setEnvironment("SSH_ASKPASS", "cvsaskpass");

    if( !d->rsh.isEmpty() )
        d->childproc->setEnvironment("CVS_RSH", d->rsh);

    if( !d->server.isEmpty() )
        d->childproc->setEnvironment("CVS_SERVER", d->server);

    if( !d->directory.isEmpty() )
        d->childproc->setWorkingDirectory(d->directory);

    connect(d->childproc, SIGNAL(processExited(KProcess*)),
        SLOT(slotProcessExited()));
    connect(d->childproc, SIGNAL(receivedStdout(KProcess*, char*, int)),
        SLOT(slotReceivedStdout(KProcess*, char*, int)));
    connect(d->childproc, SIGNAL(receivedStderr(KProcess*, char*, int)),
        SLOT(slotReceivedStderr(KProcess*, char*, int)) );

    kdDebug(8051) << "Execute cvs command: " << cvsCommand() << endl;

    d->isRunning = true;
    return d->childproc->start(KProcess::NotifyOnExit, KProcess::AllOutput);
}


void CvsJob::cancel()
{
    d->childproc->kill();
}


void CvsJob::slotProcessExited()
{
    // disconnect all connections to childproc's signals
    d->childproc->disconnect();
    d->childproc->clearArguments();

    d->isRunning = false;

    emit jobExited(d->childproc->normalExit(), d->childproc->exitStatus());
}


void CvsJob::slotReceivedStdout(KProcess* proc, char* buffer, int buflen)
{
    Q_UNUSED(proc);

    QString output = QString::fromLocal8Bit(buffer, buflen);

    // accumulate output
    d->outputLines += QStringList::split("\n", output);

    emit receivedStdout(output);
}


void CvsJob::slotReceivedStderr(KProcess* proc, char* buffer, int buflen)
{
    Q_UNUSED(proc);

    QString output = QString::fromLocal8Bit(buffer, buflen);

    // accumulate output
    d->outputLines += QStringList::split("\n", output);

    emit receivedStderr(output);
}

#include "cvsjob.moc"
