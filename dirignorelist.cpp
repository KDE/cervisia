/* 
 *  Copyright (C) 2004 Christian Loose <christian.loose@kdemail.net>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "dirignorelist.h"
using namespace Cervisia;

#include <qfileinfo.h>


DirIgnoreList::DirIgnoreList(const QString& path)
{
    addEntriesFromFile(path + "/.cvsignore");
}


void DirIgnoreList::addEntry(const QString& entry)
{
    if (entry != QChar('!'))
    {
        m_stringMatcher.add(entry);
    }
    else
    {
        m_stringMatcher.clear();
    }
}


bool DirIgnoreList::matches(const QFileInfo* fi) const
{  
    return m_stringMatcher.match(fi->fileName());
}
