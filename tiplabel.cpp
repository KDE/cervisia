/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
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


#include "tiplabel.h"

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
    doc.setWidth(QApplication::desktop()->width());
    whint = doc.widthUsed() + 2*frameWidth() + 2*indent();
}


void TipLabel::showAt(QPoint pos)
{
    adjustSize();

    pos = QPoint(QMIN(pos.x(), QApplication::desktop()->width()-width()),
                 QMIN(pos.y(), QApplication::desktop()->height()-height()));
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

