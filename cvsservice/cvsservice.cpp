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


struct CvsService::Private
{
    CvsJob*             singleCvsJob;   // non-concurrent cvs job, like update or commit
    QIntDict<CvsJob>    cvsJobs;        // concurrent cvs jobs, like diff or annotate
    unsigned            lastJobId;
    
    QCString            appId;          // cache the DCOP clients app id
    QString             configFile;     // name of config file (including path)
    
    QString             workingCopy;
    Repository*         repository;
    
    DCOPRef createCvsJob(const QString& cmdline);
};


CvsService::CvsService()
    : DCOPObject("CvsService")
    , d(new Private)
{
    // initialize private data
    d->lastJobId  = 0;
    d->repository = 0;
    d->appId      = kapp->dcopClient()->appId();

    ++d->lastJobId;
    d->singleCvsJob = new CvsJob(d->lastJobId, "");

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
    QFileInfo fi(dirName);
    QString path = fi.absFilePath();
    
    // is this really a cvs-controlled directory?
    QFileInfo cvsDirInfo(path + "/CVS");
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

    QString quotedName = KProcess::quote(fileName);
    QString cvsClient  = d->repository->cvsClient();

    // Assemble the command line
    QString cmdline = "( " + cvsClient;
    cmdline += " log ";
    cmdline += quotedName;
    cmdline += " && ";
    cmdline += cvsClient;
    cmdline += " annotate ";

    if( !revision.isEmpty() )
        cmdline += " -r " + revision;

    cmdline += " ";
    cmdline += quotedName;
    // *Hack*
    // because the string "Annotations for blabla" is
    // printed to stderr even with option -Q.
    cmdline += " ) 2>&1";

    // create a cvs job
    return d->createCvsJob(cmdline);
}


DCOPRef CvsService::log(const QString& fileName)
{
    if( !d->repository )
    {
        KMessageBox::sorry(0, i18n("You have to set a local working copy "
                                   "directory before you can use this function!"));
        return DCOPRef();
    }

    // Assemble the command line
    QString cmdline = d->repository->cvsClient();
    cmdline += " log ";
    cmdline += KProcess::quote(fileName);

    // create a cvs job
    return d->createCvsJob(cmdline);
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
    QString cmdline = d->repository->cvsClient();
    cmdline += " -n update ";

    if( !recursive )
        cmdline += "-l ";

    cmdline += files;
    cmdline += " 2>&1";

    // create a cvs job
    return d->createCvsJob(cmdline);
}


DCOPRef CvsService::status(const QString& files, bool recursive, bool tagInfo)
{
    if( !d->repository )
    {
        KMessageBox::sorry(0, i18n("You have to set a local working copy "
                                   "directory before you can use this function!"));
        return DCOPRef();
    }

    // Assemble the command line
    QString cmdline = d->repository->cvsClient();
    cmdline += " status ";

    if( !recursive )
        cmdline += "-l ";
    
    if( tagInfo )
        cmdline += "-v ";

    cmdline += files;

    d->singleCvsJob->setCvsCommand(cmdline);
    d->singleCvsJob->setRSH(d->repository->rsh());
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


DCOPRef CvsService::Private::createCvsJob(const QString& cmdline)
{
    ++lastJobId;

    // create a cvs job
    CvsJob* job = new CvsJob(lastJobId, cmdline, repository->rsh(),
                             workingCopy);
    cvsJobs.insert(lastJobId, job);

    return DCOPRef(appId, job->objId());
}

#include "cvsservice.moc"
