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

#include <qpainter.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>

#include "loginfo.h"
#include "tooltip.h"


const int LogTreeView::BORDER = 8;
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
    bool selected;
};


class LogTreeConnection
{
public:
    LogTreeItem *start;
    LogTreeItem *end;
};


LogTreeView::LogTreeView(QWidget *parent, const char *name)
    : QTable(parent, name)
{
    if (!static_initialized)
    {
        static_initialized = true;
        QFontMetrics fm( fontMetrics() );
        static_width = fm.width("1234567890") + 2*BORDER + 2*INSPACE;
        static_height = 2*fm.height() + 2*BORDER + 3*INSPACE;
    }

    setNumCols(0);
    setNumRows(0);
    setReadOnly(true);
    setFocusStyle(QTable::FollowStyle);
    setSelectionMode(QTable::NoSelection);
    setShowGrid(false);
    horizontalHeader()->hide();
    setTopMargin(0);
    verticalHeader()->hide();
    setLeftMargin(0);
    setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
    setBackgroundMode(PaletteBase);
    setFocusPolicy(NoFocus);

    currentRow = -1;
    currentCol = -1;

    items.setAutoDelete(true);
    connections.setAutoDelete(true);

    Cervisia::ToolTip* toolTip = new Cervisia::ToolTip(viewport());

    connect(toolTip, SIGNAL(queryToolTip(const QPoint&, QRect&, QString&)),
            this, SLOT(slotQueryToolTip(const QPoint&, QRect&, QString&)));
}


void LogTreeView::addRevision(const Cervisia::LogInfo& logInfo)
{
    QString branchpoint, branchrev;

    const QString rev(logInfo.m_revision);

    // find branch
    int pos1, pos2;
    if ((pos2 = rev.findRev('.')) > 0 &&
        (pos1 = rev.findRev('.', pos2-1)) > 0)
    {
        // e. g. for rev = 1.1.2.3 we have
        // branchrev = 1.1.2, branchpoint = 1.1
        branchrev = rev.left(pos2);
        branchpoint = rev.left(pos1);
    }

    if (branchrev.isEmpty())
    {
        // Most probably we are on the trunk
        setNumRows(numRows()+1);
        setNumCols(1);
        LogTreeItem *item = new LogTreeItem;
        item->m_logInfo = logInfo;
        item->branchpoint = branchpoint;
        item->firstonbranch = false;
        item->row = numRows()-1;
        item->col = 0;
        item->selected = false;
        items.append(item);
        return;
    }

    // look whether we have revisions on this branch
    // shift them up
    int row=-1, col=-1;
    QPtrListIterator<LogTreeItem> it(items);
    for (; it.current(); ++it)
    {
        if (branchrev == (it.current()->m_logInfo.m_revision).left(branchrev.length()))
        {
            it.current()->firstonbranch = false;
            row = it.current()->row;
            col = it.current()->col;
            it.current()->row--;
            // Are we at the top of the widget?
            if (row == 0)
            {
                QPtrListIterator<LogTreeItem> it2(items);
                for (; it2.current(); ++it2)
                    it2.current()->row++;
                setNumRows(numRows()+1);
                row = 1;
            }
        }
    }

    if (row == -1)
    {
        // Ok, so we must open a new branch
        // Let's find the branch point
        QPtrListIterator<LogTreeItem> it3(items);
        for (it3.toLast(); it3.current(); --it3)
        {
            if (branchpoint == it3.current()->m_logInfo.m_revision)
            {
                // Move existing branches to the right
                QPtrListIterator<LogTreeItem> it4(items);
                for (; it4.current(); ++it4)
                    if (it4.current()->col > it3.current()->col)
                    {
                        it4.current()->col++;
                    }
                setNumCols(numCols()+1);
                row = it3.current()->row-1;
                col = it3.current()->col+1;
                if (row == -1)
                {
                    QPtrListIterator<LogTreeItem> it5(items);
                    for (; it5.current(); ++it5)
                        it5.current()->row++;
                    setNumRows(numRows()+1);
                    row = 0;
                }
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
    item->selected = false;
    items.append(item);

#if 0
    cout << "Dump: " << endl;
    cout << "Rows: " << numRows() << "Cols: " << numCols() << endl;
    QPtrListIterator<LogTreeItem> it5(items);
    for (; it5.current(); ++it5)
    {
        cout << "Rev: "<< it5.current()->rev << endl;
        cout << "row: "<< it5.current()->row << ", col: " << it5.current()->col << endl;
        cout << "fob: "<< it5.current()->firstonbranch << endl;
    }
    cout << "End Dump" << endl;
#endif

}


void LogTreeView::collectConnections()
{
    QPtrListIterator<LogTreeItem> it(items);
    for (; it.current(); ++it)
    {
        QString rev = it.current()->m_logInfo.m_revision;

        QPtrListIterator<LogTreeItem> it2(items);
        for (it2=it,++it2; it2.current(); ++it2)
            if (it2.current()->branchpoint == rev &&
                it2.current()->firstonbranch)
            {
                LogTreeConnection *conn = new LogTreeConnection;
                conn->start = it.current();
                conn->end = it2.current();
                connections.append(conn);
            }
    }
}


void LogTreeView::setSelectedPair(QString selectionA, QString selectionB)
{
    QPtrListIterator<LogTreeItem> it(items);
    for(; it.current(); ++it)
    {
        bool oldstate = it.current()->selected;
        bool newstate = ( selectionA == it.current()->m_logInfo.m_revision ||
                          selectionB == it.current()->m_logInfo.m_revision );
        if (oldstate != newstate)
        {
            it.current()->selected = newstate;
            repaint(false);
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
    
    QPtrListIterator<LogTreeItem> it(items);
    for( ; it.current(); ++it )
    {
        if( it.current()->col == col && it.current()->row == row )
        {
            item = it.current();
            break;
        }
    }
    
    QString text;
    
    if( item && !item->m_logInfo.m_author.isNull() )
        text = item->m_logInfo.createToolTipText();
        
    return text;
}


void LogTreeView::paintCell(QPainter *p, int row, int col, const QRect& cr,
                            bool selected, const QColorGroup& cg)
{
    Q_UNUSED(selected)
    Q_UNUSED(cr)
    bool followed, branched;
    LogTreeItem *item;

    branched = false;
    followed = false;
    item = 0;

    QPtrListIterator<LogTreeItem> it(items);
    for(; it.current(); ++it)
    {
        int itcol = it.current()->col;
        int itrow = it.current()->row;
        if (itrow == row-1 && itcol == col)
            followed = true;
        if (itrow == row && itcol == col)
            item = it.current();
    }
    QPtrListIterator<LogTreeConnection> it2(connections);
    for (; it2.current(); ++it2)
    {
        int itcol1 = it2.current()->start->col;
        int itcol2 = it2.current()->end->col;
        int itrow = it2.current()->start->row;
        if (itrow == row && itcol1 <= col && itcol2 > col)
            branched = true;
    }

    p->fillRect(0, 0, columnWidth(col), rowHeight(row),
                cg.base());
    p->setPen(cg.text());
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

    const QSize r1 = fm.size(AlignCenter, logInfo.m_revision);
    const QSize r3 = fm.size(AlignCenter, logInfo.m_author);

    if (authorHeight)
        *authorHeight = r3.height();

    int infoWidth = kMax(static_width - 2 * BORDER, kMax(r1.width(), r3.width()));
    int infoHeight = r1.height() + r3.height() + 3 * INSPACE;

    if (!tags.isEmpty())
    {
        const QSize r2 = fm.size(AlignCenter, tags);
        infoWidth = kMax(infoWidth, r2.width());
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
                                    bool followed, bool branched, bool selected)
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
        p->fillRect(rect, KGlobalSettings::highlightColor());
        p->setPen(KGlobalSettings::highlightedTextColor());
    }
    else
    {
        p->drawRoundRect(rect, 10, 10);
    }

    rect.setY(rect.y() + INSPACE);

    p->drawText(rect, AlignHCenter, logInfo.m_author);
    rect.setY(rect.y() + authorHeight + INSPACE);

    const QString tags(logInfo.tagsToString(Cervisia::TagInfo::Branch | Cervisia::TagInfo::Tag,
                                            Cervisia::TagInfo::Branch));
    if (!tags.isEmpty())
    {
        const QFont font(p->font());
        QFont underline(font);
        underline.setUnderline(true);

        p->setFont(underline);
        p->drawText(rect, AlignHCenter, tags);
        p->setFont(font);

        rect.setY(rect.y() + tagsHeight + INSPACE);
    }

    p->drawText(rect, AlignHCenter, logInfo.m_revision);
}


void LogTreeView::contentsMousePressEvent(QMouseEvent *e)
{
    if ( e->button() == MidButton ||
         e->button() == LeftButton)
    {
        int row = rowAt( e->pos().y() );
        int col = columnAt( e->pos().x() );

        QPtrListIterator<LogTreeItem> it(items);
        for(; it.current(); ++it)
            if (it.current()->row == row
                && it.current()->col == col)
            {
                // Change selection for revision B if the middle mouse button or
                // the left mouse button with the control key was pressed
                bool changeRevB = (e->button() == MidButton) ||
                                  (e->button() == LeftButton &&
                                   e->state() & ControlButton);

                emit revisionClicked(it.current()->m_logInfo.m_revision, changeRevB);
                break;
            }
    }

    viewport()->update();
}


void LogTreeView::recomputeCellSizes ()
{
    // Compute maximum for each column and row
    for (QPtrListIterator<LogTreeItem> it(items); it.current(); ++it)
    {
        const LogTreeItem *item = it.current();

        const QSize cellSize(computeSize(item->m_logInfo) + QSize(2 * BORDER,  2 * BORDER));

        setColumnWidth(item->col, kMax(columnWidth(item->col), cellSize.width()));
        setRowHeight(item->row, kMax(rowHeight(item->row), cellSize.height()));
    }

    viewport()->update();
}


void LogTreeView::slotQueryToolTip(const QPoint& viewportPos,
                                   QRect&        viewportRect,
                                   QString&      tipText)
{
    const QPoint contentsPos(viewportToContents(viewportPos));
    const int column(columnAt(contentsPos.x()));
    const int row(rowAt(contentsPos.y()));

    tipText = text(row, column);
    if (tipText.isEmpty())
        return;

    viewportRect = cellGeometry(row, column);
    viewportRect.moveTopLeft(contentsToViewport(viewportRect.topLeft()));
}


#include "logtree.moc"


// Local Variables:
// c-basic-offset: 4
// End:
