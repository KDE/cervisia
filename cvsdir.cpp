/* 
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
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
