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

#include "svnrepository.h"

#include <qdir.h>
#include <qfile.h>
#include <qstring.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdirwatch.h>
#include <kstandarddirs.h>


struct SvnRepository::Private
{
    Private() : compressionLevel(0) {}

    QString     configFileName;

    QString     workingCopy;
    QString     location;

    QString     client;
    int         compressionLevel;

    void readConfig();
    void readGeneralConfig();
};



SvnRepository::SvnRepository()
    : QObject()
    , DCOPObject("SvnRepository")
    , d(new Private)
{
    d->readGeneralConfig();

    // other svnservice instances might change the configuration file
    // so we watch it for changes
    d->configFileName = locate("config", "svnservicerc");
    KDirWatch* fileWatcher = new KDirWatch(this);
    connect(fileWatcher, SIGNAL(dirty(const QString&)),
            this, SLOT(slotConfigDirty(const QString&)));
    fileWatcher->addFile(d->configFileName);
}


SvnRepository::SvnRepository(const QString& repository)
    : QObject()
    , DCOPObject()
    , d(new Private)
{
    d->location = repository;
    d->readGeneralConfig();
    d->readConfig();

    // other svnservice instances might change the configuration file
    // so we watch it for changes
    d->configFileName = locate("config", "svnservicerc");
    KDirWatch* fileWatcher = new KDirWatch(this);
    connect(fileWatcher, SIGNAL(dirty(const QString&)),
            this, SLOT(slotConfigDirty(const QString&)));
    fileWatcher->addFile(d->configFileName);
}


SvnRepository::~SvnRepository()
{
    delete d;
}


QString SvnRepository::svnClient() const
{
    QString client(d->client);

//     // we don't need the command line option if there is no compression level set
//     if( d->compressionLevel > 0 )
//     {
//         client += " -z" + QString::number(d->compressionLevel) + " ";
//     }

    return client;
}


bool SvnRepository::setWorkingCopy(const QString& dirName)
{
    const QFileInfo fi(dirName);
    const QString path = fi.absFilePath();

    // is this really a subversion-controlled directory?
    QFileInfo svnDirInfo(path + "/.svn");
    if( !svnDirInfo.exists() ||
        !svnDirInfo.isDir() ||
        !QFile::exists(svnDirInfo.filePath() + "/entries") )
        return false;

    d->workingCopy = path;
    d->location    = QString::null;

//     // determine path to the repository
//     QFile rootFile(path + "/CVS/Root");
//     if( rootFile.open(IO_ReadOnly) )
//     {
//         QTextStream stream(&rootFile);
//         d->location = stream.readLine();
//     }
//     rootFile.close();

    QDir::setCurrent(path);
    d->readConfig();

    return true;
}


QString SvnRepository::workingCopy() const
{
    return d->workingCopy;
}


QString SvnRepository::location() const
{
    return d->location;
}


void SvnRepository::slotConfigDirty(const QString& fileName)
{
    if( fileName == d->configFileName )
    {
        // reread the configuration data from disk
        kapp->config()->reparseConfiguration();
        d->readConfig();
    }
}


void SvnRepository::Private::readGeneralConfig()
{
    KConfig* config = kapp->config();

    // get path to cvs client programm
    config->setGroup("General");
    client = config->readPathEntry("SVNPath", "svn");
}


void SvnRepository::Private::readConfig()
{
    KConfig* config = kapp->config();

    QString repositoryGroup = QString::fromLatin1("Repository-") + location;
    config->setGroup(repositoryGroup);
}


#include "svnrepository.moc"
