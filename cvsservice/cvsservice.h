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

#ifndef CVSSERVICE_H
#define CVSSERVICE_H

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
    DCOPRef annotate(const QString& fileName, const QString& revision);
    DCOPRef status(const QString& files, bool recursive, bool createDirs,
        bool pruneDirs);

    bool openSandbox(const QString& dirName);

private:
    struct Private;
    Private* d;
};


#endif

