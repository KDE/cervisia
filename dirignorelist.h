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

#ifndef CERVISIA_DIRIGNORELIST_H
#define CERVISIA_DIRIGNORELIST_H

#include "ignorelistbase.h"
#include "stringmatcher.h"

class QFileInfo;


namespace Cervisia
{


/* Encapsulates the .cvsignore file inside a CVS-controlled directory. */
class DirIgnoreList : public IgnoreListBase
{
public:
    explicit DirIgnoreList(const QString& path);

    virtual bool matches(const QFileInfo* fi) const;

private:
    virtual void addEntry(const QString& entry);

    StringMatcher m_stringMatcher;
};


}


#endif
