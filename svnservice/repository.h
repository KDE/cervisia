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

#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <qobject.h>
#include <dcopobject.h>

class QString;


/**
 * Represents a local or remote Subversion repository with
 * its repository-specific configuration data.
 */
class KDE_EXPORT Repository : public QObject, public DCOPObject
{
    K_DCOP
    Q_OBJECT

public:
    Repository();
    explicit Repository(const QString& repository);
    ~Repository();

    /**
     */
    QString svnClient() const;

k_dcop:
    /**
     * Changes the working copy and the corresponding Subversion repository.
     *
     * @param dirName path to the local working copy directory.
     */
    bool setWorkingCopy(const QString& dirName);

    /**
     * Path to the current working copy.
     *
     * @return The working copy directory. Can be null if not set.
     */
    QString workingCopy() const;

    /**
     */
    QString location() const;

private slots:
    void slotConfigDirty(const QString& fileName);

private:
    struct Private;
    Private* d;
};


#endif