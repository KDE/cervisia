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

#include "annotateview.h"

#include <qdatetime.h>
#include <qheader.h>
#include <qpainter.h>
#include <qstylesheet.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>

#include "loginfo.h"
#include "tiplabel.h"

using namespace Cervisia;

class AnnotateViewItem : public QListViewItem
{
public:
    enum { LineNumberColumn, AuthorColumn, ContentColumn };

    AnnotateViewItem(AnnotateView *parent, const LogInfo& logInfo,
                     const QString &content, bool odd, int linenumber);

    virtual int compare(QListViewItem *item, int col, bool ascending) const;
    virtual int width(const QFontMetrics &, const QListView *, int col) const;
    virtual QString text(int col) const;
    virtual void paintCell(QPainter *, const QColorGroup &, int, int, int);

private:
    LogInfo m_logInfo;
    QString m_content;
    bool    m_odd;
    int     m_lineNumber;
    friend class AnnotateView;

    static const int BORDER;
};


const int AnnotateViewItem::BORDER = 4;


AnnotateViewItem::AnnotateViewItem(AnnotateView *parent, const LogInfo& logInfo,
                                   const QString &content, bool odd, int linenumber)
    : QListViewItem(parent)
    , m_logInfo(logInfo)
    , m_content(content)
    , m_odd(odd)
    , m_lineNumber(linenumber)
{}


int AnnotateViewItem::compare(QListViewItem *item, int, bool) const
{
    int linenum1 = m_lineNumber;
    int linenum2 = static_cast<AnnotateViewItem*>(item)->m_lineNumber;

    return (linenum2 > linenum1)? -1 : (linenum2 < linenum1)? 1 : 0;
}


QString AnnotateViewItem::text(int col) const
{
    switch (col)
    {
    case LineNumberColumn:
        return QString::number(m_lineNumber);
    case AuthorColumn:
        if( m_logInfo.m_author.isNull() )
            return QString::null;
        else
            return (m_logInfo.m_author + QChar(' ') + m_logInfo.m_revision);
    case ContentColumn:
        return m_content;
    default:
        ;
    };

    return "";
}


void AnnotateViewItem::paintCell(QPainter *p, const QColorGroup &, int col, int width, int align)
{
    QColor backgroundColor;

    switch (col)
    {
    case LineNumberColumn:
        backgroundColor = KGlobalSettings::highlightColor();
        p->setPen(KGlobalSettings::highlightedTextColor());
        break;
    default:
        backgroundColor = m_odd ? KGlobalSettings::baseColor()
                                : KGlobalSettings::alternateBackgroundColor();
        p->setPen(KGlobalSettings::textColor());
        break;
    };

    p->fillRect(0, 0, width, height(), backgroundColor);

    QString str = text(col);
    if (str.isEmpty())
        return;

    if (align & (AlignTop || AlignBottom) == 0)
            align |= AlignVCenter;

    p->drawText(BORDER, 0, width - 2*BORDER, height(), align, str);
}



int AnnotateViewItem::width(const QFontMetrics &fm, const QListView *, int col) const
{
    return fm.width(text(col)) + 2*BORDER;
}


/*!
  @todo The dummy column (remaining space eater) doesn't work
  caused by a bug in QHeader::adjustHeaderSize() in Qt <= 3.0.4.
*/

AnnotateView::AnnotateView(KConfig &cfg, QWidget *parent, const char *name)
    : QListView(parent, name, WRepaintNoErase | WResizeNoErase)
{
    setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
    setAllColumnsShowFocus(true);
    setShowToolTips(false);
    setSelectionMode(NoSelection);
    header()->hide();
    //    setResizeMode(LastColumn);

    addColumn(QString::null);
    addColumn(QString::null);
    addColumn(QString::null);

    setSorting(AnnotateViewItem::LineNumberColumn);
    setColumnAlignment(AnnotateViewItem::LineNumberColumn, Qt::AlignRight);

    connect( this, SIGNAL(contentsMoving(int, int)), this, SLOT(hideLabel()) );

    currentTipItem = 0;
    currentLabel = 0;

    KConfigGroupSaver cs(&cfg, "LookAndFeel");
    setFont(cfg.readFontEntry("AnnotateFont"));
}



AnnotateView::~AnnotateView()
{
    delete currentLabel;
}


void AnnotateView::hideLabel()
{
    delete currentLabel;
    currentLabel = 0;
}


void AnnotateView::addLine(const LogInfo& logInfo, const QString& content,
                           bool odd)
{
    new AnnotateViewItem(this, logInfo, content, odd, childCount()+1);
}


QSize AnnotateView::sizeHint() const
{
    QFontMetrics fm(fontMetrics());
    return QSize(100 * fm.width("0"), 10 * fm.lineSpacing());
}



void AnnotateView::contentsMouseMoveEvent(QMouseEvent *e)
{
    if (!isActiveWindow())
        return;

    QPoint vp = contentsToViewport(e->pos());
    AnnotateViewItem *item
        = static_cast<AnnotateViewItem*>( itemAt(vp) );
    int col = header()->sectionAt(vp.x());

    if (item != currentTipItem || col != AnnotateViewItem::AuthorColumn)
        hideLabel();

    if (!currentLabel && item && col == AnnotateViewItem::AuthorColumn &&
        !item->m_logInfo.m_author.isNull())
    {
            QString text = item->m_logInfo.createToolTipText(false);

            int left = header()->sectionPos(AnnotateViewItem::ContentColumn);
            int top = viewport()->mapTo(this, itemRect(item).topLeft()).y();
            currentLabel = new TipLabel(text);
            currentLabel->showAt(mapToGlobal(QPoint(left, top)));
            currentTipItem = item;
    }
}


void AnnotateView::windowActivationChange(bool oldActive)
{
    hideLabel();
    QListView::windowActivationChange(oldActive);
}


void AnnotateView::leaveEvent(QEvent *e)
{
    // has strange effects
    // hideLabel();
    QListView::leaveEvent(e);
}

#include "annotateview.moc"


// Local Variables:
// c-basic-offset: 4
// End:
