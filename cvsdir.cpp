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

#include <qtextstream.h>

#include <stdlib.h>

#include "stringmatcher.h"



class CvsIgnoreList
{
public:
    explicit CvsIgnoreList(const QDir &dir);

    bool matches(const QFileInfo *fi) const;

private:

    void addEntriesFromString(const QString& str);
    void addEntriesFromFile(const QString& name);
    void addEntry(const QString& entry);

    Cervisia::StringMatcher m_stringMatcher;
};


CvsIgnoreList::CvsIgnoreList(const QDir &dir)
{
    static const char *ignorestr = ". .. core RCSLOG tags TAGS RCS SCCS .make.state\
.nse_depinfo #* .#* cvslog.* ,* CVS CVS.adm .del-* *.a *.olb *.o *.obj\
*.so *.Z *~ *.old *.elc *.ln *.bak *.BAK *.orig *.rej *.exe _$* *$";

    addEntriesFromString(QString::fromLatin1(ignorestr));
    // TODO?: addEntriesFromFile($CVSROOT/CVSROOT/cvsignore)
    addEntriesFromFile(QDir::homeDirPath() + "/.cvsignore");
    addEntriesFromString(QString::fromLocal8Bit(::getenv("CVSIGNORE")));
    addEntriesFromFile(dir.absPath() + "/.cvsignore");
}


void CvsIgnoreList::addEntriesFromString(const QString& str)
{
    int posLast(0);
    int pos;
    while ((pos = str.find(' ', posLast)) >= 0)
    {
        if (pos > posLast)
            addEntry(str.mid(posLast, pos - posLast));
        posLast = pos + 1;
    }

    if (posLast < static_cast<int>(str.length()))
        addEntry(str.mid(posLast));
}


void CvsIgnoreList::addEntriesFromFile(const QString &name)
{
    QFile file(name);

    if( file.open(IO_ReadOnly) )
    {
        QTextStream stream(&file);
        while( !stream.eof() )
        {
            addEntriesFromString(stream.readLine());
        }
    }
}


void CvsIgnoreList::addEntry(const QString& entry)
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


bool CvsIgnoreList::matches(const QFileInfo *fi) const
{
    return m_stringMatcher.match(fi->fileName());
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
