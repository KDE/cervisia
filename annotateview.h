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


#ifndef ANNOTATEVIEW_H
#define ANNOTATEVIEW_H

#include <qlistview.h>

class TipLabel;
class AnnotateViewItem;


class AnnotateView : public QListView
{
    Q_OBJECT

public:
    AnnotateView( QWidget *parent=0, const char *name=0 );
    ~AnnotateView();

    void addLine(const QString &rev, const QString &author, const QString &date,
                 const QString &content, const QString &comment, bool odd);
    
    virtual QSize sizeHint() const;
    virtual void contentsMouseMoveEvent(QMouseEvent *e);
    virtual void windowActivationChange(bool oldActive);
    virtual void leaveEvent(QEvent *e);

private slots:
    void hideLabel();

private:
    TipLabel *currentLabel;
    AnnotateViewItem *currentTipItem;
};

#endif
