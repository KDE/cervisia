/*
 *  Copyright (C) 1999-2001 Bernd Gehrmann
 *                          bernd@physik.hu-berlin.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include <qtooltip.h>
#include <qpainter.h>
#include <qapp.h>
#include "tiplabel.h"
#include "misc.h"

#include "logtree.h"
#include "logtree.moc"


static const int BORDER = 8;
static const int INSPACE = 3;

static bool static_initialized = false;
static int  static_width;
static int  static_height;


class LogTreeItem
{
public:
    QString rev;
    QString author;
    QString date;
    QString comment;
    QString tagcomment;
    QString taglist;
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
    : QTableView(parent, name)
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
    setAutoUpdate(false);
    setTableFlags( Tbl_autoVScrollBar|Tbl_autoHScrollBar|
		   Tbl_smoothVScrolling | Tbl_smoothHScrolling );
    setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
    setBackgroundMode(PaletteBase);
    setMouseTracking(true);
    setFocusPolicy(ClickFocus);

    setCellWidth(0);
    setCellHeight(0);

    qApp->installEventFilter(this);
    currentRow = -1;
    currentCol = -1;
    currentLabel = 0;

    items.setAutoDelete(true);
    connections.setAutoDelete(true);
}


LogTreeView::~LogTreeView()
{
    qApp->installEventFilter(this);
}


void LogTreeView::addRevision(const QString &rev, const QString &author, const QString &date,
                              const QString &comment, const QString &taglist,
                              const QString &tagcomment)
{
    QString branchpoint, branchrev;

    branchrev = "";
    branchpoint = "";

    // find branch
    int pos1, pos2;
    if ((pos2 = rev.findRev('.')) > 0 &&
        (pos1 = rev.findRev('.', pos2-1)) > 0)
        {
            // e. g. for rev = 1.1.2.3 we have
            // branch = 1.1.2, rev2 = 1.1
            branchrev = rev.left(pos2);
            branchpoint = rev.left(pos1);
        }

    if (branchrev.isEmpty())
        {
            // Most probably we are on the trunk
            setNumRows(numRows()+1);
            setNumCols(1);
            LogTreeItem *item = new LogTreeItem;
            item->rev = rev;
            item->author = author;
            item->date = date;
            item->comment = comment;
            item->tagcomment = tagcomment;
            item->taglist = taglist;
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
            if (branchrev == (it.current()->rev).left(branchrev.length()))
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
                    if (branchpoint == it3.current()->rev)
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
    item->rev = rev;
    item->author = author;
    item->date = date;
    item->comment = comment;
    item->tagcomment = tagcomment;
    item->taglist = taglist;
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
            QString rev = it.current()->rev;

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
            bool newstate = ( selectionA == it.current()->rev ||
                              selectionB == it.current()->rev );
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


void LogTreeView::setupPainter(QPainter *p)
{
    p->setBackgroundColor(colorGroup().base());
}


void LogTreeView::paintCell(QPainter *p, int row, int col)
{
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

    p->fillRect(0, 0, cellWidth(col), cellHeight(row),
                colorGroup().base());
    p->setPen(colorGroup().text());
    if (item)
        paintRevisionCell(p, row, col, item->author, item->taglist, item->rev, followed, branched, item->selected);
    else if (followed || branched)
        paintConnector(p, row, col, followed, branched);
}


void LogTreeView::paintConnector(QPainter *p,
				 int row, int col, bool followed, bool branched)
{
    int midx = colWidths[col] / 2;
    int midy = rowHeights[row] / 2;

    p->drawLine(0, midy, branched ? colWidths[col] : midx, midy);
    if (followed)
        p->drawLine(midx, midy, midx, 0);
}


void LogTreeView::paintRevisionCell(QPainter *p,
                                    int row, int col,
                                    QString line1, QString line2, QString line3,
                                    bool followed, bool branched, bool selected)
{
    QFontMetrics fm(p->fontMetrics());

    QSize r1 = fm.size(AlignCenter, line1);
    QSize r2 = fm.size(AlignCenter, line2);
    QSize r3 = fm.size(AlignCenter, line3);

    int boxwidth, boxheight;
    boxwidth = QMAX(static_width-2*BORDER, QMAX(r1.width(), r3.width()));
    boxheight = r1.height() + r3.height() + 3*INSPACE;

    if (!line2.isEmpty())
        {
            boxwidth = QMAX(boxwidth, r2.width());
            boxheight += r2.height() + INSPACE;
        }
    boxwidth += 2*INSPACE;

    int x = (colWidths[col] - boxwidth) / 2;
    int midx = colWidths[col] / 2;
    int y = (rowHeights[row] - boxheight) / 2;
    int midy = rowHeights[row] / 2;

    // Connectors
    if (followed)
        p->drawLine(midx, 0, midx, y);

    if (branched)
        p->drawLine(midx + boxwidth / 2, midy, colWidths[col], midy);

    p->drawLine(midx, y + boxheight, midx, rowHeights[row]);

    // The box itself
    if (selected)
        {
#if QT_VERSION < 300
            if (style().guiStyle() == WindowsStyle)
                {
                    p->fillRect(x, y, boxwidth, boxheight, QApplication::winStyleHighlightColor());
                    p->setPen(white);
                }
            else
                {
                    p->fillRect(x, y, boxwidth, boxheight, colorGroup().text());
                    p->setPen(colorGroup().base());
                }
#else
	    // XXX: This is a hack
	    p->fillRect(x, y, boxwidth, boxheight, QApplication::winStyleHighlightColor());
	    p->setPen(white);

#endif
        }
    else
        {
            p->drawRoundRect(x, y, boxwidth, boxheight, 10, 10);
        }

    x += INSPACE;
    y += INSPACE;
    boxwidth -= 2*INSPACE;

    p->drawText(x, y, boxwidth, boxheight, AlignHCenter, line1);
    y += r1.height() + INSPACE;

    if (!line2.isEmpty())
        {
            QFont font(p->font());
            QFont underline(font);

            underline.setUnderline(true);
            p->setFont(underline);
            p->drawText(x, y, boxwidth, boxheight, AlignHCenter, line2);
            p->setFont(font);
            y += r2.height() + INSPACE;
        }

    p->drawText(x, y, boxwidth, boxheight, AlignHCenter, line3);
}


void LogTreeView::mousePressEvent(QMouseEvent *e)
{
    if ( e->button() == MidButton ||
         e->button() == LeftButton)
	{
	    int row = findRow( e->pos().y() );
	    int col = findCol( e->pos().x() );

	    QPtrListIterator<LogTreeItem> it(items);
	    for(; it.current(); ++it)
		if (it.current()->row == row
		    && it.current()->col == col)
		    {
			emit revisionClicked(it.current()->rev,
					     e->button() == MidButton);
			break;
		    }
	}
}

void LogTreeView::hideLabel()
{
    if (currentLabel)
        currentLabel->hide();
    delete currentLabel;
    currentLabel = 0;
}

void LogTreeView::windowActivationChange( bool )
{
    hideLabel();
}

bool LogTreeView::eventFilter(QObject *o, QEvent *e)
{
    if (o != this || e->type() != QEvent::MouseMove || !isActiveWindow())
        return QTableView::eventFilter(o, e);

    int row = findRow(static_cast<QMouseEvent*>(e)->y());
    int col = findCol(static_cast<QMouseEvent*>(e)->x());
    if (row != currentRow || col != currentCol)
        {
            hideLabel();
        }

    LogTreeItem *item = 0;

    QPtrListIterator<LogTreeItem> it(items);
    for(; it.current(); ++it)
        if (it.current()->row == row && it.current()->col == col)
            {
                item = static_cast<LogTreeItem*>(it.current());
                break;
            }

    if (!currentLabel && item)
        {
            if (true)
            //            if (!item->author.isNull())
                {
                    QString text = "<qt><b>";
                    text += item->rev;
                    text += "</b>&nbsp;&nbsp;";
                    text += item->author;
                    text += "&nbsp;&nbsp;<b>";
                    text += item->date;
                    text += "</b>";
                    QStringList list2 = QStringList::split("\n", item->comment);
                    QStringList::Iterator it2;
                    for (it2 = list2.begin(); it2 != list2.end(); ++it2)
                        {
                            text += "<br>";
                            text += (*it2);
                        }
                    if (!item->tagcomment.isEmpty())
                        {
                            text += "<i>";
                            QStringList list3 = QStringList::split("\n", item->tagcomment);
                            QStringList::Iterator it3;
                            for (it3 = list3.begin(); it3 != list3.end(); ++it3)
                                {
                                    text += "<br>";
                                    text += (*it3);
                                }
                            text += "</i>";
                        }
                    text += "</qt>";
                    int left; colXPos(col, &left); left += cellWidth(col);
                    int top = static_cast<QMouseEvent*>(e)->y();
                    currentLabel = new TipLabel(text);
                    currentLabel->showAt(mapToGlobal(QPoint(left, top)));
                    currentRow = row;
                    currentCol = col;
                }
        }

    return QTableView::eventFilter(o, e);
}


void LogTreeView::recomputeCellSizes ()
{
    // Fill with default
    int v = static_width;
    colWidths.fill(v, numCols());
    v = static_height;
    rowHeights.fill(v, numRows());

    QFontMetrics fm(fontMetrics());

    // Compute maximum for each column and row
    QPtrListIterator<LogTreeItem> it(items);
    for (; it.current(); ++it)
        {
            LogTreeItem *item = it.current();

            QSize r1 = fm.size(AlignCenter, item->rev);
            QSize r2 = fm.size(AlignCenter, item->taglist);
            QSize r3 = fm.size(AlignCenter, item->author);

            int boxwidth = QMAX(r1.width(), r3.width());
            int boxheight = r1.height() + r3.height() + 3*INSPACE;

            if (!item->taglist.isEmpty())
                {
                    boxwidth = QMAX(boxwidth, r2.width());
                    boxheight += r2.height() + INSPACE;
                }
            boxwidth += 2*INSPACE;

            colWidths[item->col] = QMAX(colWidths[item->col], boxwidth + 2*BORDER);
            rowHeights[item->row] = QMAX(rowHeights[item->row], boxheight + 2*BORDER);
	}

    setAutoUpdate(true);
    updateTableSize();
    update();
}


int LogTreeView::cellWidth(int col)
{
    if (col < 0 || col >= (int)colWidths.size())
        return 0;

    return colWidths[col];
}


int LogTreeView::cellHeight(int row)
{
    if (row < 0 || row >= (int)rowHeights.size())
        return 0;

    return rowHeights[row];
}


// Local Variables:
// c-basic-offset: 4
// End:
