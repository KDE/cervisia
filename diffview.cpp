/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "diffview.h"

#include <qpainter.h>
#include <qscrollbar.h>
#include <qpixmap.h>
#include <qstyle.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>

#include "misc.h"
#include "cervisiapart.h"


class DiffViewItem
{
public:
    QString line;
    DiffView::DiffType type;
    bool inverted;
    int no;
};


int DiffViewItemList::compareItems(QCollection::Item item1, QCollection::Item item2)
{
    return (static_cast<DiffViewItem*>(item1)->no
	    == static_cast<DiffViewItem*>(item2)->no)? 0 : 1;
}


const int DiffView::BORDER = 7;


DiffView::DiffView( bool withlinenos, bool withmarker,
		    QWidget *parent, const char *name )
    : QtTableView(parent, name, WNorthWestGravity | WRepaintNoErase)
{
    setNumRows(0);
    setNumCols( 1 + (withlinenos?1:0) + (withmarker?1:0) );
    setTableFlags( Tbl_autoVScrollBar|Tbl_autoHScrollBar|
		   Tbl_smoothVScrolling
		   );
    setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
    setBackgroundMode( PaletteBase );
    setWFlags( WResizeNoErase );

    KConfig *config = CervisiaPart::config();
    config->setGroup("LookAndFeel");
    setFont(config->readFontEntry("DiffFont"));
    QFontMetrics fm(font());
    setCellHeight(fm.lineSpacing());
    setCellWidth(0);
    textwidth = 0;

    items.setAutoDelete(true);
    linenos = withlinenos;
    marker = withmarker;

    config->setGroup("Colors");
    QColor defaultColor=QColor(237, 190, 190);
    diffChangeColor=config->readColorEntry("DiffChange",&defaultColor);
    defaultColor=QColor(190, 190, 237);
    diffInsertColor=config->readColorEntry("DiffInsert",&defaultColor);
    defaultColor=QColor(190, 237, 190);
    diffDeleteColor=config->readColorEntry("DiffDelete",&defaultColor);
}


void DiffView::setFont(const QFont &font)
{
    QtTableView::setFont(font);
    QFontMetrics fm(font);
    setCellHeight(fm.lineSpacing());
}


void DiffView::setPartner(DiffView *other)
{
    partner = other;
    if (partner)
	{
	    connect( verticalScrollBar(), SIGNAL(valueChanged(int)),
		     SLOT(vertPositionChanged(int)) );
	    connect( verticalScrollBar(), SIGNAL(sliderMoved(int)),
		     SLOT(vertPositionChanged(int)) );
	    connect( horizontalScrollBar(), SIGNAL(valueChanged(int)),
		     SLOT(horzPositionChanged(int)) );
	    connect( horizontalScrollBar(), SIGNAL(sliderMoved(int)),
		     SLOT(horzPositionChanged(int)) );
	}
}


void DiffView::vertPositionChanged(int val)
{
    if (partner)
	partner->setYOffset(QMIN(val,partner->maxYOffset()));
}


void DiffView::horzPositionChanged(int val)
{
    if (partner)
	partner->setXOffset(QMIN(val,partner->maxXOffset()));
}


// *offset methods are only for views withlineno
void DiffView::removeAtOffset(int offset)
{
    items.remove(offset);
    setNumRows(numRows()-1);
}


void DiffView::insertAtOffset(const QString &line, DiffType type, int offset)
{
    DiffViewItem *item = new DiffViewItem;
    item->line = line;
    item->type = type;
    item->no = -1;
    item->inverted = false;
    items.insert(offset, item);
    setNumRows(numRows()+1);
}


void DiffView::setCenterOffset(int offset)
{
    if (!rowIsVisible(offset))
	{
	    int visiblerows = viewHeight()/cellHeight(0);
	    setTopCell( QMAX(0, offset - visiblerows/2) );
	}
}


void DiffView::addLine(const QString &line, DiffType type, int no)
{
    QFont f(font());
    f.setBold(true);
    QFontMetrics fm(f);

    QString tabSpaces;
    KConfig *config = CervisiaPart::config();
    config->setGroup("General");
    tabSpaces.fill(' ', config->readUnsignedNumEntry("TabWidth", 8) );

    QString expandedLine=line;
    expandedLine.replace( '\t', tabSpaces );
    textwidth = QMAX(textwidth, fm.width(expandedLine));

    DiffViewItem *item = new DiffViewItem;
    item->line = expandedLine;
    item->type = type;
    item->no = no;
    item->inverted = false;
    items.append(item);
    setNumRows(numRows()+1);
}


QString DiffView::stringAtOffset(int offset)
{
    if (offset >= (int)items.count())
	{
	    kdDebug() << "Internal error: lineAtOffset" << endl;
	}
    return items.at(offset)->line;
}


int DiffView::count()
{
    return items.count();
}


int DiffView::findLine(int lineno)
{
    int offset;
    DiffViewItem tmp;
    tmp.no = lineno;
    if ( (offset = items.find(&tmp)) == -1)
	{
	    kdDebug() << "Internal Error: Line " << lineno << " not found" << endl;
	    return -1;
	}
    return offset;
}


void DiffView::setInverted(int lineno, bool inverted)
{
    int offset;
    if ( (offset = findLine(lineno)) != -1)
	items.at(offset)->inverted = inverted;
}


void DiffView::setCenterLine(int lineno)
{
    int offset;
    if ( (offset = findLine(lineno)) != -1)
	setCenterOffset(offset);
}


QString DiffView::stringAtLine(int lineno)
{
    int pos;
    if ( (pos = findLine(lineno)) != -1 )
        return items.at(pos)->line;
    else
        return QString();
}


QByteArray DiffView::compressedContent()
{
    QByteArray res(items.count());

    QPtrListIterator<DiffViewItem> it(items);
    int i=0;
    for (; it.current(); ++it)
        {
            switch (it.current()->type)
                {
                case Change:   res[i] = 'C'; break;
                case Insert:   res[i] = 'I'; break;
                case Delete:   res[i] = 'D'; break;
                case Neutral:  res[i] = 'N'; break;
                case Unchanged:res[i] = 'U'; break;
                default:       res[i] = ' ';
                }
            ++i;
        }
    return res;
}


int DiffView::cellWidth(int col)
{
    if (col == 0 && linenos)
	{
	    QFontMetrics fm(font());
	    return fm.width("10000");
	}
    else if (marker && (col == 0 || col == 1))
        {
            QFontMetrics fm( fontMetrics() );
            return QMAX(QMAX( fm.width(i18n("Delete")),
                              fm.width(i18n("Insert"))),
                        fm.width(i18n("Change")))+2*BORDER;
        }
    else
	{
	    int rest = (linenos || marker)? cellWidth(0) : 0;
	    if (linenos && marker)
		rest += cellWidth(1);
	    return QMAX(textwidth, viewWidth()-rest);
	}
}


QSize DiffView::sizeHint() const
{
    QFontMetrics fm(font());
    return QSize( 4*fm.width("0123456789"), fm.lineSpacing()*8 );
}


void DiffView::paintCell(QPainter *p, int row, int col)
{
    DiffViewItem *item = items.at(row);

    int width = cellWidth(col);
    int height = cellHeight();

    QColor backgroundColor;
    bool inverted;
    int align;
    int innerborder;
    QString str;

    QFont oldFont(p->font());
    if (item->type==Separator)
        {
            backgroundColor = KGlobalSettings::highlightColor();
            p->setPen(KGlobalSettings::highlightedTextColor());
            inverted = false;
            align = AlignLeft;
            innerborder = 0;
	    if (col == (linenos?1:0) + (marker?1:0))
                str = item->line;
            QFont f(oldFont);
            f.setBold(true);
            p->setFont(f);
        }
    else if (col == 0 && linenos)
	{
	    backgroundColor = KGlobalSettings::highlightColor();
	    p->setPen(KGlobalSettings::highlightedTextColor());
	    inverted = false;
	    align = AlignLeft;
	    innerborder = 0;
	    if (item->no == -1)
		str = "+++++";
	    else
		str.setNum(item->no);
	}
    else if (marker && (col == 0 || col == 1))
	{
	    backgroundColor = KGlobalSettings::alternateBackgroundColor();
            p->setPen(KGlobalSettings::textColor());
	    inverted = false;
	    align = AlignRight;
	    innerborder = BORDER;
	    str = (item->type==Change)? i18n("Change")
		: (item->type==Insert)? i18n("Insert")
		: (item->type==Delete)? i18n("Delete") : QString::null;
	}
    else
	{
	    backgroundColor =
		(item->type==Change)? diffChangeColor
		: (item->type==Insert)? diffInsertColor
		: (item->type==Delete)? diffDeleteColor
		: (item->type==Neutral)? KGlobalSettings::alternateBackgroundColor() : KGlobalSettings::baseColor();
            p->setPen(KGlobalSettings::textColor());
            inverted = item->inverted;
	    align = AlignLeft;
	    innerborder = 0;
	    str = item->line;
	}

    if (inverted)
	{
	    p->setPen(backgroundColor);
	    backgroundColor = KGlobalSettings::textColor();
            QFont f(oldFont);
            f.setBold(true);
            p->setFont(f);
	}

    p->fillRect(0, 0, width, height, backgroundColor);
    p->drawText(innerborder, 0, width-2*innerborder, height, align, str);
    p->setFont(oldFont);
}


void DiffView::wheelEvent(QWheelEvent *e)
{
    QApplication::sendEvent(verticalScrollBar(), e);
}


DiffZoomWidget::DiffZoomWidget(QWidget *parent, const char *name)
    : QFrame(parent, name)
{
    setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Minimum ) );

    KConfig *config = CervisiaPart::config();
    config->setGroup("Colors");
    QColor defaultColor=QColor(237, 190, 190);
    diffChangeColor=config->readColorEntry("DiffChange",&defaultColor);
    defaultColor=QColor(190, 190, 237);
    diffInsertColor=config->readColorEntry("DiffInsert",&defaultColor);
    defaultColor=QColor(190, 237, 190);
    diffDeleteColor=config->readColorEntry("DiffDelete",&defaultColor);
}


DiffZoomWidget::~DiffZoomWidget()
{}


void DiffZoomWidget::setDiffView(DiffView *view)
{
    diffview = view;
    QScrollBar *sb = const_cast<QScrollBar*>(diffview->scrollBar());
    sb->installEventFilter(this);
}


QSize DiffZoomWidget::sizeHint() const
{
    return QSize(25, style().pixelMetric(QStyle::PM_ScrollBarExtent, this));
}


bool DiffZoomWidget::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Show
        || e->type() == QEvent::Hide
        || e->type() == QEvent::Resize)
        repaint();

    return QFrame::eventFilter(o, e);
}


void DiffZoomWidget::paintEvent(QPaintEvent *)
{
    const QScrollBar *bar = diffview->scrollBar();
    if (!bar)
        return;

    int sliderMin, sliderMax, sliderLength;
    if (bar->isVisible())
        {
            sliderMin = style().pixelMetric(QStyle::PM_ScrollBarSliderMin, bar);
            sliderLength = style().pixelMetric(QStyle::PM_SliderLength, bar);
            sliderMax = style().pixelMetric(QStyle::PM_SliderSpaceAvailable, bar);
        }
    else
        {
            sliderMin = 0;
            sliderMax = height();
            sliderLength = 0;
        }

    QByteArray str = diffview->compressedContent();

    QPixmap pixbuf(size());
    QPainter p(&pixbuf, this);
    p.fillRect(0, 0, pixbuf.width(), pixbuf.height(), colorGroup().background());
    if (str.size())
        {
            double scale = ((double)(sliderMax-sliderMin+sliderLength)) / str.size();
            int y0, y1;
            y0 = y1 = 0;
            for (int i=0; i < (int)str.size(); ++i)
                {
                    char c = str[i];
                    int y1 = (int)(i*scale);
                    int y2 = (int)((i+1)*scale);
                    if (y1 != y0 || c != 'U')
                        {
                            QColor color =
                              (c==' ')? KGlobalSettings::alternateBackgroundColor()
                              : (c=='C')? diffChangeColor
                              : (c=='I')? diffInsertColor
                              : (c=='D')? diffDeleteColor
                              : (c=='N')? KGlobalSettings::alternateBackgroundColor() : KGlobalSettings::baseColor();

                            if (y2 == y1)
                                y2++;
                            p.fillRect(0, sliderMin+y1, pixbuf.width(), y2-y1, QBrush(color));
                            y0 = y1;
                        }
                }
        }
    p.flush();
    bitBlt(this, 0, 0, &pixbuf);
}

#include "diffview.moc"


// Local Variables:
// c-basic-offset: 4
// End:
