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


#ifndef TIPLABEL_H
#define TIPLABEL_H

#include <qlabel.h>


class TipLabel : public QLabel
{
    Q_OBJECT
    
public:
    TipLabel(const QString &text);

    void showAt(QPoint pos);
    
    virtual QSize minimumSizeHint() const;
    virtual QSize sizeHint() const;

private:
    int whint;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
