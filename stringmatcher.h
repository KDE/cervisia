/*
 * Copyright (c) 2003-2005 André Wöbbeking <Woebbeking@web.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef CERVISIA_STRINGMATCHER_H
#define CERVISIA_STRINGMATCHER_H


#include <qstringlist.h>


namespace Cervisia
{


class StringMatcher
{
public:

    /**
     * @return \c true, if text matches one of the given patterns.
     */
    bool match(const QString& text) const;

    /**
     * Adds pattern \a pattern.
     */
    void add(const QString& pattern);

    /**
     * Removes all patterns.
     */
    void clear();

private:

    /**
     * The patterns which are tested in match().
     */
    QStringList m_exactPatterns;

    /**
     * The patterns which are tested in match().
     */
    QStringList m_startPatterns;

    /**
     * The patterns which are tested in match().
     */
    QStringList m_endPatterns;

    /**
     * The patterns which are tested in match().
     */
    QValueList<QCString> m_generalPatterns;
};


} // namespace Cervisia


#endif // CERVISIA_STRINGMATCHER_H
