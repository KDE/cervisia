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


#ifndef _MULTILINELABEL_H_
#define _MULTILINELABEL_H_

#include <qmultilineedit.h>


class MultiLineLabel : public QMultiLineEdit
{
    Q_OBJECT

public:
    MultiLineLabel( QWidget *parent, const char *name=0 );
    ~MultiLineLabel();

protected:
    virtual QSize sizeHint() const;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
