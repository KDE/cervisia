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


#include <qpainter.h>
#include <kapp.h>
#include <kconfig.h>

#include "tiplabel.h"
#include "misc.h"
#include "cervisiapart.h"

#include "annotateview.h"
#include "annotateview.moc"


static const int BORDER = 4;


class AnnotateViewItem
{
public:
    QString rev;
    QString author;
    QString date;
    QString content;
    QString comment;
    bool odd;
};
    

AnnotateView::AnnotateView( QWidget *parent, const char *name )
    : QTableView(parent, name, WNorthWestGravity | WRepaintNoErase | WResizeNoErase)
{
    setNumRows(0);
    setNumCols(3);
    setTableFlags( Tbl_autoVScrollBar|Tbl_autoHScrollBar|
		   Tbl_smoothVScrolling );
    setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
    setBackgroundMode(PaletteBase);
    setMouseTracking(true);

    KConfig *config = CervisiaPart::config();
    config->setGroup("LookAndFeel");
    setFont(config->readFontEntry("AnnotateFont"));
    QFontMetrics fm(font());

    setCellHeight(fm.lineSpacing());
    setCellWidth(0);

    currentRow = -1;
    currentLabel = 0;
    
    items.setAutoDelete(true);
}


AnnotateView::~AnnotateView()
{
    delete currentLabel;
}


void AnnotateView::setFont(const QFont &font)
{
    QTableView::setFont(font);
    QFontMetrics fm(font);
    setCellHeight(fm.lineSpacing());
}


void AnnotateView::addLine(const QString &rev, const QString &author, const QString &date,
                           const QString &content, const QString &comment, bool odd)
{
    AnnotateViewItem *item = new AnnotateViewItem;
    item->rev = rev;
    item->author = author;
    item->date = date;
    item->content = content;
    item->comment = comment;
    item->odd = odd;
    items.append(item);
    setNumRows(numRows()+1);
}


int AnnotateView::cellWidth(int col)
{
    QFontMetrics fm(font());

    switch (col)
	{
        case 0:  return fm.width("10000");
	case 1:  return fm.width("1.0.1.0.1 gehrmab");
        case 2:  { int rest = cellWidth(0)+cellWidth(1); return viewWidth()-rest; }
	default: return 0; // should not occur
	}
}


QSize AnnotateView::sizeHint() const
{
    QFontMetrics fm(font());
    AnnotateView *that = const_cast<AnnotateView*>(this);
    return QSize( that->cellWidth(0) + that->cellWidth(1) + 8*fm.width("0123456789"),
		  fm.lineSpacing()*8 );
}


void AnnotateView::paintCell(QPainter *p, int row, int col)
{
    AnnotateViewItem *item = items.at(row);

    int width = cellWidth(col);
    int height = cellHeight();

    QColor backgroundColor;
    int innerborder;
    QString str;
    
    if (col == 0)
	{
	    backgroundColor = QColor(222, 222, 222);
	    innerborder = 0;
	    str.setNum(row+1);
	}
    else if (col == 1)
	{
	    backgroundColor = item->odd? white : lightGray;
	    innerborder = BORDER;
            if (!item->author.isNull())
                {
                    str = item->author;
                    str += " ";
                    str += item->rev;
                }
            else
                str = "";
	}
    else // col == 3
	{
	    backgroundColor = item->odd? white : lightGray;
	    innerborder = 0;
	    str = item->content;
	}
    
    p->fillRect(0, 0, width, height, backgroundColor);
    p->drawText(innerborder, 0, width-2*innerborder, height, AlignLeft, str);
}    


void AnnotateView::mouseMoveEvent(QMouseEvent *e)
{
    int row = findRow(e->y());
    int col = findCol(e->x());
    if (row != currentRow || col != 1)
        {
            if (currentLabel)
                currentLabel->hide();
            delete currentLabel;
            currentLabel = 0;
        }
    if (!currentLabel && row != -1 && col == 1)
        {
            AnnotateViewItem *item = items.at(row);
            if (!item->author.isNull())
                {
                    QString text = "<qt><b>";
                    text += item->rev;
                    text += "</b>&nbsp;&nbsp;";
                    text += item->author;
                    text += "&nbsp;&nbsp;<b>";
                    text += item->date;
                    text += "</b>";
                    QStringList list = QStringList::split("\n", item->comment);
                    QStringList::Iterator it;
                    for (it = list.begin(); it != list.end(); ++it)
                        {
                            text += "<br>";
                            text += (*it);
                        }
                    text += "</qt>";
                    
                    int left; colXPos(2, &left);
                    int top; rowYPos(row, &top);
                    currentLabel = new TipLabel(text);
                    currentLabel->showAt(mapToGlobal(QPoint(left, top)));
                    currentRow = row;
                }
        }
    
    QTableView::mouseMoveEvent(e);
}
