/*
 * Copyright (c) 2002 Christian Loose <christian.loose@hamburg.de>
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

#include <qstring.h>
#include <kprocess.h>


struct CvsJob::Private
{
    KProcess*   childproc;
    QString     cvsCommand;
    QString     rsh;
    QString     directory;
    bool        isRunning;
};


CvsJob::CvsJob(unsigned jobId, const QString& cvsCommand, const QString& rsh,
               const QString& directory)
    : QObject()
    , DCOPObject()
    , d(new Private)
{
    // initialize private data
    d->cvsCommand = cvsCommand;
    d->rsh        = rsh;
    d->directory  = directory;
    d->isRunning  = false;

    QString objId("CvsJob" + QString::number(jobId));
    setObjId(objId.local8Bit());

    d->childproc = new KProcess;
    d->childproc->setUseShell(true, "/bin/sh");
}


CvsJob::~CvsJob()
{
    delete d->childproc;
    delete d;
}


void CvsJob::setCvsCommand(const QString& cvsCommand)
{
    d->cvsCommand = cvsCommand;
}


void CvsJob::setRSH(const QString& rsh)
{
    d->rsh = rsh;
}


void CvsJob::setDirectory(const QString& directory)
{
    d->directory = directory;
}


bool CvsJob::isRunning() const
{
    return d->isRunning;
}


QString CvsJob::cvsCommand() const
{
    return d->cvsCommand;
}


bool CvsJob::execute()
{
    if( !d->rsh.isEmpty() )
        d->childproc->setEnvironment("CVS_RSH", d->rsh);

    if( !d->directory.isEmpty() )
        d->childproc->setWorkingDirectory(d->directory);
        
    *d->childproc << d->cvsCommand;

    connect(d->childproc, SIGNAL(processExited(KProcess*)),
        SLOT(slotProcessExited()));
    connect(d->childproc, SIGNAL(receivedStdout(KProcess*, char*, int)),
        SLOT(slotReceivedStdout(KProcess*, char*, int)));
    connect(d->childproc, SIGNAL(receivedStderr(KProcess*, char*, int)),
        SLOT(slotReceivedStderr(KProcess*, char*, int)) );

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

    emit receivedStdout(output);
}


void CvsJob::slotReceivedStderr(KProcess* proc, char* buffer, int buflen)
{
    Q_UNUSED(proc);

    QString output = QString::fromLocal8Bit(buffer, buflen);

    emit receivedStderr(output);
}

#include "cvsjob.moc"
