/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2003-2004 Christian Loose <christian.loose@hamburg.de>
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

#include "logtree.h"

#include <QApplication>
#include <qpainter.h>
#include <QDebug>
#include <kcolorscheme.h>

#include "loginfo.h"
#include "tooltip.h"

#include <QHeaderView>


const int LogTreeView::BORDER = 5;
const int LogTreeView::INSPACE = 3;

namespace
{
    bool static_initialized = false;
    int static_width;
    int static_height;
}

class LogTreeItem
{
public:
    Cervisia::LogInfo m_logInfo;
    QString branchpoint;
    bool firstonbranch;
    int row;
    int col;
    SelectedRevision selected;
};


class LogTreeConnection
{
public:
    LogTreeItem *start;
    LogTreeItem *end;
};


LogTreeView::LogTreeView(QWidget *parent, const char *name)
    : QTableView(parent), rowCount(0), columnCount(1)
{
    setObjectName(QLatin1String(name));

    if (!static_initialized)
    {
        static_initialized = true;
        QFontMetrics fm( fontMetrics() );
        static_width = fm.width("1234567890") + 2*BORDER + 2*INSPACE;
        static_height = 2*fm.height() + 2*BORDER + 3*INSPACE;
    }

    setItemDelegate(new LogTreeDelegate(this));
    setModel(model = new LogTreeModel(this));

    setSelectionMode(QAbstractItemView::NoSelection);
    setShowGrid(false);
    horizontalHeader()->hide();
    verticalHeader()->hide();
    setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
    setBackgroundRole( QPalette::Base );
    setFocusPolicy(Qt::NoFocus);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    Cervisia::ToolTip* toolTip = new Cervisia::ToolTip(viewport());

    connect(toolTip, SIGNAL(queryToolTip(QPoint,QRect&,QString&)),
            this, SLOT(slotQueryToolTip(QPoint,QRect&,QString&)));

    connect(this, SIGNAL(pressed(QModelIndex)),
            this, SLOT(mousePressed(QModelIndex)));
}


LogTreeView::~LogTreeView()
{
    qDeleteAll(items);
    qDeleteAll(connections);
}


void LogTreeView::addRevision(const Cervisia::LogInfo& logInfo)
{
    QString branchpoint, branchrev;

    const QString rev(logInfo.m_revision);

    // find branch
    int pos1, pos2;
    if ((pos2 = rev.lastIndexOf('.')) > 0 &&
        (pos1 = rev.lastIndexOf('.', pos2-1)) > 0)
    {
        // e. g. for rev = 1.1.2.3 we have
        // branchrev = 1.1.2, branchpoint = 1.1
        branchrev = rev.left(pos2);
        branchpoint = rev.left(pos1);
    }

    if (branchrev.isEmpty())
    {
        // Most probably we are on the trunk
        model->beginInsertRows(QModelIndex(), rowCount, rowCount);
        rowCount++;
        LogTreeItem *item = new LogTreeItem;
        item->m_logInfo = logInfo;
        item->branchpoint = branchpoint;
        item->firstonbranch = false;
        item->row = rowCount-1;
        item->col = 0;
        item->selected = NoRevision;
        items.append(item);
        model->endInsertRows();
        return;
    }

    // look whether we have revisions on this branch
    // shift them up
    int row=-1, col=-1;
    foreach (LogTreeItem* item1, items)
    {
        if (branchrev == (item1->m_logInfo.m_revision).left(branchrev.length()))
        {
            item1->firstonbranch = false;
            row = item1->row;
            col = item1->col;
            item1->row--;
            // Are we at the top of the widget?
            if (row == 0)
            {
                foreach (LogTreeItem* item2, items)
                    item2->row++;
                model->beginInsertRows(QModelIndex(), rowCount, rowCount);
                rowCount++;
                row = 1;
                model->endInsertRows();
            }
        }
    }

    if (row == -1)
    {
        // Ok, so we must open a new branch
        // Let's find the branch point
        QListIterator<LogTreeItem*> iter(items);
        iter.toBack();
        while (iter.hasPrevious())
        {
            LogTreeItem* item1(iter.previous());
            if (branchpoint == item1->m_logInfo.m_revision)
            {
                // Move existing branches to the right
                foreach (LogTreeItem* item2, items)
                    if (item2->col > item1->col)
                        item2->col++;
                model->beginInsertColumns(QModelIndex(), columnCount, columnCount);
                columnCount++;
                row = item1->row-1;
                col = item1->col+1;
                if (row == -1)
                {
                    foreach (LogTreeItem* item3, items)
                        item3->row++;
                    model->beginInsertRows(QModelIndex(), rowCount, rowCount);
                    rowCount++;
                    row = 0;
                    model->endInsertRows();
                }
                model->endInsertColumns();
                break;
            }
        }
    }

    LogTreeItem *item = new LogTreeItem;
    item->m_logInfo = logInfo;
    item->branchpoint = branchpoint;
    item->firstonbranch = true;
    item->row = row;
    item->col = col;
    item->selected = NoRevision;
    items.append(item);

#if 0
    qCDebug(log_cervisia) << "Dump:";
    qCDebug(log_cervisia) << "Rows:" << numRows() << "Cols:" << numCols();
    foreach (LogTreeItem* treeItem, items)
    {
        qCDebug(log_cervisia) << "Rev:" << treeItem->m_logInfo.m_revision;
        qCDebug(log_cervisia) << "row:" << treeItem->row << ", col:" << treeItem->col;
        qCDebug(log_cervisia) << "fob:" << treeItem->firstonbranch;
    }
    qCDebug(log_cervisia) << "End Dump";
#endif
}


void LogTreeView::collectConnections()
{
    for (LogTreeItemList::const_iterator it(items.begin()),
                                         itEnd(items.end());
         it != itEnd; ++it)
    {
        QString rev = (*it)->m_logInfo.m_revision;

        LogTreeItemList::const_iterator it2(it);
        ++it2;
        for (; it2 != itEnd; ++it2)
            if ((*it2)->branchpoint == rev &&
                (*it2)->firstonbranch)
            {
                LogTreeConnection *conn = new LogTreeConnection;
                conn->start = (*it);
                conn->end = (*it2);
                connections.append(conn);
            }
    }
}


void LogTreeView::setSelectedPair(QString selectionA, QString selectionB)
{
    foreach (LogTreeItem* item, items)
    {
        const SelectedRevision oldSelection = item->selected;
        SelectedRevision newSelection;

        if ( selectionA == item->m_logInfo.m_revision )
            newSelection = RevisionA;
        else if ( selectionB == item->m_logInfo.m_revision )
            newSelection = RevisionB;
        else
            newSelection = NoRevision;

        if (oldSelection != newSelection)
        {
            item->selected = newSelection;
            viewport()->update();
        }
    }
}


QSize LogTreeView::sizeHint() const
{
    return QSize(2 * static_width, 3 * static_height);
}


QString LogTreeView::text(int row, int col) const
{
    LogTreeItem* item = 0;

    foreach (LogTreeItem* treeItem, items)
    {
        if( treeItem->col == col && treeItem->row == row )
        {
            item = treeItem;
            break;
        }
    }

    QString text;

    if( item && !item->m_logInfo.m_author.isNull() )
        text = item->m_logInfo.createToolTipText();

    return text;
}


void LogTreeView::paintCell(QPainter *p, int row, int col)
{
    bool followed, branched;
    LogTreeItem *item;

    branched = false;
    followed = false;
    item = 0;

    foreach (LogTreeItem* treeItem, items)
    {
        int itcol = treeItem->col;
        int itrow = treeItem->row;
        if (itrow == row-1 && itcol == col)
            followed = true;
        if (itrow == row && itcol == col)
            item = treeItem;
    }
    foreach (LogTreeConnection* connection, connections)
    {
        int itcol1 = connection->start->col;
        int itcol2 = connection->end->col;
        int itrow = connection->start->row;
        if (itrow == row && itcol1 <= col && itcol2 > col)
            branched = true;
    }

    if (item)
        paintRevisionCell(p, row, col, item->m_logInfo,
                          followed, branched, item->selected);
    else if (followed || branched)
        paintConnector(p, row, col, followed, branched);
}


void LogTreeView::paintConnector(QPainter *p,
                                 int row, int col, bool followed, bool branched)
{
    const int midx = columnWidth(col) / 2;
    const int midy = rowHeight(row) / 2;

    p->drawLine(0, midy, branched ? columnWidth(col) : midx, midy);
    if (followed)
        p->drawLine(midx, midy, midx, 0);
}


QSize LogTreeView::computeSize(const Cervisia::LogInfo& logInfo,
                               int*                     authorHeight,
                               int*                     tagsHeight) const
{
    const QFontMetrics fm(fontMetrics());

    const QString tags(logInfo.tagsToString(Cervisia::TagInfo::Branch | Cervisia::TagInfo::Tag,
                                            Cervisia::TagInfo::Branch));

    const QSize r1 = fm.size(Qt::AlignCenter, logInfo.m_revision);
    const QSize r3 = fm.size(Qt::AlignCenter, logInfo.m_author);

    if (authorHeight)
        *authorHeight = r3.height();

    int infoWidth = qMax(static_width - 2 * BORDER, qMax(r1.width(), r3.width()));
    int infoHeight = r1.height() + r3.height() + 3 * INSPACE;

    if (!tags.isEmpty())
    {
        const QSize r2 = fm.size(Qt::AlignCenter, tags);
        infoWidth = qMax(infoWidth, r2.width());
        infoHeight += r2.height() + INSPACE;
        if (tagsHeight)
            *tagsHeight = r2.height();
    }
    else
    {
        if (tagsHeight)
            *tagsHeight = 0;
    }
    infoWidth += 2 * INSPACE;

    return QSize(infoWidth, infoHeight);
}


void LogTreeView::paintRevisionCell(QPainter *p,
                                    int row, int col,
                                    const Cervisia::LogInfo& logInfo,
                                    bool followed, bool branched, SelectedRevision selected)
{
    int authorHeight;
    int tagsHeight;
    const QSize infoSize(computeSize(logInfo, &authorHeight, &tagsHeight));
    const QSize cellSize(columnWidth(col), rowHeight(row));

    const int midx(cellSize.width() / 2);
    const int midy(cellSize.height() / 2);

    QRect rect(QPoint((cellSize.width() - infoSize.width()) / 2,
                      (cellSize.height() - infoSize.height()) / 2),
               infoSize);

    // Connectors
    if (followed)
        p->drawLine(midx, 0, midx, rect.y()); // to the top

    if (branched)
        p->drawLine(rect.x() + infoSize.width(), midy, cellSize.width(), midy); // to the right

    p->drawLine(midx, rect.y() + infoSize.height(), midx, cellSize.height()); // to the bottom

    // The box itself
    if (selected)
    {
        if ( selected == RevisionA )
        {
            p->fillRect(rect, KColorScheme(QPalette::Active, KColorScheme::Selection).background());
            p->setPen(KColorScheme(QPalette::Active, KColorScheme::Selection).foreground().color());
            p->drawText(rect, Qt::AlignLeft | Qt::AlignTop, "A");
        }
        else
        {
            p->fillRect(rect, KColorScheme(QPalette::Active, KColorScheme::Selection)
                .background().color().lighter(130));
            p->setPen(KColorScheme(QPalette::Active, KColorScheme::Selection)
                .foreground().color().lighter(130));
            p->drawText(rect, Qt::AlignLeft | Qt::AlignTop, "B");
        }
    }
    else
    {
        p->drawRoundedRect(rect, 10, 10);
    }

    rect.setY(rect.y() + INSPACE);

    p->drawText(rect, Qt::AlignHCenter, logInfo.m_author);
    rect.setY(rect.y() + authorHeight + INSPACE);

    const QString tags(logInfo.tagsToString(Cervisia::TagInfo::Branch | Cervisia::TagInfo::Tag,
                                            Cervisia::TagInfo::Branch));
    if (!tags.isEmpty())
    {
        const QFont font(p->font());
        QFont underline(font);
        underline.setUnderline(true);

        p->setFont(underline);
        p->drawText(rect, Qt::AlignHCenter, tags);
        p->setFont(font);

        rect.setY(rect.y() + tagsHeight + INSPACE);
    }

    p->drawText(rect, Qt::AlignHCenter, logInfo.m_revision);
}


void LogTreeView::mousePressed(const QModelIndex &index)
{
    Qt::MouseButtons buttons = QApplication::mouseButtons();

    if ( buttons == Qt::MidButton ||
         buttons == Qt::LeftButton)
    {
        int row = index.row();
        int col = index.column();

        foreach (LogTreeItem* item, items)
        {
            if (item->row == row && item->col == col)
            {
                // Change selection for revision B if the middle mouse button or
                // the left mouse button with the control key was pressed
                bool changeRevB = (buttons == Qt::MidButton) ||
                                  (buttons == Qt::LeftButton &&
                                   QApplication::keyboardModifiers() & Qt::ControlModifier);

                emit revisionClicked(item->m_logInfo.m_revision, changeRevB);
                viewport()->update();
                break;
            }
        }
    }
}


void LogTreeView::recomputeCellSizes ()
{
    // Compute maximum for each column and row
    foreach (const LogTreeItem* item, items)
    {
        const QSize cellSize(computeSize(item->m_logInfo) + QSize(2 * BORDER,  2 * BORDER));

        setColumnWidth(item->col, qMax(columnWidth(item->col), cellSize.width()));
        setRowHeight(item->row, qMax(rowHeight(item->row), cellSize.height()));
    }

    viewport()->update();
}


void LogTreeView::slotQueryToolTip(const QPoint& viewportPos,
                                   QRect&        viewportRect,
                                   QString&      tipText)
{
    QModelIndex index(indexAt(viewportPos));

    tipText = text(index.row(), index.column());
    if (tipText.isEmpty())
        return;

    viewportRect = visualRect(index);
}

//---------------------------------------------------------------------

void LogTreeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    painter->translate(option.rect.x(), option.rect.y());
    logView->paintCell(painter, index.row(), index.column());

    painter->restore();
}



// Local Variables:
// c-basic-offset: 4
// End:
