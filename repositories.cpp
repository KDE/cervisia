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
#include <kapp.h>
#include <kconfig.h>

#include "cervisiapart.h"
#include "repositories.h"


void Repositories::readCvsPassFile(QStrList *list)
{
    QFile f(QDir::homeDirPath() + "/.cvspass");
    if (f.open(IO_ReadOnly))
        {
            QTextStream stream(&f);
	    while (!stream.eof())
		{
		    int pos;
		    QString line = stream.readLine();
		    if ( (pos = line.find(' ')) != -1)
                        list->append(line.left(pos).latin1());
		}
            f.close();
	}
}


void Repositories::readConfigFile(QStrList *list)
{
    KConfig *config = CervisiaPart::config();
    config->setGroup("Repositories");
    config->readListEntry("Repos", *list);

    // Some people actually use CVSROOT, so we add it here
    char *env;
    if ( (env = getenv("CVSROOT")) != 0 && !list->contains(env))
        list->append(env);
}


// Local Variables:
// c-basic-offset: 4
// End:
