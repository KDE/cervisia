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
#include <ktempfile.h>
#include <stdlib.h> // for getenv()

#include "cvsservice_stub.h"
#include "progressdlg.h"


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


void GlobalIgnoreList::retrieveServerIgnoreList(CvsService_stub* cvsService,
                                                const QString& repository)
{
    KTempFile tmpFile;
    tmpFile.setAutoDelete(true);
       
    // clear old ignore list
    m_stringMatcher.clear();
    
    // now set it up again
    setup();
    
    DCOPRef ref = cvsService->downloadCvsIgnoreFile(repository, 
                                                    tmpFile.name());
      
    ProgressDialog dlg(0, "Edit", ref, "checkout", "CVS Edit");
    if( !dlg.execute() )
        return;
    
    addEntriesFromFile(tmpFile.name());
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
    addEntriesFromString(QString::fromLocal8Bit(::getenv("CVSIGNORE")));
    addEntriesFromFile(QDir::homeDirPath() + "/.cvsignore");  
    
    m_isInitialized = true;
}
