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

#include "repository.h"

#include <qfile.h>
#include <qstring.h>
#include <kapplication.h>
#include <kconfig.h>


struct Repository::Private
{
    QString client;
    QString location;
    QString rsh;
    int     compressionLevel;

    void readConfig();
};


void Repository::Private::readConfig()
{
    KConfig* config = kapp->config();
    config->setGroup(QString::fromLatin1("Repository-") + location);

    // see if there is a specific compression level set for this repository
    compressionLevel = config->readNumEntry("Compression", -1);

    // use default global compression level instead?
    if( compressionLevel < 0 )
    {
        config->setGroup("General");
        compressionLevel = config->readNumEntry("Compression", 0);
    }

    // get shell command to access the remote repository
    rsh = config->readEntry("rsh", QString());
    
    // get path to cvs client programm
    config->setGroup("General");
    client = config->readEntry("CVSPath", "cvs");   
}



Repository::Repository(const QString& workingCopyDir)
    : d(new Private)
{
    // initialize private data
    d->compressionLevel = 0;

    // determine path to the repository
    QFile rootFile(workingCopyDir + "/CVS/Root");
    if( rootFile.open(IO_ReadOnly) ) 
    {
        QTextStream stream(&rootFile);
        d->location = stream.readLine();
    }
    rootFile.close();

    d->readConfig();
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


void Repository::updateConfig()
{
    d->readConfig();
}


QString Repository::location() const
{
    return d->location;
}
