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


#include <stdlib.h>
#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>
#include <kapplication.h>
#include <kconfig.h>

#include "repositories.h"
#include "cervisiapart.h"


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
                        list.append(line.left(pos));
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
