/* 
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@physik.hu-berlin.de
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
#include <fnmatch.h>
#include "misc.h"

#include "cvsdir.h"


class CvsIgnoreList : public QStrList
{
public:
    CvsIgnoreList(const QDir &dir);

    void addEntriesFromString(const QString &str);
    void addEntriesFromFile(const QString &name);
    bool matches(QFileInfo *fi);
};


CvsIgnoreList::CvsIgnoreList(const QDir &dir)
    : QStrList(true) // always make deep copies
{
    static const char *ignorestr = ". .. core RCSLOG tags TAGS RCS SCCS .make.state\
.nse_depinfo #* .#* cvslog.* ,* CVS CVS.adm .del-* *.a *.olb *.o *.obj\
*.so *.Z *~ *.old *.elc *.ln *.bak *.BAK *.orig *.rej *.exe _$* *$";

    addEntriesFromString(ignorestr);
    // TODO?: addEntriesFromFile($CVSROOT/CVSROOT/cvsignore)
    addEntriesFromFile(QDir::homeDirPath() + "/.cvsignore");
    addEntriesFromString(getenv("CVSIGNORE"));
    addEntriesFromFile(dir.absPath() + "/.cvsignore");
}


void CvsIgnoreList::addEntriesFromString(const QString &str)
{
    QStringList tokens = splitLine(str);
    
    for ( QStringList::Iterator it = tokens.begin();
          it != tokens.end(); ++it )
	{
            if ( *it == "!" )
		clear();
	    else
                append((*it).latin1());
	}
}


void CvsIgnoreList::addEntriesFromFile(const QString &name)
{
    char buf[512];
    // FIXME: Use QFile
    FILE *f = fopen(name.latin1(), "r");
    if (!f)
	return;

    while (fgets(buf, sizeof buf, f))
	{
	    QString line = buf;
	    addEntriesFromString(buf);
	}
    fclose(f);
}


bool CvsIgnoreList::matches(QFileInfo *fi)
{
    // Directories e.g. with the name core never match
    //    if (!fi->isFile())
    //        return false;
    
    QStrListIterator it(*this);
    for (; it.current(); ++it)
	{
	    if (fnmatch(it.current(), fi->fileName().latin1(), FNM_PATHNAME) == 0)
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
    
    QFileInfoList &elist = const_cast<QFileInfoList&>(entrylist);
    elist.clear();

    QFileInfoListIterator it(*fulllist);
    for (; it.current(); ++it)
        {
            if (it.current()->fileName() == ".")
                continue;
            if (it.current()->fileName() == "..")
                continue;
            if (ignorelist.matches(it.current()))
                continue;
            elist.append(it.current());
        }

    return &entrylist;
}


// Local Variables:
// c-basic-offset: 4
// End:
