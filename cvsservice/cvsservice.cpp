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

#include <qintdict.h>
#include <qstring.h>
#include <dcopref.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include "cvsjob.h"
#include "sandbox.h"


struct CvsService::Private
{
    CvsJob*             singleCvsJob;   // non-concurrent cvs job, like update or commit
    QIntDict<CvsJob>    cvsJobs;        // concurrent cvs jobs, like diff or annotate
    unsigned            lastJobId;
    Sandbox             sandbox;
    QCString            appId;          // cache the DCOP clients app id
};


CvsService::CvsService()
    : DCOPObject("CvsService")
    , d(new Private)
{
    d->lastJobId = 0;

    ++d->lastJobId;
    d->singleCvsJob = new CvsJob(d->lastJobId, "");

    d->cvsJobs.setAutoDelete(true);

    d->appId = kapp->dcopClient()->appId();
}


CvsService::~CvsService()
{
    d->cvsJobs.clear();
    delete d->singleCvsJob;
    delete d;
}


DCOPRef CvsService::annotate(const QString& fileName, const QString& revision)
{
    QString quotedName = KProcess::quote(fileName);
    QString cmdline = "( " + d->sandbox.client() + " log ";
    cmdline += quotedName;
    cmdline += " && ";
    cmdline += d->sandbox.client() + " annotate ";

    if( !revision.isEmpty() )
        cmdline += " -r " + revision;

    cmdline += " " + quotedName;
    // Hack because the string ´Annotations for blabla´ is
    // printed to stderr even with option -Q. Arg!
    cmdline += " ) 2>&1";

    ++d->lastJobId;
    CvsJob* job = new CvsJob(d->lastJobId, cmdline);
    d->cvsJobs.insert(d->lastJobId, job);

    return DCOPRef(d->appId, job->objId());
}


DCOPRef CvsService::status(const QString& files, bool recursive, 
                           bool createDirs, bool pruneDirs)
{
    if( !d->sandbox.isOpen() ) {
        KMessageBox::sorry(0, i18n("No open sandbox"), "CvsService");
        return DCOPRef();
    }

    if( d->singleCvsJob->isRunning() ) {
        KMessageBox::sorry(0, i18n("There is already a job running"),
                           "CvsService");
        return DCOPRef();
    }

    QString cmdline = d->sandbox.client() + " -n update ";

    if( recursive )
        cmdline += "-R ";
    else
        cmdline += "-l ";

    if( createDirs )
        cmdline += "-d ";

    if( pruneDirs )
        cmdline += "-P ";

    cmdline += files;
    cmdline += " 2>&1";

    d->singleCvsJob->setCvsCommand(cmdline);

    kdDebug() << cmdline << endl;

    return DCOPRef(d->appId, d->singleCvsJob->objId());
}


bool CvsService::openSandbox(const QString& dirName)
{
    return d->sandbox.open(dirName);
}


QString CvsService::sandbox() const
{
    return d->sandbox.sandboxPath();
}


QString CvsService::repository() const
{
    return d->sandbox.repository();
}


void CvsService::quit()
{
    kapp->quit();
}
