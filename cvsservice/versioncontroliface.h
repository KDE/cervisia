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

#ifndef VERSIONCONTROLIFACE_H
#define VERSIONCONTROLIFACE_H

class DCOPRef;
class QString;
class QStringList;


class VersionControlInterface
{
public:
    virtual ~VersionControlInterface() {}

    /**
     * Adds new files to an existing project. The files don't actually
     * appear in the repository until a subsequent commit is performed.
     *
     * @param files A list of files that should be added to the repository.
     * @param isBinary
     *
     * @return A DCOP reference to the job or in case of failure a
     *         null reference.
     */
    virtual DCOPRef add(const QStringList& files, bool isBinary) = 0;

    /**
     * Shows information on who last modified each line of a file and when.
     *
     * @param fileName the name of the file to show annotations for
     * @param revision show annotations for this revision (number or tag)
     *
     * @return A DCOP reference to the job or in case of failure a
     *         null reference.
     */
    virtual DCOPRef annotate(const QString& fileName,
                             const QString& revision) = 0;

    /**
     * Checks out a module from the repository into a working copy.
     *
     * @param workingDir
     * @param repository
     * @param module
     * @param tag
     * @param pruneDirs remove empty directories from the working copy.
     *
     * @return A DCOP reference to the job or in case of failure a
     *         null reference.
     */
    virtual DCOPRef checkout(const QString& workingDir,
                             const QString& repository, const QString& module,
                             const QString& tag, bool pruneDirs) = 0;

    /**
     */
    virtual DCOPRef commit(const QStringList& files,
                           const QString& commitMessage, bool recursive) = 0;

    /**
     * Shows log messages for a file.
     *
     * @param fileName the name of the file to show log messages for
     *
     * @return A DCOP reference to the job or in case of failure a
     *         null reference.
     */
    virtual DCOPRef log(const QString& fileName) = 0;

    /**
     */
    DCOPRef login();

    /**
     */
    DCOPRef logout();

    /**
     */
    virtual DCOPRef remove(const QStringList& files, bool recursive) = 0;

    /**
     * Shows a summary of what's been done locally, without changing the
     * working copy. (cvs -n update)
     *
     * @param files
     * @param recursive descend into subdirectories.
     *
     * @return A DCOP reference to the job or in case of failure a
     *         null reference.
     */
    virtual DCOPRef status(const QStringList& files, bool recursive) = 0;

    /**
     * Shows the status of the files in the working copy.
     *
     * @param files
     * @param recursive descend into subdirectories.
     * @param tagInfo show tag information for the file.
     *
     * @return A DCOP reference to the job or in case of failure a
     *         null reference.
     */
    virtual DCOPRef status(const QStringList& files, bool recursive,
                           bool tagInfo) = 0;

    /**
     * Merges changes from the repository into the files of the
     * working copy.
     *
     * @param files
     * @param recursive descend into subdirectories.
     * @param createDirs create directories that exist in the repository
     *                   but not yet in the working copy.
     * @param pruneDirs remove empty directories from the working copy.
     * @param extraOpt
     *
     * @return A DCOP reference to the job or in case of failure a
     *         null reference.
     */
    virtual DCOPRef update(const QStringList& files, bool recursive,
                           bool createDirs, bool pruneDirs,
                           const QString& extraOpt) = 0;

    /**
     * Quits the DCOP service.
     */
    virtual void quit() = 0;
};


#endif
