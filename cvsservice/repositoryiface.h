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

#ifndef REPOSITORYIFACE_H
#define REPOSITORYIFACE_H

class QString;


/**
 * Represents a local or remote repository with
 * its repository-specific configuration data.
 */
class RepositoryInterface
{
public:
    virtual ~RepositoryInterface() {}

    /**
     * Changes the working copy and the corresponding repository.
     *
     * @param dirName path to the local working copy directory.
     */
    virtual bool setWorkingCopy(const QString& dirName) = 0;

    /**
     * Path to the current working copy.
     *
     * @return The working copy directory. Can be null if not set.
     */
    virtual QString workingCopy() const = 0;

    /**
     * Path and method to access the current repository.
     *
     * @return The path and method to access the repository.
     */
    virtual QString location() const = 0;
};


#endif
