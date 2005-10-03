/*
 *  Copyright (C) 1999-2001 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include <stdlib.h>
#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>
#include <kapplication.h>
#include <kconfig.h>

#include "repositories.h"
#include "cervisiapart.h"


// old .cvspass format:
//    user@host:/path Acleartext_password
//
// new .cvspass format (since cvs 1.11.1):
//    /1 user@host:port/path Aencoded_password
//
QStringList Repositories::readCvsPassFile()
{
    QStringList list;
    
    QFile f(QDir::homeDirPath() + "/.cvspass");
    if (f.open(IO_ReadOnly))
        {
            QTextStream stream(&f);
	    while (!stream.eof())
		{
		    int pos;
		    QString line = stream.readLine();
		    if ( (pos = line.find(' ')) != -1)
		    {
			if (line[0] != '/')	// old format
                            list.append(line.left(pos));
			else			// new format
			    list.append(line.section(' ', 1, 1));
		    }
		}
            f.close();
	}

    return list;
}


QStringList Repositories::readConfigFile()
{
    QStringList list;
    
    KConfig *config = CervisiaPart::config();
    config->setGroup("Repositories");
    list = config->readListEntry("Repos");

    // Some people actually use CVSROOT, so we add it here
    char *env;
    if ( (env = ::getenv("CVSROOT")) != 0 && !list.contains(env))
        list.append(env);

    return list;
}


// Local Variables:
// c-basic-offset: 4
// End:
