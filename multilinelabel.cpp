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


#include "multilinelabel.h"
#include "multilinelabel.moc"


MultiLineLabel::MultiLineLabel(QWidget *parent, const char *name)
    : QMultiLineEdit(parent, name)
{
    setReadOnly(true);
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setWordWrap(QMultiLineEdit::WidgetWidth);
    // setAlignment(AlignTop);
}


MultiLineLabel::~MultiLineLabel()
{}


QSize MultiLineLabel::sizeHint() const
{
    QFontMetrics fm(fontMetrics());
    
    return QSize(fm.width('X') * 30,
                 fm.lineSpacing()+fm.height()+frameWidth()*2);
}

// Local Variables:
// c-basic-offset: 4
// End:
