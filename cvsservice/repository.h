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

#ifndef REPOSITORY_H
#define REPOSITORY_H

class QString;


/**
 * Represents a local or remote cvs repository with
 * its repository-specific configuration data.
 */
class Repository
{
public:
    explicit Repository(const QString& workingCopyDir);
    ~Repository();
    
    void updateConfig();

    /**
     * cvs command (including the user-specified path) with the options
     * for this repository.
     *
     * @return A cvs command (including path).
     */
    QString cvsClient() const;

    /**
     * Path and method to access the cvs repository.
     * i.e. :pserver:user@cvs.project.org:/home/project
     *
     * @return The path and method to access the cvs repository.
     */
    QString location() const;

    QString rsh() const;

private:
    struct Private;
    Private* d;
};


#endif
