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

#include <kfind.h>
#include <kfinddialog.h>
#include <klocale.h>

#include "loginfo.h"

using namespace Cervisia;


LogPlainView::LogPlainView(QWidget* parent, const char* name)
    : QTextBrowser(parent, name)
    , m_find(0)
{
}


LogPlainView::~LogPlainView()
{
    delete m_find; m_find = 0;
}


void LogPlainView::addRevision(const LogInfo& logInfo)
{
    // assemble revision information lines
    QString logEntry;

    logEntry += "<b>" + i18n("revision %1").arg(Qt::escape(logInfo.m_revision)) +
                "</b>";
    logEntry += " &nbsp;[<a href=\"revA#" + Qt::escape(logInfo.m_revision) + "\">" +
                i18n("Select for revision A") +
                "</a>]";
    logEntry += " [<a href=\"revB#" + Qt::escape(logInfo.m_revision) + "\">" +
                i18n("Select for revision B") +
                "</a>]<br>";
    logEntry += "<i>" +
                i18n("date: %1; author: %2").arg(Qt::escape(logInfo.dateTimeToString()))
                                            .arg(Qt::escape(logInfo.m_author)) +
                "</i><br><br>";

    insertHtml(logEntry);

    const QLatin1String lineBreak("<br>");

    insertPlainText(logInfo.m_comment);
    insertHtml(lineBreak);

    for( LogInfo::TTagInfoSeq::const_iterator it = logInfo.m_tags.begin();
         it != logInfo.m_tags.end(); ++it )
    {
        insertHtml("<br><i>" + Qt::escape((*it).toString()) + "</i>");
    }

    // add an empty line when we had tags or branches
    if( !logInfo.m_tags.empty() )
        insertHtml(lineBreak);
    insertHtml(lineBreak);

    // add horizontal line
    insertHtml(QLatin1String("<hr><br>"));
}


void LogPlainView::searchText(int options, const QString& pattern)
{
    m_find = new KFind(pattern, options, this);

    connect(m_find, SIGNAL(highlight(const QString&, int, int)),
            this, SLOT(searchHighlight(const QString&, int, int)));
    connect(m_find, SIGNAL(findNext()),
           this, SLOT(findNext()));

    m_currentBlock = (m_find->options() & KFindDialog::FindBackwards)
        ? document()->end().previous()
        : document()->begin();
    if( options & KFindDialog::FromCursor )
    {
#warning maybe this can be improved
        const QPoint pos(horizontalScrollBar()->value(), 0);
        const QTextCursor cursor(cursorForPosition(pos));
        if (!cursor.isNull())
            m_currentBlock = cursor.block();
    }

    findNext();
}


void LogPlainView::scrollToTop()
{
    QTextCursor cursor(document());
    cursor.movePosition(QTextCursor::Start);
    setTextCursor(cursor);
}


void LogPlainView::findNext()
{
    KFind::Result res = KFind::NoMatch;

    while( (res == KFind::NoMatch) && m_currentBlock.isValid() )
    {
        if( m_find->needData() )
            m_find->setData(m_currentBlock.text());

        res = m_find->find();

        if( res == KFind::NoMatch )
        {
            if( m_find->options() & KFindDialog::FindBackwards )
                m_currentBlock = m_currentBlock.previous();
            else
                m_currentBlock = m_currentBlock.next();
        }
    }

    // reached the end?
    if( res == KFind::NoMatch )
    {
        if( m_find->shouldRestart() )
        {
            m_currentBlock = (m_find->options() & KFindDialog::FindBackwards)
                ? document()->end().previous()
                : document()->begin();
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

    const int position(m_currentBlock.position() + index);

    QTextCursor cursor(document());
    cursor.setPosition(position);
    cursor.setPosition(position + length, QTextCursor::KeepAnchor);
    setTextCursor(cursor);
}


void LogPlainView::setSource(const QUrl& url)
{
    const QString name(url.toString());
    if( name.isEmpty() )
        return;

    bool selectedRevisionB = name.startsWith("revB#");
    if( selectedRevisionB || name.startsWith("revA#") )
    {
        emit revisionClicked(name.mid(5), selectedRevisionB);
    }
}

#include "logplainview.moc"
