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

#ifndef CERVISIA_IGNORELISTBASE_H
#define CERVISIA_IGNORELISTBASE_H

class QFileInfo;
class QString;


namespace Cervisia
{


class IgnoreListBase
{
public:
    virtual ~IgnoreListBase() {}
    
    virtual bool matches(const QFileInfo* fi) const = 0;

protected:
    void addEntriesFromString(const QString& str);
    void addEntriesFromFile(const QString& name);

private:
    virtual void addEntry(const QString& entry) = 0;
};


}


#endif
