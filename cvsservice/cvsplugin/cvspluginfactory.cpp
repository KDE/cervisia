/*
 * Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
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

#include "cvspluginfactory.h"

#include <qfileinfo.h>

#include "cvsrepository.h"


CvsPluginFactory::CvsPluginFactory(QObject* parent, const char* name,
                                   const QStringList& args)
    : VcsPluginFactory(parent, name, args)
{
}


bool CvsPluginFactory::canHandleDirectory(const QString& dirName)
{
    const QFileInfo fi(dirName);
    const QString path = fi.absFilePath();
    
    // is this a cvs-controlled directory?
    const QFileInfo cvsDirInfo(path + "/CVS");
    
    return (cvsDirInfo.exists() && cvsDirInfo.isDir());
}


RepositoryInterface* CvsPluginFactory::repository()
{
    return new CvsRepository;
}


VersionControlInterface* CvsPluginFactory::versionControlSystem()
{
#warning FIXME: Not Yet Implemented!
    return 0;
}
