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


#ifndef _TIPLABEL_H_
#define _TIPLABEL_H_

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
