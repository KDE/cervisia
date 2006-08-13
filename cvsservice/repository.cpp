/*
 * Copyright (c) 2002-2004 Christian Loose <christian.loose@kdemail.net>
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
    bool        retrieveCvsignoreFile;

    void readConfig();
    void readGeneralConfig();
};



Repository::Repository()
    : QObject()
    , DCOPObject("CvsRepository")
    , d(new Private)
{
    d->readGeneralConfig();

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
    d->readGeneralConfig();
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


QString Repository::clientOnly() const
{
    return d->client;
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
    if( !cvsDirInfo.exists() || !cvsDirInfo.isDir() ||
        !QFile::exists( cvsDirInfo.filePath() + "/Entries" ) ||
        !QFile::exists( cvsDirInfo.filePath() + "/Repository" ) ||
        !QFile::exists( cvsDirInfo.filePath() + "/Root" ) )
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


bool Repository::retrieveCvsignoreFile() const
{
    return d->retrieveCvsignoreFile;
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


void Repository::Private::readGeneralConfig()
{
    KConfig* config = kapp->config();

    // get path to cvs client programm
    config->setGroup("General");
    client = config->readPathEntry("CVSPath", "cvs");
}


void Repository::Private::readConfig()
{
    KConfig* config = kapp->config();

    // Sometimes the location can be unequal to the entry in the CVS/Root.
    //
    // This can happen when the checkout was done with a repository name
    // like :pserver:user@cvs.kde.org:/home/kde. When cvs then saves the
    // name into the .cvspass file, it adds the default cvs port to it like
    // this :pserver:user@cvs.kde.org:2401/home/kde. This name is then also
    // used for the configuration group.
    //
    // In order to be able to read this group, we then have to manually add
    // the port number to it.
    QString repositoryGroup = QString::fromLatin1("Repository-") + location;
    if( !config->hasGroup(repositoryGroup) )
    {
        // find the position of the first path separator
        const int insertPos = repositoryGroup.find('/');
        if( insertPos > 0 )
        {
            // add port to location
            // (1) :pserver:user@hostname.com:/path
            if( repositoryGroup.at(insertPos - 1) == ':' )
                repositoryGroup.insert(insertPos, "2401");
            // (2) :pserver:user@hostname.com/path
            else
                repositoryGroup.insert(insertPos, ":2401");
        }
    }

    config->setGroup(repositoryGroup);

    // should we retrieve the CVSROOT/cvsignore file from the cvs server?
    retrieveCvsignoreFile = config->readBoolEntry("RetrieveCvsignore", false);
    
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
}


#include "repository.moc"
