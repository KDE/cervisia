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

#include <qheader.h>
#include <qpainter.h>
#include <kconfig.h>
#include <kglobalsettings.h>

#include "cervisiapart.h"
#include "tiplabel.h"



class AnnotateViewItem : public QListViewItem
{
public:
    enum { LineNumberColumn, AuthorColumn, ContentColumn };

    AnnotateViewItem(AnnotateView *parent, const QString &rev, const QString &author,
                     const QString &date, const QString &content, const QString &comment,
                     bool odd, int linenumber);
        
    virtual int compare(QListViewItem *item, int col, bool ascending) const;
    virtual int width(const QFontMetrics &, const QListView *, int col) const;
    virtual QString text(int col) const;
    virtual void paintCell(QPainter *, const QColorGroup &, int, int, int);

private:
    QString mrev;
    QString mauthor;
    QString mdate;
    QString mcontent;
    QString mcomment;
    bool modd;
    int mlinenumber;
    friend class AnnotateView;
    
    static const int BORDER;
};


const int AnnotateViewItem::BORDER = 4;

 
AnnotateViewItem::AnnotateViewItem(AnnotateView *parent, const QString &rev, const QString &author,
                                   const QString &date, const QString &content, const QString &comment,
                                   bool odd, int linenumber)
    : QListViewItem(parent),
      mrev(rev), mauthor(author), mdate(date), mcontent(content),
      mcomment(comment), modd(odd), mlinenumber(linenumber)
{}


int AnnotateViewItem::compare(QListViewItem *item, int, bool) const
{
    int linenum1 = mlinenumber;
    int linenum2 = static_cast<AnnotateViewItem*>(item)->mlinenumber;

    return (linenum2 > linenum1)? -1 : (linenum2 < linenum1)? 1 : 0;
}


QString AnnotateViewItem::text(int col) const
{
    switch (col)
    {
    case LineNumberColumn:
        return QString::number(mlinenumber);
    case AuthorColumn:
        return mauthor.isNull()? "" : (mauthor + QChar(' ') + mrev);
    case ContentColumn:
        return mcontent;
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
        backgroundColor = modd? KGlobalSettings::baseColor() : KGlobalSettings::alternateBackgroundColor();
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

AnnotateView::AnnotateView(QWidget *parent, const char *name)
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

    KConfig *config = CervisiaPart::config();
    config->setGroup("LookAndFeel");
    setFont(config->readFontEntry("AnnotateFont"));
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


void AnnotateView::addLine(const QString &rev, const QString &author, const QString &date,
                           const QString &content, const QString &comment, bool odd)
{
    (void) new AnnotateViewItem(this, rev, author, date, content, comment, odd, childCount()+1);
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

    if (!currentLabel && item && col == AnnotateViewItem::AuthorColumn && !item->mauthor.isNull())
        {
            QString text = "<qt><b>";
            text += QStyleSheet::escape(item->mrev);
            text += "</b>&nbsp;&nbsp;";
            text += QStyleSheet::escape(item->mauthor);
            text += "&nbsp;&nbsp;<b>";
            text += QStyleSheet::escape(item->mdate);
            text += "</b>";
            QStringList list = QStringList::split("\n", item->mcomment);
            QStringList::Iterator it;
            for (it = list.begin(); it != list.end(); ++it)
                {
                    text += "<br>";
                    text += QStyleSheet::escape(*it);
                }
            text += "</qt>";
            
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
