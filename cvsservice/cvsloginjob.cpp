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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include "cvsloginjob.h"

#include <kdebug.h>
#include <klocale.h>
#include <kpassdlg.h>

#include <sys/types.h>
#include <signal.h>

static const char LOGIN_PHRASE[]   = "Logging in to";
static const char FAILURE_PHRASE[] = "authorization failed:";
static const char PASS_PHRASE[]    = "CVS password: ";


CvsLoginJob::CvsLoginJob(unsigned jobNum)
    : DCOPObject()
    , m_Proc(0)
{
    QString objId("CvsLoginJob" + QString::number(jobNum));
    setObjId(objId.local8Bit());

    m_Proc = new PtyProcess;
}


CvsLoginJob::~CvsLoginJob()
{
    delete m_Proc;
}


void CvsLoginJob::setServer(const QString& server)
{
    m_Server = server;
}


void CvsLoginJob::setCvsClient(const QCString& cvsClient)
{
    m_CvsClient = cvsClient;

    m_Arguments.clear();
    m_Arguments += "-f";
}


void CvsLoginJob::setRepository(const QCString& repository)
{
    m_Arguments += "-d";
    m_Arguments += repository;
    m_Arguments += "login";
}


bool CvsLoginJob::execute()
{
    static QCString repository;

    int res = m_Proc->exec(m_CvsClient, m_Arguments);
    if( res < 0 )
    {
        kdDebug(8051) << "Couldn't start 'cvs login' process!" << endl;
        return false;
    }

    bool result = false;
    while( true )
    {
        QCString line = m_Proc->readLine();
        if( line.isNull() )
        {
            return result;
        }

        // add line to output list
        m_output << line;
        kdDebug(8051) << "process output = " << line << endl;

        // retrieve repository from 'Logging in to'-line
        if( line.contains(LOGIN_PHRASE) )
        {
            repository = line.remove(0, line.find(":pserver:"));
            continue;
        }

        // process asks for the password
        if( line.contains(PASS_PHRASE) )
        {
            kdDebug(8051) << "process waits for the password." << endl;

            // show password dialog
            // TODO: We really should display the repository name. Unfortunately
            //       the dialog doesn't show part of the repository name, because
            //       it's too long. :-(
            QCString password;
            int res = KPasswordDialog::getPassword(password, i18n("Please type "
                        "in your password for the repository below."));
            if( res == KPasswordDialog::Accepted )
            {
                // send password to process
                m_Proc->WaitSlave();
                m_Proc->writeLine(password);

                // wait for the result
                while( !line.contains(FAILURE_PHRASE) )
                {
                    line = m_Proc->readLine();
                    if( line.isNull() )
                        return true;

                    // add line to output list
                    m_output << line;
                    kdDebug(8051) << "process output = " << line << endl;
                }

                result = false;
            }
            else
            {
                // user pressed cancel so kill the process
                kill(m_Proc->pid(), SIGKILL);
                m_Proc->waitForChild();
                result = false;
            }
        }
    }

    return false;
}


QStringList CvsLoginJob::output()
{
    return m_output;
}
