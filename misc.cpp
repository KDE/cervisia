/* 
 *  Copyright (C) 1999-2001 Bernd Gehrmann
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

#include "config.h"
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <qfile.h>
#include <qregexp.h>
#include <ctype.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kprocess.h>
#include <ktempfile.h>
#include "cervisiapart.h"
#include "misc.h"


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


QString colorAsString(const QColor &color)
{
    int n = color.blue() + (color.green() << 8) + (color.red() << 16);
    // Ensure leading zeros
    return QString::number(n + 0x1000000, 16).mid(1);
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


// Gives the user name (real name + mail address) for the changelog entry
QString userName()
{
    char hostname[512];
    
    struct passwd *pw = ::getpwuid(getuid());
    // pw==0 => the system must be really fucked up
    if (!pw)
        return QString();

    // I guess we don't have to support users with longer host names ;-)
    gethostname(hostname, sizeof hostname);

    QString res = pw->pw_gecos;
    res += "  <";
    res += pw->pw_name;
    res += "@";
    res += hostname;
    res += ">";

    return res;
}


static QStringList *tempFiles = 0;

void cleanupTempFiles()
{
    if (tempFiles)
        {
            QStringList::Iterator it;
            for (it = tempFiles->begin(); it != tempFiles->end(); ++it)
#if 0
                QFile::remove(*it);
#else
                {
                    QFile f(*it);
                    f.remove();
                }
#endif
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

// Local Variables:
// c-basic-offset: 4
// End:
