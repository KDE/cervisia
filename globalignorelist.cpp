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

#include "globalignorelist.h"
using namespace Cervisia;

#include <qdir.h>
#include <kdebug.h>
#include <stdlib.h> // for getenv()

StringMatcher GlobalIgnoreList::m_stringMatcher;
bool          GlobalIgnoreList::m_isInitialized = false;


GlobalIgnoreList::GlobalIgnoreList()
{
    if( !m_isInitialized )
        setup();
}


bool GlobalIgnoreList::matches(const QFileInfo* fi) const
{
    return m_stringMatcher.match(fi->fileName());
}


void GlobalIgnoreList::addEntry(const QString& entry)
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


void GlobalIgnoreList::setup()
{
    static const char ignorestr[] = ". .. core RCSLOG tags TAGS RCS SCCS .make.state\
.nse_depinfo #* .#* cvslog.* ,* CVS CVS.adm .del-* *.a *.olb *.o *.obj\
*.so *.Z *~ *.old *.elc *.ln *.bak *.BAK *.orig *.rej *.exe _$* *$";
    
    addEntriesFromString(QString::fromLatin1(ignorestr));
    // TODO?: addEntriesFromFile($CVSROOT/CVSROOT/cvsignore)
    addEntriesFromString(QString::fromLocal8Bit(::getenv("CVSIGNORE")));
    addEntriesFromFile(QDir::homeDirPath() + "/.cvsignore");  
    
    m_isInitialized = true;
}
