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

#include "ignorelistbase.h"
using namespace Cervisia;

#include <qfile.h>
#include <qstringlist.h>
#include <qtextstream.h>


void IgnoreListBase::addEntriesFromString(const QString& str)
{
    QStringList entries = QStringList::split(' ', str);
    for( QStringList::iterator it = entries.begin(); it != entries.end(); ++it )
    {
        addEntry(*it);
    }
}


void IgnoreListBase::addEntriesFromFile(const QString& name)
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
