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

#include "repository.h"

#include <qdir.h>
#include <qfile.h>
#include <qstring.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdirwatch.h>
#include <kstandarddirs.h>

#include "sshagent.h"


struct Repository::Private
{
    Private() : compressionLevel(0) {}

    QString     configFileName;

    QString     workingCopy;
    QString     location;

    QString     client;
    QString     rsh;
    QString     server;
    int         compressionLevel;

    void readConfig();
};



Repository::Repository()
    : QObject()
    , DCOPObject("CvsRepository")
    , d(new Private)
{
    // other cvsservice instances might change the configuration file
    // so we watch it for changes
    d->configFileName = locate("config", "cvsservicerc");
    KDirWatch* fileWatcher = new KDirWatch(this);
    connect(fileWatcher, SIGNAL(dirty(const QString&)),
            this, SLOT(slotConfigDirty(const QString&)));
    fileWatcher->addFile(d->configFileName);
}


Repository::Repository(const QString& repository)
    : QObject()
    , DCOPObject()
    , d(new Private)
{
    d->location = repository;
    d->readConfig();
    
    // other cvsservice instances might change the configuration file
    // so we watch it for changes
    d->configFileName = locate("config", "cvsservicerc");
    KDirWatch* fileWatcher = new KDirWatch(this);
    connect(fileWatcher, SIGNAL(dirty(const QString&)),
            this, SLOT(slotConfigDirty(const QString&)));
    fileWatcher->addFile(d->configFileName);
}


Repository::~Repository()
{
    delete d;
}


QString Repository::cvsClient() const
{
    QString client(d->client);

    // suppress reading of the '.cvsrc' file
    client += " -f";

    // we don't need the command line option if there is no compression level set
    if( d->compressionLevel > 0 )
    {
        client += " -z" + QString::number(d->compressionLevel) + " ";
    }

    return client;
}


QString Repository::rsh() const
{
    return d->rsh;
}


QString Repository::server() const
{
    return d->server;
}


bool Repository::setWorkingCopy(const QString& dirName)
{
    const QFileInfo fi(dirName);
    const QString path = fi.absFilePath();

    // is this really a cvs-controlled directory?
    const QFileInfo cvsDirInfo(path + "/CVS");
    if( !cvsDirInfo.exists() || !cvsDirInfo.isDir() )
        return false;

    d->workingCopy = path;
    d->location    = QString::null;

    // determine path to the repository
    QFile rootFile(path + "/CVS/Root");
    if( rootFile.open(IO_ReadOnly) ) 
    {
        QTextStream stream(&rootFile);
        d->location = stream.readLine();
    }
    rootFile.close();

    // add identities (ssh-add) to ssh-agent
    // TODO CL make sure this is called only once
    if( d->location.contains(":ext:", false) > 0 )
    {
        SshAgent ssh;
        ssh.addSshIdentities();
    }

    QDir::setCurrent(path);
    d->readConfig();

    return true;
}


QString Repository::workingCopy() const
{
    return d->workingCopy;
}


QString Repository::location() const
{
    return d->location;
}


void Repository::slotConfigDirty(const QString& fileName)
{
    if( fileName == d->configFileName )
    {
        // reread the configuration data from disk
        kapp->config()->reparseConfiguration();
        d->readConfig();
    }
}


void Repository::Private::readConfig()
{
    KConfig* config = kapp->config();
    config->setGroup(QString::fromLatin1("Repository-") + location);

    // see if there is a specific compression level set for this repository
    compressionLevel = config->readNumEntry("Compression", -1);

    // use default global compression level instead?
    if( compressionLevel < 0 )
    {
        KConfigGroupSaver cs(config, "General");
        compressionLevel = config->readNumEntry("Compression", 0);
    }

    // get remote shell client to access the remote repository
    rsh = config->readPathEntry("rsh");

    // get program to start on the server side
    server = config->readEntry("cvs_server");

    // get path to cvs client programm
    config->setGroup("General");
    client = config->readPathEntry("CVSPath", "cvs");   
}


#include "repository.moc"
