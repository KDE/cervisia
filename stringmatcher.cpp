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


#include "stringmatcher.h"

// For some reason fnmatch is defined as ap_fnmatch
#define ap_fnmatch fnmatch
#include <fnmatch.h>


namespace Cervisia
{
namespace
{
    const QChar asterix('*');
    const QChar question('?');

    inline bool isMetaCharacter(QChar c)
    {
        return c == asterix || c == question;
    }


    unsigned int countMetaCharacters(const QString& text);
}


bool StringMatcher::match(const QString& text) const
{
    if (m_exactPatterns.find(text) != m_exactPatterns.end())
    {
        return true;
    }

    for (QStringList::const_iterator it(m_startPatterns.begin()),
                                     itEnd(m_startPatterns.end());
         it != itEnd; ++it)
    {
        if (text.startsWith(*it))
        {
            return true;
        }
    }

    for (QStringList::const_iterator it(m_endPatterns.begin()),
                                     itEnd(m_endPatterns.end());
         it != itEnd; ++it)
    {
        if (text.endsWith(*it))
        {
            return true;
        }
    }

    for (QValueList<QCString>::const_iterator it(m_generalPatterns.begin()),
                                              itEnd(m_generalPatterns.end());
         it != itEnd; ++it)
    {
        if (::fnmatch(*it, text.local8Bit(), FNM_PATHNAME) == 0)
        {
            return true;
        }
    }

    return false;
}


void StringMatcher::add(const QString& pattern)
{
    if (pattern.isEmpty())
    {
        return;
    }

    const int lengthMinusOne(pattern.length() - 1);
    switch (countMetaCharacters(pattern))
    {
    case 0:
        m_exactPatterns.push_back(pattern);
        break;

    case 1:
        if (pattern.constref(0) == asterix)
        {
            m_endPatterns.push_back(pattern.right(lengthMinusOne));
        }
        else if (pattern.constref(lengthMinusOne) == asterix)
        {
            m_startPatterns.push_back(pattern.left(lengthMinusOne));
        }
        else
        {
            m_generalPatterns.push_back(pattern.local8Bit());
        }
        break;

    default:
        m_generalPatterns.push_back(pattern.local8Bit());
        break;
    }
}


void StringMatcher::clear()
{
    m_exactPatterns.clear();
    m_startPatterns.clear();
    m_endPatterns.clear();
    m_generalPatterns.clear();
}


namespace
{
unsigned int countMetaCharacters(const QString& text)
{
    unsigned int count(0);

    const QChar* pos(text.unicode());
    const QChar* posEnd(pos + text.length());
    while (pos < posEnd)
    {
        count += isMetaCharacter(*pos++);
    }

    return count;
}
}
} // namespace Cervisia
