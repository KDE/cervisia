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

#include "svnservice.h"

#include <qintdict.h>

#include <dcopref.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>

#include "svnjob.h"
#include "svnrepository.h"


static const char SINGLE_JOB_ID[]   = "NonConcurrentJob";
static const char REDIRECT_STDERR[] = "2>&1";


struct SvnService::Private
{
    Private() : singleSvnJob(0), lastJobId(0), repository(0) {}
    ~Private()
    {
        delete repository;
        delete singleSvnJob;
    }

    SvnJob*               singleSvnJob;   // non-concurrent svn job, like update or commit
    DCOPRef               singleJobRef;   // DCOP reference to non-concurrent cvs job
    QIntDict<SvnJob>      svnJobs;        // concurrent svn jobs, like diff or annotate
    unsigned              lastJobId;

    QCString              appId;          // cache the DCOP clients app id

    SvnRepository*        repository;

    SvnJob* createSvnJob();
    DCOPRef setupNonConcurrentJob(SvnRepository* repo = 0);

    bool hasWorkingCopy();
    bool hasRunningJob();
};


static QString JoinFileList(const QStringList& files)
{
    QString result;

    QStringList::ConstIterator it  = files.begin();
    QStringList::ConstIterator end = files.end();

    for( ; it != end; ++it )
    {
        result += KProcess::quote(*it);
        result += " ";
    }

    if( result.length() > 0 )
        result.truncate(result.length()-1);

    return result;
}


SvnService::SvnService()
    : DCOPObject("SvnService")
    , d(new Private)
{
    d->appId = kapp->dcopClient()->appId();

    // create non-concurrent subversion job
    d->singleSvnJob = new SvnJob(SINGLE_JOB_ID);
    d->singleJobRef.setRef(d->appId, d->singleSvnJob->objId());

    // create repository manager
    d->repository = new SvnRepository();

    d->svnJobs.setAutoDelete(true);
}


SvnService::~SvnService()
{
    d->svnJobs.clear();
    delete d;
}


DCOPRef SvnService::add(const QStringList& files)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // svn add [FILES]
    d->singleSvnJob->clearCommand();

    *d->singleSvnJob << d->repository->svnClient() << "add"
                     << JoinFileList(files) << REDIRECT_STDERR;

    return d->setupNonConcurrentJob();
}


DCOPRef SvnService::commit(const QStringList& files, const QString& commitMessage,
                           bool recursive)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // svn commit [-N] [-m MESSAGE] [FILES]
    d->singleSvnJob->clearCommand();

    *d->singleSvnJob << d->repository->svnClient() << "commit";

    if( !recursive )
        *d->singleSvnJob << "-N";

    *d->singleSvnJob << "-m" << KProcess::quote(commitMessage)
                     << JoinFileList(files) << REDIRECT_STDERR;

    return d->setupNonConcurrentJob();
}


DCOPRef SvnService::remove(const QStringList& files)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // svn delete [FILES]
    d->singleSvnJob->clearCommand();

    *d->singleSvnJob << d->repository->svnClient() << "delete"
                     << JoinFileList(files) << REDIRECT_STDERR;

    return d->setupNonConcurrentJob();
}


void SvnService::quit()
{
    kapp->quit();
}


SvnJob* SvnService::Private::createSvnJob()
{
    ++lastJobId;

    // create a new subversion job
    SvnJob* job = new SvnJob(lastJobId);
    svnJobs.insert(lastJobId, job);

    job->setDirectory(repository->workingCopy());

    return job;
}


DCOPRef SvnService::Private::setupNonConcurrentJob(SvnRepository* repo)
{
    // no explicit repository provided?
    if( !repo )
        repo = repository;

    singleSvnJob->setDirectory(repo->workingCopy());

    return singleJobRef;
}


bool SvnService::Private::hasWorkingCopy()
{
    if( repository->workingCopy().isEmpty() )
    {
        KMessageBox::sorry(0, i18n("You have to set a local working copy "
                                   "directory before you can use this function!"));
        return false;
    }

    return true;
}


bool SvnService::Private::hasRunningJob()
{
    bool result = singleSvnJob->isRunning();

    if( result )
        KMessageBox::sorry(0, i18n("There is already a job running"));

    return result;
}
