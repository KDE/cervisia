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


#ifndef _LISTVIEW_H_
#define _LISTVIEW_H_

#include <klistview.h>

class QTimer;


class ListView : public KListView
{
    Q_OBJECT

public:
    ListView( QWidget *parent=0, const char *name=0 );

    void setPreferredColumn(int i);
    int preferredColumn() const;

protected:
    virtual void setColumnWidth(int column, int w);
    virtual void resizeEvent(QResizeEvent *e);
    
private slots:
    void headerSizeChange();

private:
    int m_preferredColumn;
    QTimer *timer;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
