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

#include "sandbox.h"

#include <qdir.h>
#include <qstring.h>
#include <qtextstream.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>


struct Sandbox::Private
{
    QString path;
    QString repository;

    bool    isOpen;
};


Sandbox::Sandbox()
    : d(new Private)
{
    d->isOpen = false;
}


Sandbox::~Sandbox()
{
    delete d;
}


bool Sandbox::open(const QString& dirName)
{
    QFileInfo fi(dirName);
    d->path = fi.absFilePath();

    // is this really a cvs-controlled directory?
    QFileInfo cvsDirInfo(d->path + "/CVS");
    if( !cvsDirInfo.exists() || !cvsDirInfo.isDir() )
        return false;

    // determine repository
    QFile rootFile(d->path + "/CVS/Root");
    if( rootFile.open(IO_ReadOnly) ) {
        QTextStream stream(&rootFile);
        d->repository = stream.readLine();
    }
    rootFile.close();

    QDir::setCurrent(d->path);

    kdDebug() << d->path << endl;
    kdDebug() << d->repository << endl;

    d->isOpen = true;

    return true;
}


bool Sandbox::isOpen() const
{
    return d->isOpen;
}


QString Sandbox::client() const
{
    KConfig* config = kapp->config();
    config->setGroup("General");

    return config->readEntry("CVSPath", "cvs") + " -f";
}


QString Sandbox::sandboxPath() const
{
    return d->path;
}


QString Sandbox::repository() const
{
    return d->repository;
}
