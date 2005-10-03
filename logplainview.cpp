/*
 *  Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
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


#include "logplainview.h"

#include <qregexp.h>
#include <qstringlist.h>
#include <qstylesheet.h>
#include <kfind.h>
#include <kfinddialog.h>
#include <klocale.h>

#include "loginfo.h"

using namespace Cervisia;


LogPlainView::LogPlainView(QWidget* parent, const char* name)
    : KTextBrowser(parent, name)
    , m_find(0)
    , m_findPos(0)
{
    setNotifyClick(false);
}


LogPlainView::~LogPlainView()
{
    delete m_find; m_find = 0;
}


void LogPlainView::addRevision(const LogInfo& logInfo)
{
    setTextFormat(QStyleSheet::RichText);

    // assemble revision information lines
    QString logEntry;

    logEntry += "<b>" + i18n("revision %1").arg(QStyleSheet::escape(logInfo.m_revision)) +
                "</b>";
    logEntry += " &nbsp;[<a href=\"revA#" + QStyleSheet::escape(logInfo.m_revision) + "\">" +
                i18n("Select for revision A") +
                "</a>]";
    logEntry += " [<a href=\"revB#" + QStyleSheet::escape(logInfo.m_revision) + "\">" +
                i18n("Select for revision B") +
                "</a>]<br>";
    logEntry += "<i>" +
                i18n("date: %1; author: %2").arg(QStyleSheet::escape(logInfo.dateTimeToString()))
                                            .arg(QStyleSheet::escape(logInfo.m_author)) +
                "</i>";

    append(logEntry);

    setTextFormat(QStyleSheet::PlainText);

    const QChar newline('\n');

    // split comment in separate lines
    QStringList lines = QStringList::split(newline, logInfo.m_comment, true);

    append(newline);
    QStringList::Iterator it  = lines.begin();
    QStringList::Iterator end = lines.end();
    for( ; it != end; ++it )
    {
        append((*it).isEmpty() ? QString(newline) : *it);
    }
    append(newline);

    setTextFormat(QStyleSheet::RichText);

    for( LogInfo::TTagInfoSeq::const_iterator it = logInfo.m_tags.begin();
         it != logInfo.m_tags.end(); ++it )
    {
        append("<i>" + QStyleSheet::escape((*it).toString()) + "</i>");
    }

    // add an empty line when we had tags or branches
    if( !logInfo.m_tags.empty() )
    {
        setTextFormat(QStyleSheet::PlainText);
        append(newline);
    }

    // add horizontal line
    setTextFormat(QStyleSheet::RichText);
    append("<hr>");
}


void LogPlainView::searchText(int options, const QString& pattern)
{
    m_find = new KFind(pattern, options, this);

    connect(m_find, SIGNAL(highlight(const QString&, int, int)),
            this, SLOT(searchHighlight(const QString&, int, int)));
    connect(m_find, SIGNAL(findNext()),
           this, SLOT(findNext()));

    m_findPos = 0;
    if( options & KFindDialog::FromCursor )
    {
        const QPoint pos(contentsX(), contentsY());
        m_findPos = paragraphAt(pos);
    }

    findNext();
}


void LogPlainView::scrollToTop()
{
    setContentsPos(0, 0);
}


void LogPlainView::findNext()
{
    static const QRegExp breakLineTag("<br[^>]*>");
    static const QRegExp htmlTags("<[^>]*>");

    KFind::Result res = KFind::NoMatch;

    while( res == KFind::NoMatch && m_findPos < paragraphs() && m_findPos >= 0 )
    {
        if( m_find->needData() )
        {
            QString richText = text(m_findPos);

            // replace <br/> with '\n'
            richText.replace(breakLineTag, "\n");

            // remove html tags from text
            richText.replace(htmlTags, "");

            m_find->setData(richText);
        }

        res = m_find->find();

        if( res == KFind::NoMatch )
        {
            if( m_find->options() & KFindDialog::FindBackwards )
                --m_findPos;
            else
                ++m_findPos;
        }
    }

    // reached the end?
    if( res == KFind::NoMatch )
    {
        if( m_find->shouldRestart() )
        {
            m_findPos = 0;
            findNext();
        }
        else
        {
            delete m_find;
            m_find = 0;
        }
    }
}


void LogPlainView::searchHighlight(const QString& text, int index, int length)
{
    Q_UNUSED(text);
    setSelection(m_findPos, index, m_findPos, index + length);
}


void LogPlainView::setSource(const QString& name)
{
    if( name.isEmpty() )
        return;

    bool selectedRevisionB = name.startsWith("revB#");
    if( selectedRevisionB || name.startsWith("revA#") )
    {
        emit revisionClicked(name.mid(5), selectedRevisionB);
    }
}

#include "logplainview.moc"
