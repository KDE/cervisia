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

#include <qstringlist.h>
#include <qdir.h>
#include <stdlib.h>

// For some reason fnmatch is defined as ap_fnmatch
#define ap_fnmatch fnmatch
#include <fnmatch.h>
#include "misc.h"

#include "cvsdir.h"


class CvsIgnoreList
{
public:
    explicit CvsIgnoreList(const QDir &dir);

    void addEntriesFromString(const QString &str);
    void addEntriesFromFile(const QString &name);

    bool matches(const QFileInfo *fi) const;

private:
    QStrList ignoreList;
};


CvsIgnoreList::CvsIgnoreList(const QDir &dir)
{
    static const char *ignorestr = ". .. core RCSLOG tags TAGS RCS SCCS .make.state\
.nse_depinfo #* .#* cvslog.* ,* CVS CVS.adm .del-* *.a *.olb *.o *.obj\
*.so *.Z *~ *.old *.elc *.ln *.bak *.BAK *.orig *.rej *.exe _$* *$";

    addEntriesFromString(ignorestr);
    // TODO?: addEntriesFromFile($CVSROOT/CVSROOT/cvsignore)
    addEntriesFromFile(QDir::homeDirPath() + "/.cvsignore");
    addEntriesFromString(::getenv("CVSIGNORE"));
    addEntriesFromFile(dir.absPath() + "/.cvsignore");
}


void CvsIgnoreList::addEntriesFromString(const QString &str)
{
    QStringList tokens = splitLine(str);
    
    for ( QStringList::Iterator it = tokens.begin();
          it != tokens.end(); ++it )
	{
            if ( *it == "!" )
		ignoreList.clear();
	    else
                ignoreList.append((*it).local8Bit());
	}
}


void CvsIgnoreList::addEntriesFromFile(const QString &name)
{
    QFile file(name);

    if( file.open(IO_ReadOnly) )
    {
        QString line;

        while( file.readLine(line, 512) != -1 )
            addEntriesFromString(line);

        file.close();
    }
}


bool CvsIgnoreList::matches(const QFileInfo *fi) const
{
    // Directories e.g. with the name core never match
    //    if (!fi->isFile())
    //        return false;

    const QCString& fileName(QFile::encodeName(fi->fileName()));
    QStrListIterator it(ignoreList);
    for (; it.current(); ++it)
	{
	    if (::fnmatch(it.current(), fileName, FNM_PATHNAME) == 0)
		return true;
	}
    
    return false;
}


CvsDir::CvsDir(const QString &path)
    : QDir( path, 0, QDir::Name,
            QDir::All | QDir::Hidden | QDir::NoSymLinks )
{}


const QFileInfoList *CvsDir::entryInfoList() const
{
    CvsIgnoreList ignorelist(*this);
    const QFileInfoList *fulllist = QDir::entryInfoList();
    if (!fulllist)
        return 0;
    
    entrylist.clear();

    QFileInfoListIterator it(*fulllist);
    for (; it.current(); ++it)
        {
            if (!ignorelist.matches(it.current()))
                entrylist.append(it.current());
        }

    return &entrylist;
}


// Local Variables:
// c-basic-offset: 4
// End:
