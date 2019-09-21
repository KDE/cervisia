/*
 * Copyright (C) 1999-2002 Bernd Gehrmann <bernd@mail.berlios.de>
 * Copyright (c) 2003-2008 André Wöbbeking <Woebbeking@kde.org>
 * Copyright (c) 2015 Martin Koller <kollix@aon.at>
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

#include "annotateview.h"

#include <QHeaderView>
#include <qpainter.h>
#include <kcolorscheme.h>
#include <QApplication>

#include "cervisiasettings.h"
#include "loginfo.h"
#include "tooltip.h"


using namespace Cervisia;


class AnnotateViewItem : public QTreeWidgetItem
{
public:
    enum { LineNumberColumn, AuthorColumn, ContentColumn };

    AnnotateViewItem(AnnotateView *parent, const LogInfo& logInfo,
                     const QString &content, bool odd, int linenumber);

    int lineNumber() const { return m_lineNumber; }

    QVariant data(int column, int role) const override;

private:
    LogInfo m_logInfo;
    QString m_content;
    bool    m_odd;
    int     m_lineNumber;

    friend class AnnotateView;
    friend class AnnotateViewDelegate;
};


AnnotateViewItem::AnnotateViewItem(AnnotateView *parent, const LogInfo& logInfo,
                                   const QString &content, bool odd, int linenumber)
    : QTreeWidgetItem(parent)
    , m_logInfo(logInfo)
    , m_content(content)
    , m_odd(odd)
    , m_lineNumber(linenumber)
{}


QVariant AnnotateViewItem::data(int column, int role) const
{
    if ( role == Qt::DisplayRole )
    {
        switch (column)
        {
        case LineNumberColumn:
            return QString::number(m_lineNumber);
        case AuthorColumn:
            if( m_logInfo.m_author.isNull() )
                return QString();
            else
                return (m_logInfo.m_author + QChar(' ') + m_logInfo.m_revision);
        case ContentColumn:
            return m_content;
        default:
            ;
        };

        return QString();
    }

    return QTreeWidgetItem::data(column, role);
}

//---------------------------------------------------------------------

void AnnotateViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    AnnotateViewItem *item = static_cast<AnnotateViewItem *>(view->topLevelItem(index.row()));

    QColor backgroundColor;
    QColor foregroundColor;

    if ( item->isSelected() || index.column() == AnnotateViewItem::LineNumberColumn )
    {
        backgroundColor = KColorScheme(QPalette::Active, KColorScheme::Selection).background().color();
        foregroundColor = KColorScheme(QPalette::Active, KColorScheme::Selection).foreground().color();
    }
    else
    {
        backgroundColor = item->m_odd
            ? KColorScheme(QPalette::Active, KColorScheme::View).background().color()
            : KColorScheme(QPalette::Active, KColorScheme::View).background(KColorScheme::AlternateBackground).color();
        foregroundColor = KColorScheme(QPalette::Active, KColorScheme::View).foreground().color();
    }

    painter->setPen(foregroundColor);
    painter->fillRect(option.rect, backgroundColor);

    QString str = item->text(index.column());
    if (str.isEmpty())
    {
        painter->restore();
        return;
    }

    Qt::Alignment align = (index.column() == AnnotateViewItem::LineNumberColumn) ? Qt::AlignRight : option.displayAlignment;

    if ((align & (Qt::AlignTop | Qt::AlignBottom)) == 0)
            align |= Qt::AlignVCenter;

    // only the content shall use the configured font, others use default
    // reason: usually you want fixed-width font in sourcecode, but line numbers and revision column look
    // nicer when they are not fixed-width
    if ( index.column() == AnnotateViewItem::ContentColumn )
        painter->setFont(view->font());
    else
        painter->setFont(QApplication::font());

    painter->drawText(option.rect.x() + BORDER, option.rect.y(), option.rect.width() - 2*BORDER, option.rect.height(), align, str);

    painter->restore();
}


QSize AnnotateViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt(option);

    if ( index.column() == AnnotateViewItem::ContentColumn )
        opt.font = view->font();
    else
        opt.font = QApplication::font();

    QSize s = QStyledItemDelegate::sizeHint(opt, index);
    s.setWidth(s.width() + 2*BORDER);

    return s;
}

//---------------------------------------------------------------------

AnnotateView::AnnotateView(QWidget *parent)
    : QTreeWidget(parent)
{
    setItemDelegate(new AnnotateViewDelegate(this));

    setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
    setAllColumnsShowFocus(true);
    setRootIsDecorated(false);
    setAutoScroll(false);
    setSelectionMode(QAbstractItemView::SingleSelection); // to be able to show the found item
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    header()->setStretchLastSection(false);
    header()->hide();
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    setColumnCount(3);

    ToolTip* toolTip = new ToolTip(viewport());

    connect(toolTip, SIGNAL(queryToolTip(QPoint,QRect&,QString&)),
            this, SLOT(slotQueryToolTip(QPoint,QRect&,QString&)));

    configChanged();

    connect(CervisiaSettings::self(), SIGNAL(configChanged()),
            this, SLOT(configChanged()));
}



void AnnotateView::addLine(const LogInfo& logInfo, const QString& content, bool odd)
{
    new AnnotateViewItem(this, logInfo, content, odd, topLevelItemCount()+1);
}


QSize AnnotateView::sizeHint() const
{
    QFontMetrics fm(fontMetrics());
    return QSize(100 * fm.width("0"), 10 * fm.lineSpacing());
}


void AnnotateView::configChanged()
{
    setFont(CervisiaSettings::annotateFont());
}


void AnnotateView::slotQueryToolTip(const QPoint& viewportPos,
                                    QRect&        viewportRect,
                                    QString&      text)
{
    if (const AnnotateViewItem* item = static_cast<AnnotateViewItem*>(itemAt(viewportPos)))
    {
        const int column(indexAt(viewportPos).column());
        if ((column == AnnotateViewItem::AuthorColumn) && !item->m_logInfo.m_author.isNull())
        {
            viewportRect = visualRect(indexAt(viewportPos));
            text = item->m_logInfo.createToolTipText(false);
        }
    }
}

void AnnotateView::findText(const QString &textToFind, bool up)
{
    QTreeWidgetItem *item = currentItem();
    if ( !item )
    {
        if ( up )
            item = topLevelItem(topLevelItemCount() - 1);  // last
        else
            item = topLevelItem(0);  // first
    }
    else
    {
        if ( up )
            item = itemAbove(item);
        else
            item = itemBelow(item);
    }

    for (; item && !item->text(AnnotateViewItem::ContentColumn).contains(textToFind, Qt::CaseInsensitive);
         item = up ? itemAbove(item) : itemBelow(item))
      ;

    setCurrentItem(item);

    if ( item )
    {
        item->setSelected(true);
        scrollToItem(item);
    }
}

int AnnotateView::currentLine() const
{
    QTreeWidgetItem *item = currentItem();
    if ( !item )
        return -1;

    return static_cast<AnnotateViewItem*>(item)->lineNumber();
}

int AnnotateView::lastLine() const
{
    AnnotateViewItem *item = static_cast<AnnotateViewItem*>(topLevelItem(topLevelItemCount() - 1));
    if ( !item )
        return 0;

    return item->lineNumber();
}


void AnnotateView::gotoLine(int line)
{
    for (AnnotateViewItem *item = static_cast<AnnotateViewItem*>(topLevelItem(0));
         item;
         item = static_cast<AnnotateViewItem*>(itemBelow(item)))
    {
        if ( item->lineNumber() == line )
        {
            setCurrentItem(item);
            item->setSelected(true);
            scrollToItem(item);
            return;
        }
    }
}


// Local Variables:
// c-basic-offset: 4
// End:
