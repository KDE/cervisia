/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
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


#include "loglist.h"

#include <qapplication.h>
#include <qnamespace.h>

#include <QMouseEvent>
#include <QKeyEvent>
#include <QHeaderView>

#include <KConfigGroup>
#include <KConfig>
#include <klocalizedstring.h>

#include "loginfo.h"
#include "misc.h"
#include "tooltip.h"


class LogListViewItem : public QTreeWidgetItem
{
public:

    enum { Revision, Author, Date, Branch, Comment, Tags };

    LogListViewItem(QTreeWidget* list, const Cervisia::LogInfo& logInfo);

    bool operator<(const QTreeWidgetItem &other) const override;

private:
    static QString truncateLine(const QString &s);

    Cervisia::LogInfo m_logInfo;
    friend class LogListView;
};


LogListViewItem::LogListViewItem(QTreeWidget* list, const Cervisia::LogInfo& logInfo)
    : QTreeWidgetItem(list),
      m_logInfo(logInfo)
{
    setText(Revision, logInfo.m_revision);
    setText(Author, logInfo.m_author);
    setText(Date, logInfo.dateTimeToString());
    setText(Comment, truncateLine(logInfo.m_comment));

    for (Cervisia::LogInfo::TTagInfoSeq::const_iterator it = logInfo.m_tags.begin();
         it != logInfo.m_tags.end(); ++it)
    {
        const Cervisia::TagInfo& tagInfo(*it);

        if (tagInfo.m_type == Cervisia::TagInfo::OnBranch)
        {
            setText(Branch, tagInfo.m_name);
        }
    }

    setText(Tags, logInfo.tagsToString(Cervisia::TagInfo::Tag,
                                       Cervisia::LogInfo::NoTagType,
                                       QLatin1String(", ")));
}


QString LogListViewItem::truncateLine(const QString &s)
{
    int pos;

    QString res = s.simplified();
    if ( (pos = res.indexOf('\n')) != -1 )
        res = res.left(pos) + "...";

    return res;
}


bool LogListViewItem::operator<(const QTreeWidgetItem &other) const
{
    const LogListViewItem &item = static_cast<const LogListViewItem &>(other);

    switch ( treeWidget()->sortColumn() )
    {
    case Revision: return ::compareRevisions(m_logInfo.m_revision, item.m_logInfo.m_revision) == -1;
    case Date: return ::compare(m_logInfo.m_dateTime, item.m_logInfo.m_dateTime) == -1;
    }

    return QTreeWidgetItem::operator<(other);
}


LogListView::LogListView(KConfig& cfg, QWidget *parent)
    : QTreeWidget(parent)
    , partConfig(cfg)
{
    setAllColumnsShowFocus(true);
    header()->setSortIndicatorShown(true);
    setSelectionMode(QAbstractItemView::NoSelection);
    setRootIsDecorated(false);
    setSortingEnabled(true);
    sortByColumn(LogListViewItem::Revision, Qt::DescendingOrder);
    setHeaderLabels(QStringList() << i18n("Revision") << i18n("Author") << i18n("Date")
                                  << i18n("Branch") << i18n("Comment") << i18n("Tags"));

    Cervisia::ToolTip* toolTip = new Cervisia::ToolTip(viewport());

    connect(toolTip, SIGNAL(queryToolTip(QPoint,QRect&,QString&)),
            this, SLOT(slotQueryToolTip(QPoint,QRect&,QString&)));

    QByteArray state = cfg.group("LogList view").readEntry<QByteArray>("Columns", QByteArray());
    header()->restoreState(state);
}


LogListView::~LogListView()
{
    partConfig.group("LogList view").writeEntry("Columns", header()->saveState());
}


void LogListView::addRevision(const Cervisia::LogInfo& logInfo)
{
    (void) new LogListViewItem(this, logInfo);
}


void LogListView::setSelectedPair(const QString &selectionA, const QString &selectionB)
{
    for (int j = 0; j < topLevelItemCount(); j++)
    {
        LogListViewItem *i = static_cast<LogListViewItem*>(topLevelItem(j));
        i->setSelected(selectionA == i->text(LogListViewItem::Revision) ||
                       selectionB == i->text(LogListViewItem::Revision));
    }
}

void LogListView::mousePressEvent(QMouseEvent *e)
{
    // Retrieve selected item
    const LogListViewItem* selItem
        = static_cast<LogListViewItem*>(itemAt(e->pos()));
    if( !selItem )
        return;

    // Retrieve revision
    const QString revision = selItem->text(LogListViewItem::Revision);

    if ( e->button() == Qt::LeftButton )
    {
        // If the control key was pressed, then we change revision B not A
        if( e->modifiers() & Qt::ControlModifier )
            emit revisionClicked(revision, true);
        else
            emit revisionClicked(revision, false);
    }
    else if ( e->button() == Qt::MidButton )
        emit revisionClicked(revision, true);
}


void LogListView::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_A:
        if (currentItem())
            emit revisionClicked(currentItem()->text(LogListViewItem::Revision), false);
        break;
        break;
    case Qt::Key_B:
        if (currentItem())
            emit revisionClicked(currentItem()->text(LogListViewItem::Revision), true);
        break;
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
    case Qt::Key_Down:
    case Qt::Key_Up:
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_PageDown:
    case Qt::Key_PageUp:
        if (e->modifiers() == Qt::NoModifier)
             QTreeWidget::keyPressEvent(e);
        else
            QApplication::postEvent(this, new QKeyEvent(QEvent::KeyPress, e->key(), Qt::NoModifier, e->text()));
        break;
    default:
        // Ignore Key_Enter, Key_Return
        e->ignore();
    }
}


void LogListView::slotQueryToolTip(const QPoint& viewportPos,
                                   QRect&        viewportRect,
                                   QString&      text)
{
    if (const LogListViewItem* item = static_cast<LogListViewItem*>(itemAt(viewportPos)))
    {
        viewportRect = visualRect(indexAt(viewportPos));
        text = item->m_logInfo.createToolTipText();
    }
}


// Local Variables:
// c-basic-offset: 4
// End:
