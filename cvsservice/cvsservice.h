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

#ifndef CVSSERVICE_H
#define CVSSERVICE_H

#include <qstringlist.h>
#include <dcopref.h>
#include <dcopobject.h>

class QString;


class KDE_EXPORT CvsService : public DCOPObject
{
    K_DCOP

public:
    CvsService();
    ~CvsService();

k_dcop:
    /**
     * Adds new files to an existing project. The files don't actually
     * appear in the repository until a subsequent commit is performed.
     *
     * @param files A list of files that should be added to the repository.
     * @param isBinary Set to true to treat the files as binary files (-kb)
     *
     * @return A DCOP reference to the cvs job or in case of failure a
     *         null reference.
     */
    DCOPRef add(const QStringList& files, bool isBinary);

    /**
     */
    DCOPRef addWatch(const QStringList& files, int events);

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
     * @param workingDir path to a local working copy directory
     * @param repository
     * @param module the name of the module
     * @param tag
     * @param pruneDirs remove empty directories from the working copy.
     *
     * @return A DCOP reference to the cvs job or in case of failure a
     *         null reference.
     */
    DCOPRef checkout(const QString& workingDir, const QString& repository,
                     const QString& module, const QString& tag, bool pruneDirs);
    
    /**
     * Checks out a module from the repository into a working copy.
     *
     * @param workingDir path to a local working copy directory
     * @param repository
     * @param module the name of the module
     * @param tag
     * @param pruneDirs remove empty directories from the working copy.
     * @param alias alternative directory to check out to
     * @param exportOnly flag to show we want a cvs export rather than a checkout
     *
     * @return A DCOP reference to the cvs job or in case of failure a
     *         null reference.
     */
    //### KDE4: merge with above checkout() method
    DCOPRef checkout(const QString& workingDir, const QString& repository,
                     const QString& module, const QString& tag, bool pruneDirs, 
                     const QString& alias, bool exportOnly);

    /**
     * Checks out a module from the repository into a working copy.
     *
     * @param workingDir path to a local working copy directory
     * @param repository
     * @param module the name of the module
     * @param tag
     * @param pruneDirs remove empty directories from the working copy.
     * @param alias alternative directory to check out to
     * @param exportOnly flag to show we want a cvs export rather than a checkout
     * @param recursive check out dirs recursively
     *
     * @return A DCOP reference to the cvs job or in case of failure a
     *         null reference.
     */
    DCOPRef checkout(const QString& workingDir, const QString& repository,
                     const QString& module, const QString& tag, bool pruneDirs, 
                     const QString& alias, bool exportOnly, bool recursive);

    /**
     *
     * @param files A list of files with changes that should be committed to
     *              the repository.
     * @param commitMessage log message describing the changes
     * @param recursive
     *
     * @return A DCOP reference to the cvs job or in case of failure a
     *         null reference.
     */
    DCOPRef commit(const QStringList& files, const QString& commitMessage,
                   bool recursive);

    /**
     * Creates a new root repository.
     *
     * @param repository
     */
    DCOPRef createRepository(const QString& repository);
    
    /**
     */
    DCOPRef createTag(const QStringList& files, const QString& tag,
                      bool branch, bool force);

    /**
     */
    DCOPRef deleteTag(const QStringList& files, const QString& tag,
                      bool branch, bool force);

    /**
     */
    DCOPRef downloadCvsIgnoreFile(const QString& repository,
                                  const QString& outputFile);
    
    /**
     */
    DCOPRef downloadRevision(const QString& fileName, const QString& revision,
                             const QString& outputFile);

    /**
     */
    DCOPRef downloadRevision(const QString& fileName, const QString& revA,
                             const QString& outputFileA, const QString& revB,
                             const QString& outputFileB);

    /**
     *
     * @param fileName
     * @param revA
     * @param revB
     * @param diffOptions
     * @param contextLines
     *
     * @return A DCOP reference to the cvs job or in case of failure a
     *         null reference.
     */
    DCOPRef diff(const QString& fileName, const QString& revA,
                 const QString& revB, const QString& diffOptions,
                 unsigned contextLines);
    
    /**
     *
     * @param fileName
     * @param revA
     * @param revB
     * @param diffOptions
     * @param format
     *
     * @return A DCOP reference to the cvs job or in case of failure a
     *         null reference.
     */
    DCOPRef diff(const QString& fileName, const QString& revA,
                 const QString& revB, const QString& diffOptions,
                 const QString& format);

    /**
     * @param files
     */
    DCOPRef edit(const QStringList& files);

    /**
     * @param files
     */
    DCOPRef editors(const QStringList& files);

    /**
     * Shows a history of activity (like checkouts, commits, etc) in the
     * repository for all users and all record types.
     *
     * @return A DCOP reference to the cvs job or in case of failure a
     *         null reference.
     */
    DCOPRef history();

    /**
     * @return A DCOP reference to the cvs job or in case of failure a
     *         null reference.
     */
    DCOPRef import(const QString& workingDir, const QString& repository,
                   const QString& module, const QString& ignoreList,
                   const QString& comment, const QString& vendorTag,
                   const QString& releaseTag, bool importAsBinary);

    /**
     * @return A DCOP reference to the cvs job or in case of failure a
     *         null reference.
     */
    //### KDE4: merge with above import() method
    DCOPRef import(const QString& workingDir, const QString& repository,
                   const QString& module, const QString& ignoreList,
                   const QString& comment, const QString& vendorTag,
                   const QString& releaseTag, bool importAsBinary,
                   bool useModificationTime);

    /**
     * @param files
     */
    DCOPRef lock(const QStringList& files);

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
     * @param repository
     *
     * @return A DCOP reference to the cvs job or in case of failure a
     *         null reference.
     */
    DCOPRef login(const QString& repository);

    /**
     * @param repository
     *
     * @return A DCOP reference to the cvs job or in case of failure a
     *         null reference.
     */
    DCOPRef logout(const QString& repository);

    /**
     */
    DCOPRef makePatch();
    
    /**
     */
    //### KDE4: merge with above makePatch() method
    DCOPRef makePatch(const QString& diffOptions, const QString& format);

    /**
     * @param repository
     *
     * @return A DCOP reference to the cvs job or in case of failure a
     *         null reference.
     */
    DCOPRef moduleList(const QString& repository);

    /**
     * Deletes files from the local working copy and schedules them to be
     * removed from the repository. The files don't actually disappear from
     * the repository until a subsequent commit is performed.
     *
     * @param files A list of files that should be removed from the repository.
     * @param recursive descend into subdirectories.
     *
     * @return A DCOP reference to the cvs job or in case of failure a
     *         null reference.
     */
    DCOPRef remove(const QStringList& files, bool recursive);

    /**
     */
    DCOPRef removeWatch(const QStringList& files, int events);

    /**
     */
    DCOPRef rlog(const QString& repository, const QString& module, 
                 bool recursive);

    /**
     * Shows a summary of what's been done locally, without changing the
     * working copy. (cvs -n update)
     *
     * @param files
     * @param recursive descend into subdirectories.
     * @param createDirs
     * @param pruneDirs
     *
     * @return A DCOP reference to the cvs job or in case of failure a
     *         null reference.
     */
    DCOPRef simulateUpdate(const QStringList& files, bool recursive,
                           bool createDirs, bool pruneDirs);

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
     * @param files
     */
    DCOPRef unedit(const QStringList& files);

    /**
     * @param files
     */
    DCOPRef unlock(const QStringList& files);

    /**
     * Merges changes from the repository into the files of the
     * working copy.
     *
     * @param files A list of files that should be updated.
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
     * @param files
     */
    DCOPRef watchers(const QStringList& files);

    /**
     * Quits the DCOP service.
     */
    void quit();

private:
    struct Private;
    Private* d;
};


#endif
