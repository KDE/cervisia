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


#include "tiplabel.h"

#include <kdeversion.h>
#include <kglobalsettings.h>

#include <qapplication.h>
#include <qsimplerichtext.h>
#include <qtooltip.h>


TipLabel::TipLabel(const QString &text)
    : QLabel(0, "annotate label", WStyle_Customize|WStyle_StaysOnTop|WStyle_NoBorder|WStyle_Tool|WX11BypassWM)
{
    setMargin(2);
    setIndent(0);
    setFrameStyle( QFrame::Plain | QFrame::Box );
    setText(text);
    setPalette( QToolTip::palette() );

    QSimpleRichText doc(text, font());
#if KDE_IS_VERSION(3,1,90)
    doc.setWidth(KGlobalSettings::desktopGeometry(this).width());
#else
    doc.setWidth(QApplication::desktop()->width());
#endif
    whint = doc.widthUsed() + 2*frameWidth() + 2*indent();
}


void TipLabel::showAt(QPoint pos)
{
    adjustSize();

#if KDE_IS_VERSION(3,1,90)
    QRect desk = KGlobalSettings::desktopGeometry(this);
    QPoint maxpos = QPoint(QMAX(desk.right()-width(), desk.left()),
                           QMAX(desk.bottom()-height(), desk.top()));
#else
    QPoint maxpos = QPoint(QMAX(QApplication::desktop()->width()-width(), 0),
                           QMAX(QApplication::desktop()->height()-height(), 0));
#endif
    pos = QPoint(QMIN(pos.x(), maxpos.x()),
                 QMIN(pos.y(), maxpos.y()));
    move(pos);
    show();
}


QSize TipLabel::minimumSizeHint() const
{
    return QSize(whint, heightForWidth(whint));
}


QSize TipLabel::sizeHint() const
{
    return QSize(whint, heightForWidth(whint));
}

#include "tiplabel.moc"


// Local Variables:
// c-basic-offset: 4
// End:

