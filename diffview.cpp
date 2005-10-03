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


#include "diffview.h"

#include <qpainter.h>
#include <qscrollbar.h>
#include <qpixmap.h>
#include <qregexp.h>
#include <qstyle.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <klocale.h>


class DiffViewItem
{
public:
    QString line;
    DiffView::DiffType type;
    bool inverted;
    int no;
};


int DiffViewItemList::compareItems(QPtrCollection::Item item1, QPtrCollection::Item item2)
{
    return (static_cast<DiffViewItem*>(item1)->no
            == static_cast<DiffViewItem*>(item2)->no)? 0 : 1;
}


const int DiffView::BORDER = 7;


DiffView::DiffView( KConfig& cfg, bool withlinenos, bool withmarker,
                    QWidget *parent, const char *name )
    : QtTableView(parent, name, WRepaintNoErase)
    , partConfig(cfg)
{
    setNumRows(0);
    setNumCols( 1 + (withlinenos?1:0) + (withmarker?1:0) );
    setTableFlags( Tbl_autoVScrollBar|Tbl_autoHScrollBar|
                   Tbl_smoothVScrolling );
    setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
    setBackgroundMode( PaletteBase );
    setWFlags( WResizeNoErase );

    partConfig.setGroup("LookAndFeel");
    setFont(partConfig.readFontEntry("DiffFont"));
    QFontMetrics fm(font());
    setCellHeight(fm.lineSpacing());
    setCellWidth(0);
    textwidth = 0;

    partConfig.setGroup("General");
    m_tabWidth = partConfig.readNumEntry("TabWidth", 8);

    items.setAutoDelete(true);
    linenos = withlinenos;
    marker = withmarker;

    partConfig.setGroup("Colors");
    QColor defaultColor=QColor(237, 190, 190);
    diffChangeColor=partConfig.readColorEntry("DiffChange",&defaultColor);
    defaultColor=QColor(190, 190, 237);
    diffInsertColor=partConfig.readColorEntry("DiffInsert",&defaultColor);
    defaultColor=QColor(190, 237, 190);
    diffDeleteColor=partConfig.readColorEntry("DiffDelete",&defaultColor);
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
    QFontMetrics fmbold(f);
    QFontMetrics fm(font());


    // calculate textwidth based on 'line' where tabs are expanded
    //
    // *Please note*
    // For some fonts, e.g. "Clean", is fm.maxWidth() greater than
    // fmbold.maxWidth().
    QString copy(line);
    const int numTabs = copy.contains('\t', false);
    copy.replace( QRegExp("\t"), "");

    const int tabSize   = m_tabWidth * QMAX(fm.maxWidth(), fmbold.maxWidth());
    const int copyWidth = QMAX(fm.width(copy), fmbold.width(copy));
    textwidth = QMAX(textwidth, copyWidth + numTabs * tabSize);

    DiffViewItem *item = new DiffViewItem;
    item->line = line;
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
        kdDebug(8050) << "Internal error: lineAtOffset" << endl;
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
        kdDebug(8050) << "Internal Error: Line " << lineno << " not found" << endl;
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
    QFontMetrics fm(font());
    p->setTabStops(m_tabWidth * fm.maxWidth());

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
    p->drawText(innerborder, 0, width-2*innerborder, height, align|ExpandTabs, str);
    p->setFont(oldFont);
}


void DiffView::wheelEvent(QWheelEvent *e)
{
    QApplication::sendEvent(verticalScrollBar(), e);
}


DiffZoomWidget::DiffZoomWidget(KConfig& cfg, QWidget *parent, const char *name)
    : QFrame(parent, name)
{
    setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Minimum ) );

    cfg.setGroup("Colors");
    QColor defaultColor=QColor(237, 190, 190);
    diffChangeColor=cfg.readColorEntry("DiffChange",&defaultColor);
    defaultColor=QColor(190, 190, 237);
    diffInsertColor=cfg.readColorEntry("DiffInsert",&defaultColor);
    defaultColor=QColor(190, 237, 190);
    diffDeleteColor=cfg.readColorEntry("DiffDelete",&defaultColor);
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
    const QScrollBar* scrollBar = diffview->scrollBar();
    if (!scrollBar)
        return;

    // only y and height are important
    const QRect scrollBarGroove(scrollBar->isVisible()
                                ? style().querySubControlMetrics(QStyle::CC_ScrollBar,
                                                                 scrollBar,
                                                                 QStyle::SC_ScrollBarGroove)
                                : rect());

    // draw rectangles at the positions of the differences

    const QByteArray& lineTypes(diffview->compressedContent());

    QPixmap pixbuf(width(), scrollBarGroove.height());
    pixbuf.fill(KGlobalSettings::baseColor());

    QPainter p(&pixbuf, this);
    if (const unsigned int numberOfLines = lineTypes.size())
    {
        const double scale(((double) scrollBarGroove.height()) / numberOfLines);
        for (unsigned int index(0); index < numberOfLines;)
        {
            const char lineType(lineTypes[index]);

            // don't use qRound() to avoid painting outside of the pixmap
            // (yPos1 must be lesser than scrollBarGroove.height())
            const int yPos1(static_cast<int>(index * scale));

            // search next line with different lineType
            for (++index; index < numberOfLines && lineType == lineTypes[index]; ++index)
                ;

            QColor color;
            switch (lineType)
            {
            case 'C':
                color = diffChangeColor;
                break;
            case 'I':
                color = diffInsertColor;
                break;
            case 'D':
                color = diffDeleteColor;
                break;
            case ' ':
            case 'N':
                color = KGlobalSettings::alternateBackgroundColor();
                break;
            }

            if (color.isValid())
            {
                const int yPos2(qRound(index * scale));
                const int areaHeight((yPos2 != yPos1) ? yPos2 - yPos1 : 1);

                p.fillRect(0, yPos1, pixbuf.width(), areaHeight, QBrush(color));
            }
        }
    }
    p.flush();
    bitBlt(this, 0, scrollBarGroove.y(), &pixbuf);
}

#include "diffview.moc"


// Local Variables:
// c-basic-offset: 4
// End:
