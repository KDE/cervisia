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

#ifndef CERVISIA_GLOBALIGNORELIST_H
#define CERVISIA_GLOBALIGNORELIST_H

#include "ignorelistbase.h"
#include "stringmatcher.h"

class QFileInfo;


namespace Cervisia
{


class GlobalIgnoreList : public IgnoreListBase
{
public:
    GlobalIgnoreList();

    virtual bool matches(const QFileInfo* fi) const;

private:
    void setup();
    virtual void addEntry(const QString& entry);

    static StringMatcher m_stringMatcher;
    static bool          m_isInitialized;
};


}


#endif
