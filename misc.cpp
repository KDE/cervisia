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


#include "misc.h"

#include <ctype.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <qfile.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <kconfig.h>
#include <kemailsettings.h>
#include <kprocess.h>
#include <ktempfile.h>

#include "config.h"
#include "cervisiapart.h"
#include "cvsprogressdlg.h"
#include "cvsservice_stub.h"
#include "progressdlg.h"


namespace
{
    QStringList const fetchBranchesAndTags(QString const& rsSearchedType,
                                           CvsService_stub* cvsService,
                                           QWidget*       pParentWidget);
}


void chomp(QCString *line)
{
    int pos;
    if ( (pos = line->find('\n')) != -1 )
	line->truncate(pos);  
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


int findWhiteSpace(QString const& rsString, int iStartIndex)
{
    int const length(rsString.length());
    if (iStartIndex < 0)
        iStartIndex += length;
    if (iStartIndex < 0 || iStartIndex >= length)
        return -1;

    QChar const* const startPos = rsString.unicode();
    QChar const* const endPos   = startPos + length;

    QChar const* pos = startPos + iStartIndex;
    while (pos < endPos && pos->isSpace() == false)
        ++pos;

    int const foundIndex(pos - startPos);
    return foundIndex < length ? foundIndex : -1;
}


bool isValidTag(const QString &str)
{
    if (!isalpha(str[0].latin1()))
        return false;

    for (int i = 1; i < (int)str.length(); ++i)
        {
            if (!isgraph(str[i].latin1()) || QString("$,.:;@").contains(str[i]))
                return false;
        }

    return true;
}


// Gives the name (including path) of the cvs command line client
// also gives you global commands
QString cvsClient( QString sRepository )
{
    KConfig *config = CervisiaPart::config();
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


QStringList const fetchBranches(CvsService_stub* cvsService,
                                QWidget*       pParentWidget)
{
    return fetchBranchesAndTags(QString::fromLatin1("branch"),
                                cvsService,
                                pParentWidget);
}


QStringList const fetchTags(CvsService_stub* cvsService,
                            QWidget*       pParentWidget)
{
    return fetchBranchesAndTags(QString::fromLatin1("revision"),
                                cvsService,
                                pParentWidget);
}


// Gives the user name (real name + mail address) for the changelog entry
QString userName()
{
    // 1. Try to retrieve the information from the control center settings
    KEMailSettings settings;
    QString name  = settings.getSetting(KEMailSettings::RealName);
    QString email = settings.getSetting(KEMailSettings::EmailAddress);
    
    if (name.isNull() || email.isNull())
    {
        // 2. Try to retrieve the information from the system
        struct passwd *pw = getpwuid(getuid());
        if (!pw)
            return QString::null;
            
        char hostname[512];
        hostname[0] = '\0';
        
        if (!gethostname(hostname, sizeof(hostname)))
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

QTextCodec *detectCodec(const QString &fileName)
{
    // TODO, the following conditions are a rough hack

    if (fileName.endsWith(".ui") || fileName.endsWith(".docbook")
        || fileName.endsWith(".xml"))
        return QTextCodec::codecForName("utf8");

    return QTextCodec::codecForLocale();
}


namespace
{
    QStringList const fetchBranchesAndTags(QString const& rsSearchedType,
                                           CvsService_stub* cvsService,
                                           QWidget*       pParentWidget)
    {
        QStringList listBranchesOrTags;
        
        DCOPRef job = cvsService->status(QStringList(), true, true);
        if( !cvsService->ok() )
            return listBranchesOrTags;
        
        ProgressDialog dlg(pParentWidget, "Status", job, QString::null, i18n("CVS Status"));

        if (dlg.execute())
        {
            QString sLine;
            while (dlg.getLine(sLine))
            {
                int pos1, pos2, pos3;
                if (sLine.isEmpty() || sLine[0] != '\t')
                    continue;
                if ((pos1 = ::findWhiteSpace(sLine, 2)) < 0)
                    continue;
                if ((pos2 = sLine.find('(', pos1 + 1)) < 0)
                    continue;
                if ((pos3 = sLine.find(':', pos2 + 1)) < 0)
                    continue;

                QString const tag(sLine.mid(1, pos1 - 1));
                QString const type(sLine.mid(pos2 + 1, pos3 - pos2 - 1));
                if (type == rsSearchedType && !listBranchesOrTags.contains(tag))
                    listBranchesOrTags.push_back(tag);
            }

            listBranchesOrTags.sort();
        }

        return listBranchesOrTags;
    }
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
