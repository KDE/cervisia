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

#include "cvsservice.h"

#include <qdir.h>
#include <qintdict.h>
#include <qstring.h>

#include <dcopref.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kstandarddirs.h>

#include "cvsjob.h"
#include "repository.h"


static const char* SINGLE_JOB_ID = "NonConcurrentJob";

struct CvsService::Private
{
    CvsJob*             singleCvsJob;   // non-concurrent cvs job, like update or commit
    QIntDict<CvsJob>    cvsJobs;        // concurrent cvs jobs, like diff or annotate
    unsigned            lastJobId;
    
    QCString            appId;          // cache the DCOP clients app id
    QString             configFile;     // name of config file (including path)
    
    QString             workingCopy;
    Repository*         repository;
    
    CvsJob* createCvsJob();
};


CvsService::CvsService()
    : DCOPObject("CvsService")
    , d(new Private)
{
    // initialize private data
    d->lastJobId    = 0;
    d->repository   = 0;
    d->appId        = kapp->dcopClient()->appId();
    d->singleCvsJob = new CvsJob(SINGLE_JOB_ID);

    d->cvsJobs.setAutoDelete(true);

    // other cvsservice instances might change the configuration file
    // so we watch it for changes
    d->configFile = locate("config", "cvsservicerc");
    kdDebug() << d->configFile << endl;
    KDirWatch* fileWatcher = new KDirWatch(this);
    connect(fileWatcher, SIGNAL(dirty(const QString&)),
            this, SLOT(slotConfigDirty(const QString&)));
    fileWatcher->addFile(d->configFile);
}


CvsService::~CvsService()
{
    d->cvsJobs.clear();
    delete d->singleCvsJob;
    delete d->repository;
    delete d;
}


bool CvsService::setWorkingCopy(const QString& dirName)
{
    const QFileInfo fi(dirName);
    QString path = fi.absFilePath();
    
    // is this really a cvs-controlled directory?
    const QFileInfo cvsDirInfo(path + "/CVS");
    if( !cvsDirInfo.exists() || !cvsDirInfo.isDir() )
        return false;

    // delete old data
    delete d->repository;
    d->repository = 0;   

    d->workingCopy = path;
    d->repository = new Repository(path);
    
    QDir::setCurrent(path);
    
    return true;
}


QString CvsService::workingCopy() const
{
    return d->workingCopy;
}


QString CvsService::repository() const
{
    if( d->repository )
        return d->repository->location();
    else
        return QString::null;
}


DCOPRef CvsService::annotate(const QString& fileName, const QString& revision)
{
    if( !d->repository )
    {
        KMessageBox::sorry(0, i18n("You have to set a local working copy "
                                   "directory before you can use this function!"));
        return DCOPRef();
    }

    // create a cvs job
    CvsJob* job = d->createCvsJob();
    
    // Assemble the command line
    QString quotedName = KProcess::quote(fileName);
    QString cvsClient  = d->repository->cvsClient();

    *job << "(" << cvsClient << "log" << quotedName << "&&"
         << cvsClient << "annotate";
        
    if( !revision.isEmpty() )
        *job << "-r" << revision;

    // *Hack*
    // because the string "Annotations for blabla" is
    // printed to stderr even with option -Q.
    *job << quotedName << ") 2>&1";

    // return a DCOP reference to the cvs job
    return DCOPRef(d->appId, job->objId());
}


DCOPRef CvsService::log(const QString& fileName)
{
    if( !d->repository )
    {
        KMessageBox::sorry(0, i18n("You have to set a local working copy "
                                   "directory before you can use this function!"));
        return DCOPRef();
    }

    // create a cvs job
    CvsJob* job = d->createCvsJob();
    
    // Assemble the command line
    *job << d->repository->cvsClient() << "log" << KProcess::quote(fileName);

    // return a DCOP reference to the cvs job
    return DCOPRef(d->appId, job->objId());
}


DCOPRef CvsService::status(const QString& files, bool recursive)
{
    if( !d->repository )
    {
        KMessageBox::sorry(0, i18n("You have to set a local working copy "
                                   "directory before you can use this function!"));
        return DCOPRef();
    }

    if( d->singleCvsJob->isRunning() )
    {
        KMessageBox::sorry(0, i18n("There is already a job running"));
        return DCOPRef();
    }

    // Assemble the command line
    d->singleCvsJob->clearCvsCommand();
    
    *d->singleCvsJob << d->repository->cvsClient() << "-n update";

    if( !recursive )
        *d->singleCvsJob << "-l";

    *d->singleCvsJob << files << "2>&1";

    d->singleCvsJob->setRSH(d->repository->rsh());
    d->singleCvsJob->setServer(d->repository->server());
    d->singleCvsJob->setDirectory(d->workingCopy);

    return DCOPRef(d->appId, d->singleCvsJob->objId());
}


DCOPRef CvsService::status(const QString& files, bool recursive, bool tagInfo)
{
    if( !d->repository )
    {
        KMessageBox::sorry(0, i18n("You have to set a local working copy "
                                   "directory before you can use this function!"));
        return DCOPRef();
    }

    // create a cvs job
    CvsJob* job = d->createCvsJob();

    // Assemble the command line
    *job << d->repository->cvsClient() << "status";

    if( !recursive )
        *job << "-l";
    
    if( tagInfo )
        *job << "-v";

    *job << files;

    // return a DCOP reference to the cvs job
    return DCOPRef(d->appId, job->objId());
}


DCOPRef CvsService::update(const QString& files, bool recursive, 
                           bool createDirs, bool pruneDirs, const QString& extraOpt)
{
    if( !d->repository )
    {
        KMessageBox::sorry(0, i18n("You have to set a local working copy "
                                   "directory before you can use this function!"));
        return DCOPRef();
    }

    if( d->singleCvsJob->isRunning() )
    {
        KMessageBox::sorry(0, i18n("There is already a job running"));
        return DCOPRef();
    }

    // Assemble the command line
    d->singleCvsJob->clearCvsCommand();
    
    *d->singleCvsJob << d->repository->cvsClient() << "update";

    if( !recursive )
        *d->singleCvsJob << "-l";

    if( createDirs )
        *d->singleCvsJob << "-d";

    if( pruneDirs )
        *d->singleCvsJob << "-P";

    *d->singleCvsJob << extraOpt << files << "2>&1";

    d->singleCvsJob->setRSH(d->repository->rsh());
    d->singleCvsJob->setServer(d->repository->server());
    d->singleCvsJob->setDirectory(d->workingCopy);

    return DCOPRef(d->appId, d->singleCvsJob->objId());
}

                   
DCOPRef CvsService::checkout(const QString& workingDir, const QString& repository,
                             const QString& module, const QString& tag, bool pruneDirs)
{
    if( !d->repository )
    {
        KMessageBox::sorry(0, i18n("You have to set a local working copy "
                                   "directory before you can use this function!"));
        return DCOPRef();
    }

    if( d->singleCvsJob->isRunning() )
    {
        KMessageBox::sorry(0, i18n("There is already a job running"));
        return DCOPRef();
    }

    // Assemble the command line
    d->singleCvsJob->clearCvsCommand();
    
    *d->singleCvsJob << "cd" << workingDir << "&&"
                    << d->repository->cvsClient()
                    << "-d" << repository
                    << "checkout";
                    
    if( !tag.isEmpty() )
        *d->singleCvsJob << "-r" << tag;

    if( pruneDirs )
        *d->singleCvsJob << "-P";

    *d->singleCvsJob << module;

    d->singleCvsJob->setRSH(d->repository->rsh());
    d->singleCvsJob->setServer(d->repository->server());
    d->singleCvsJob->setDirectory(d->workingCopy);

    return DCOPRef(d->appId, d->singleCvsJob->objId());
}


void CvsService::quit()
{
    kapp->quit();
}


void CvsService::slotConfigDirty(const QString& fileName)
{
    if( d->repository && fileName == d->configFile )
    {
        // reread the configuration data from disk
        kapp->config()->reparseConfiguration();
        d->repository->updateConfig();
    }
}


CvsJob* CvsService::Private::createCvsJob()
{
    ++lastJobId;

    // create a cvs job
    CvsJob* job = new CvsJob(lastJobId);
    cvsJobs.insert(lastJobId, job);

    job->setRSH(repository->rsh());
    job->setServer(repository->server());
    job->setDirectory(workingCopy);

    return job;
}

#include "cvsservice.moc"
