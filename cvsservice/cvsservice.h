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

#ifndef CVSSERVICE_H
#define CVSSERVICE_H

#include <qstringlist.h>
#include <dcopref.h>
#include <dcopobject.h>

class QString;


class CvsService : public DCOPObject
{
    K_DCOP

public:
    CvsService();
    ~CvsService();

k_dcop:
    /**
     */
    DCOPRef add(const QStringList& files, bool isBinary);

    /**
     * Shows information on who last modified each line of a file and when.
     *
     * @param fileName the name of the file to show annotations for
     * @param revision show annotations for this revision (number or tag)
     *
     * @return A DCOP reference to the cvs job or in case of failure a
     *         null reference.
     */
    DCOPRef annotate(const QString& fileName, const QString& revision);
    
    /**
     * Checks out a module from the repository into a working copy.
     *
     * @param workingDir
     * @param repository
     * @param module
     * @param tag
     * @param pruneDirs remove empty directories from the working copy.
     *
     * @return A DCOP reference to the cvs job or in case of failure a
     *         null reference.
     */
    DCOPRef checkout(const QString& workingDir, const QString& repository,
                     const QString& module, const QString& tag, bool pruneDirs);

    /**
     */
    DCOPRef commit(const QStringList& files, const QString& commitMessage,
                   bool recursive);

    /**
     * Shows log messages for a file.
     *
     * @param fileName the name of the file to show log messages for
     *
     * @return A DCOP reference to the cvs job or in case of failure a
     *         null reference.
     */
    DCOPRef log(const QString& fileName);
    
    /**
     * Shows a summary of what's been done locally, without changing the
     * working copy. (cvs -n update)
     *
     * @param files
     * @param recursive descend into subdirectories.
     *
     * @return A DCOP reference to the cvs job or in case of failure a
     *         null reference.
     */    
    DCOPRef status(const QStringList& files, bool recursive);
    
    /**
     * Shows the status of the files in the working copy.
     *
     * @param files
     * @param recursive descend into subdirectories.
     * @param tagInfo show tag information for the file.
     *
     * @return A DCOP reference to the cvs job or in case of failure a
     *         null reference.
     */    
    DCOPRef status(const QStringList& files, bool recursive, bool tagInfo);
    
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
     * @return A DCOP reference to the cvs job or in case of failure a
     *         null reference.
     */
    DCOPRef update(const QStringList& files, bool recursive, bool createDirs,
                   bool pruneDirs, const QString& extraOpt);
                   
    /**
     * Quits the DCOP service.
     */
    void quit();

private:
    struct Private;
    Private* d;
};


#endif
