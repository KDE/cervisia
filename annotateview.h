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


#ifndef _ANNOTATEVIEW_H_
#define _ANNOTATEVIEW_H_

#include <qlabel.h>
#if QT_VERSION < 300
#include <qlist.h>
#include <qtableview.h>
#else
#include <qptrlist.h>
#include <qttableview.h>
#endif

class AnnotateViewItem;
class TipLabel;

#if QT_VERSION < 300
typedef QList<AnnotateViewItem> AnnotateViewItemList;
#else
typedef QtTableView QTableView; // XXX: This is a hack
typedef QPtrList<AnnotateViewItem> AnnotateViewItemList;
#endif

class AnnotateView : public QTableView
{
    Q_OBJECT
    
public:
    AnnotateView( QWidget *parent=0, const char *name=0 );
    ~AnnotateView();

    void addLine(const QString &rev, const QString &author, const QString &date,
                 const QString &content, const QString &comment, bool odd);

    virtual void setFont(const QFont &font);
    virtual int cellWidth(int col);
    virtual QSize sizeHint() const;
    virtual void paintCell(QPainter *p, int row, int col);
    virtual void mouseMoveEvent(QMouseEvent *e);

private:
    AnnotateViewItemList items;
    int currentRow;
    TipLabel *currentLabel;
};

#endif
