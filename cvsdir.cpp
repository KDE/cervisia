/* 
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include "cvsdir.h"

#include "dirignorelist.h"
#include "globalignorelist.h"
using namespace Cervisia;


CvsDir::CvsDir(const QString &path)
    : QDir( path, 0, QDir::Name,
            QDir::All | QDir::Hidden | QDir::NoSymLinks )
{}


const QFileInfoList *CvsDir::entryInfoList() const
{
    DirIgnoreList ignorelist(absPath());
    const QFileInfoList *fulllist = QDir::entryInfoList();
    if (!fulllist)
        return 0;
    
    entrylist.clear();

    QFileInfoListIterator it(*fulllist);
    for (; it.current(); ++it)
        {
            if (!ignorelist.matches(it.current()) && !GlobalIgnoreList().matches(it.current()))
                entrylist.append(it.current());
        }

    return &entrylist;
}


// Local Variables:
// c-basic-offset: 4
// End:
