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

#ifndef VCSPLUGINFACTORY_H
#define VCSPLUGINFACTORY_H

#include <qobject.h>

class QString;
class QStringList;
class RepositoryInterface;
class VersionControlInterface;


class VcsPluginFactory : public QObject
{
public:
    VcsPluginFactory(QObject* parent, const char* name, const QStringList&)
        : QObject(parent, name)
    {
    }
    virtual ~VcsPluginFactory() {}

    /**
     * Returns true when this plugin is able to handle the
     * passed working copy directory.
     */
    virtual bool canHandleDirectory(const QString& dirName) = 0;

    virtual RepositoryInterface* repository() = 0;
    virtual VersionControlInterface* versionControlSystem() = 0;
};


#endif
