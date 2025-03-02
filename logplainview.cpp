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

#include "loginfo.h"
#include <KLocalizedString>
#include <QScrollBar>
#include <kfind.h>
#include <kfinddialog.h>

using namespace Cervisia;

LogPlainView::LogPlainView(QWidget *parent)
    : QTextBrowser(parent)
    , m_find(0)
{
}

LogPlainView::~LogPlainView()
{
    delete m_find;
    m_find = 0;
}

void LogPlainView::addRevision(const LogInfo &logInfo)
{
    // assemble revision information lines
    QString logEntry;

    logEntry += "<b>" + i18n("revision %1", logInfo.m_revision.toHtmlEscaped()) + "</b>";
    logEntry += " &nbsp;[<a href=\"revA#" + logInfo.m_revision.toHtmlEscaped() + "\">" + i18n("Select for revision A") + "</a>]";
    logEntry += " [<a href=\"revB#" + logInfo.m_revision.toHtmlEscaped() + "\">" + i18n("Select for revision B") + "</a>]<br>";
    logEntry += "<i>" + i18n("date: %1; author: %2", logInfo.dateTimeToString().toHtmlEscaped(), logInfo.m_author.toHtmlEscaped()) + "</i><br><br>";

    insertHtml(logEntry);

    const QLatin1String lineBreak("<br>");

    insertPlainText(logInfo.m_comment);
    insertHtml(lineBreak);

    for (LogInfo::TTagInfoSeq::const_iterator it = logInfo.m_tags.begin(); it != logInfo.m_tags.end(); ++it) {
        insertHtml("<br><i>" + (*it).toString().toHtmlEscaped() + "</i>");
    }

    // add an empty line when we had tags or branches
    if (!logInfo.m_tags.empty())
        insertHtml(lineBreak);
    // seems to be superfluous with Qt 4.4
    // insertHtml(lineBreak);

    // workaround Qt bug (TT ID 166111)
    const QTextBlockFormat blockFmt(textCursor().blockFormat());

    // add horizontal line
    insertHtml(QLatin1String("<hr><br>"));

    textCursor().setBlockFormat(blockFmt);
}

void LogPlainView::searchText(int options, const QString &pattern)
{
    m_find = new KFind(pattern, options, this);

    connect(m_find, SIGNAL(highlight(QString, int, int)), this, SLOT(searchHighlight(QString, int, int)));
    connect(m_find, SIGNAL(findNext()), this, SLOT(findNext()));

    m_currentBlock = (m_find->options() & KFind::FindBackwards) ? document()->end().previous() : document()->begin();
    if (options & KFind::FromCursor) {
#ifdef __GNUC__
#warning maybe this can be improved
#endif
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

    while ((res == KFind::NoMatch) && m_currentBlock.isValid()) {
        if (m_find->needData())
            m_find->setData(m_currentBlock.text());

        res = m_find->find();

        if (res == KFind::NoMatch) {
            if (m_find->options() & KFind::FindBackwards)
                m_currentBlock = m_currentBlock.previous();
            else
                m_currentBlock = m_currentBlock.next();
        }
    }

    // reached the end?
    if (res == KFind::NoMatch) {
        if (m_find->shouldRestart()) {
            m_currentBlock = (m_find->options() & KFind::FindBackwards) ? document()->end().previous() : document()->begin();
            findNext();
        } else {
            delete m_find;
            m_find = 0;
        }
    }
}

void LogPlainView::searchHighlight(const QString &text, int index, int length)
{
    Q_UNUSED(text);

    const int position(m_currentBlock.position() + index);

    QTextCursor cursor(document());
    cursor.setPosition(position);
    cursor.setPosition(position + length, QTextCursor::KeepAnchor);
    setTextCursor(cursor);
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void LogPlainView::setSource(const QUrl &url)
#else
void LogPlainView::doSetSource(const QUrl &url, QTextDocument::ResourceType type)
#endif
{
    const QString name(url.toString());
    if (name.isEmpty())
        return;

    bool selectedRevisionB = name.startsWith(QLatin1String("revB#"));
    if (selectedRevisionB || name.startsWith(QLatin1String("revA#"))) {
        Q_EMIT revisionClicked(name.mid(5), selectedRevisionB);
    }
}
