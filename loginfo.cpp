/*
 * Copyright (c) 2003 André Wöbbeking <Woebbeking@web.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "loginfo.h"

#include <qstylesheet.h>

#include <kglobal.h>
#include <klocale.h>


namespace Cervisia
{


TagInfo::TagInfo(const QString& name, Type type)
    : m_name(name),
      m_type(type)
{
}


QString TagInfo::toString(bool prefixWithType) const
{
    QString text;
    if (prefixWithType)
    {
        text += typeToString() + QString::fromLatin1(": ");
    }
    text += m_name;

    return text;
}


QString TagInfo::typeToString() const
{
    QString text;
    switch (m_type)
    {
    case Branch:
        text = i18n("Branchpoint");
        break;
    case OnBranch:
        text = i18n("On Branch");
        break;
    case Tag:
        text = i18n("Tag");
        break;
    }

    return text;
}


QString LogInfo::createToolTipText() const
{
    QString text(QString::fromLatin1("<b>"));
    text += QStyleSheet::escape(m_revision);
    text += QString::fromLatin1("</b>&nbsp;&nbsp;");
    text += QStyleSheet::escape(m_author);
    text += QString::fromLatin1("&nbsp;&nbsp;<b>");
    text += QStyleSheet::escape(dateTimeToString());
    text += QString::fromLatin1("</b>");

    if (!m_comment.isEmpty())
    {
        text += QString::fromLatin1("<pre>");
        text += m_comment;
        text += QString::fromLatin1("</pre>");
    }

    if (!m_tags.isEmpty())
    {
        text += QString::fromLatin1("<i>");
        for (TTagInfoSeq::const_iterator it = m_tags.begin();
             it != m_tags.end(); ++it)
        {
            if (it != m_tags.begin() || m_comment.isEmpty())
                text += QString::fromLatin1("<br>");
            text += QStyleSheet::escape((*it).toString());
        }
        text += QString::fromLatin1("</i>");
    }

    return text;
}


QString LogInfo::dateTimeToString(bool shortFormat) const
{
    return KGlobal::locale()->formatDateTime(m_dateTime, shortFormat);
}


QString LogInfo::tagsToString(unsigned int types,
                              unsigned int prefixWithType,
                              const QString& separator) const
{
    QString text;
    for (TTagInfoSeq::const_iterator it = m_tags.begin();
         it != m_tags.end(); ++it)
    {
        const TagInfo& tagInfo(*it);

        if (tagInfo.m_type & types)
        {
            if (!text.isEmpty())
            {
                text += separator;
            }

            text += tagInfo.toString(tagInfo.m_type & prefixWithType);
        }
    }

    return text;
}


} // namespace Cervisia
