/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "misc.h"

#include <ctype.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <qfile.h>
#include <qstringlist.h>
#include <kconfig.h>
#include <kemailsettings.h>
#include <kprocess.h>
#include <ktempfile.h>

#include "config.h"
#include "cervisiapart.h"
#include "cvsprogressdlg.h"
#include "cvsservice_stub.h"
#include "progressdlg.h"


static int FindWhiteSpace(const QString& str, int index)
{
    const int length = str.length();

    if( index < 0 )
        index += length;

    if( index < 0 || index >= length )
        return -1;

    const QChar* const startPos = str.unicode();
    const QChar* const endPos   = startPos + length;

    const QChar* pos = startPos + index;
    while( pos < endPos && !pos->isSpace() )
        ++pos;

    const int foundIndex = pos - startPos;
    return (foundIndex < length ? foundIndex : -1);
}


static const QStringList FetchBranchesAndTags(const QString& searchedType,
                                              CvsService_stub* cvsService,
                                              QWidget* parent)
{
    QStringList branchOrTagList;

    DCOPRef job = cvsService->status(QStringList(), true, true);
    if( !cvsService->ok() )
        return branchOrTagList;

    ProgressDialog dlg(parent, "Status", job, QString::null, i18n("CVS Status"));

    if( dlg.execute() )
    {
        QString line;
        while( dlg.getLine(line) )
        {
            int wsPos, bracketPos, colonPos;

            if( line.isEmpty() || line[0] != '\t' )
                continue;
            if( (wsPos = FindWhiteSpace(line, 2)) < 0 )
                continue;
            if( (bracketPos = line.find('(', wsPos + 1)) < 0 )
                continue;
            if( (colonPos = line.find(':', bracketPos + 1)) < 0 )
                continue;

            const QString tag  = line.mid(1, wsPos - 1);
            const QString type = line.mid(bracketPos + 1, colonPos - bracketPos - 1);
            if( type == searchedType && !branchOrTagList.contains(tag) )
                branchOrTagList.push_back(tag);
        }

        branchOrTagList.sort();
    }

    return branchOrTagList;
}


bool Cervisia::IsValidTag(const QString& tag)
{
    static const QString prohibitedChars("$,.:;@");

    if( !isalpha(tag[0].latin1()) )
        return false;

    for( uint i = 1; i < tag.length(); ++i )
    {
        if( !isgraph(tag[i].latin1()) || prohibitedChars.contains(tag[i]) )
                return false;
    }

    return true;
}


QString Cervisia::UserName()
{
    // 1. Try to retrieve the information from the control center settings
    KEMailSettings settings;
    QString name  = settings.getSetting(KEMailSettings::RealName);
    QString email = settings.getSetting(KEMailSettings::EmailAddress);

    if( name.isEmpty() || email.isEmpty() )
    {
        // 2. Try to retrieve the information from the system
        struct passwd* pw = getpwuid(getuid());
        if( !pw )
            return QString::null;

        char hostname[512];
        hostname[0] = '\0';

        if( !gethostname(hostname, sizeof(hostname)) )
            hostname[sizeof(hostname)-1] = '0';

        name  = QString::fromLocal8Bit(pw->pw_gecos);
        email = QString::fromLocal8Bit(pw->pw_name) + "@" +
                QString::fromLocal8Bit(hostname);
    }

    QString result = name;
    result += "  <";
    result += email;
    result += ">";

    return result;
}


QString joinLine(const QStringList &list)
{
    QString line;
    for ( QStringList::ConstIterator it = list.begin();
          it != list.end(); ++it )
	{
            line += KShellProcess::quote(*it);
            line += " ";
	}

    if (line.length() > 0)
	line.truncate(line.length()-1);

    return line;
}


// Should be replaceable by QStringList::split
QStringList splitLine(QString line, char delim)
{
    int pos;
    QStringList list;

    line = line.simplifyWhiteSpace();
    while ((pos = line.find(delim)) != -1)
	{
	    list.append(line.left(pos));
	    line = line.mid(pos+1, line.length()-pos-1);
	}
    if (!line.isEmpty())
	list.append(line);
    return list;
}


// Gives the name (including path) of the cvs command line client
// also gives you global commands
QString cvsClient( const QString &sRepository, KConfig* config )
{
    config->setGroup("General");

    // everybody gets the -f option, unconditionally
    QString sReturn = config->readEntry("CVSPath", "cvs") + " -f";

    // see if there is a specific level set for this repository
    config->setGroup( QString("Repository-") + sRepository );
    int compressionlevel = config->readNumEntry("Compression", -1);

    // if we were left to the default value, then see what the default value should be
    if ( compressionlevel < 0 )
        {
            config->setGroup("General");
            compressionlevel = config->readNumEntry("Compression", 0);
        }

    // we don't need a command line option if there is no compression
    if (compressionlevel > 0)
        {
            sReturn += " -z";
            sReturn += QString::number(compressionlevel);
            sReturn += " ";
        }

    return sReturn;
}


const QStringList fetchBranches(CvsService_stub* cvsService, QWidget* parent)
{
    return FetchBranchesAndTags(QString::fromLatin1("branch"), cvsService,
                                parent);
}


const QStringList fetchTags(CvsService_stub* cvsService, QWidget* parent)
{
    return FetchBranchesAndTags(QString::fromLatin1("revision"), cvsService,
                                parent);
}


static QStringList *tempFiles = 0;

void cleanupTempFiles()
{
    if (tempFiles)
        {
            QStringList::Iterator it;
            for (it = tempFiles->begin(); it != tempFiles->end(); ++it)
                QFile::remove(*it);
            delete tempFiles;
        }
}


QString tempFileName(const QString &suffix)
{
    if (!tempFiles)
        tempFiles = new QStringList;

    KTempFile f(QString::null, suffix);
    tempFiles->append(f.name());
    return f.name();
}


int compareRevisions(const QString& rev1, const QString& rev2)
{
    const int length1(rev1.length());
    const int length2(rev2.length());

    // compare all parts of the revision

    int startPos1(0);
    int startPos2(0);
    while (startPos1 < length1 && startPos2 < length2)
    {
        int pos1(rev1.find('.', startPos1));
        if (pos1 < 0)
            pos1 = length1;
        const int partLength1(pos1 - startPos1);

        int pos2(rev2.find('.', startPos2));
        if (pos2 < 0)
            pos2 = length2;
        const int partLength2(pos2 - startPos2);

        // if the number of digits in both parts is not equal we are ready
        if (const int comp = ::compare(partLength1, partLength2))
            return comp;

        // if the parts are not equal we are ready
        if (const int comp = ::compare(rev1.mid(startPos1, partLength1),
                                       rev2.mid(startPos2, partLength2)))
            return comp;

        // continue with next part
        startPos1 = pos1 + 1;
        startPos2 = pos2 + 1;
    }

    // rev1 has more parts than rev2: rev2 < rev1
    if (startPos1 < length1)
        return 1;
    // rev2 has more parts than rev1: rev1 < rev2
    else if (startPos2 < length2)
        return -1;
    // all parts of rev1 and rev2 were compared (the number of parts is equal): rev1 == rev2
    else
        return 0;
}


// Local Variables:
// c-basic-offset: 4
// End:
