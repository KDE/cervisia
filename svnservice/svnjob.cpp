/*
 * Copyright (c) 2005 Christian Loose <christian.loose@kdemail.net>
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

#include "svnjob.h"

#include <qfile.h>
#include <kdebug.h>
#include <kprocess.h>


struct SvnJob::Private
{
    Private() : isRunning(false)
    {
        childproc = new KProcess;
        childproc->setUseShell(true, "/bin/sh");
    }
    ~Private() { delete childproc; }

    KProcess*   childproc;
    QString     directory;
    bool        isRunning;
    QStringList outputLines;
};


SvnJob::SvnJob(unsigned jobNum)
    : QObject()
    , DCOPObject()
    , d(new Private)
{
    QString objId("SvnJob" + QString::number(jobNum));
    setObjId(objId.local8Bit());
}


SvnJob::SvnJob(const QString& objId)
    : QObject()
    , DCOPObject()
    , d(new Private)
{
    setObjId(objId.local8Bit());
}


SvnJob::~SvnJob()
{
    delete d;
}


void SvnJob::clearCommand()
{
    d->childproc->clearArguments();
}


void SvnJob::setDirectory(const QString& directory)
{
    d->directory = directory;
}


bool SvnJob::isRunning() const
{
    return d->isRunning;
}


SvnJob& SvnJob::operator<<(const QString& arg)
{
    *d->childproc << arg;
    return *this;
}


SvnJob& SvnJob::operator<<(const char* arg)
{
    *d->childproc << arg;
    return *this;
}


SvnJob& SvnJob::operator<<(const QCString& arg)
{
    *d->childproc << arg;
    return *this;
}


SvnJob& SvnJob::operator<<(const QStringList& args)
{
    *d->childproc << args;
    return *this;
}


QString SvnJob::command() const
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


QStringList SvnJob::output() const
{
    return d->outputLines;
}


bool SvnJob::execute()
{
    if( !d->directory.isEmpty() )
        d->childproc->setWorkingDirectory(d->directory);

    connect(d->childproc, SIGNAL(processExited(KProcess*)),
        SLOT(slotProcessExited()));
    connect(d->childproc, SIGNAL(receivedStdout(KProcess*, char*, int)),
        SLOT(slotReceivedStdout(KProcess*, char*, int)));
    connect(d->childproc, SIGNAL(receivedStderr(KProcess*, char*, int)),
        SLOT(slotReceivedStderr(KProcess*, char*, int)) );

    kdDebug(8051) << "Execute svn command: " << command() << endl;

    d->isRunning = true;
    return d->childproc->start(KProcess::NotifyOnExit, KProcess::AllOutput);
}


void SvnJob::cancel()
{
    d->childproc->kill();
}


void SvnJob::slotProcessExited()
{
    // disconnect all connections to childproc's signals
    d->childproc->disconnect();
    d->childproc->clearArguments();

    d->isRunning = false;

    emit jobExited(d->childproc->normalExit(), d->childproc->exitStatus());
}


void SvnJob::slotReceivedStdout(KProcess* proc, char* buffer, int buflen)
{
    Q_UNUSED(proc);

    QString output = QString::fromLocal8Bit(buffer, buflen);

    // accumulate output
    d->outputLines += QStringList::split("\n", output);

    emit receivedStdout(output);
}


void SvnJob::slotReceivedStderr(KProcess* proc, char* buffer, int buflen)
{
    Q_UNUSED(proc);

    QString output = QString::fromLocal8Bit(buffer, buflen);

    // accumulate output
    d->outputLines += QStringList::split("\n", output);

    emit receivedStderr(output);
}

#include "svnjob.moc"
