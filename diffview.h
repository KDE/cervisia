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


#ifndef DIFFVIEW_H
#define DIFFVIEW_H


#include "qttableview.h"

#include <qptrcollection.h>
#include <qptrlist.h>


class DiffViewItem;


class DiffViewItemList : public QPtrList<DiffViewItem>
{
protected:
    virtual int compareItems(QPtrCollection::Item item1, QPtrCollection::Item item2);
};


class DiffView : public QtTableView
{
    Q_OBJECT

public:
    enum DiffType { Change, Insert, Delete, Neutral, Unchanged, Separator };

    DiffView( bool withlinenos, bool withmarker,
	      QWidget *parent=0, const char *name=0 );

    void setPartner(DiffView *other);

    void up()
        { setTopCell(topCell()-1); }
    void down()
        { setTopCell(topCell()+1); }
    void next()
        { setTopCell(topCell()+viewHeight()/cellHeight()); }
    void prior()
        { setTopCell(topCell()-viewHeight()/cellHeight()); }

    void addLine(const QString &line, DiffType type, int no=-1);
    QString stringAtLine(int lineno);
    void setCenterLine(int lineno);
    void setInverted(int lineno, bool inverted);
    int count();
    void removeAtOffset(int offset);
    void insertAtOffset(const QString &line, DiffType type, int offset);
    void setCenterOffset(int offset);
    QString stringAtOffset(int offset);
    QByteArray compressedContent();

    virtual void setFont(const QFont &font);
    virtual int cellWidth(int col);
    virtual QSize sizeHint() const;
    virtual void paintCell(QPainter *p, int row, int col);
    virtual void wheelEvent(QWheelEvent *);
    const QScrollBar *scrollBar() const
        { return verticalScrollBar(); }

protected slots:
    void vertPositionChanged(int val);
    void horzPositionChanged(int val);
  
private:
    int findLine(int lineno);
    DiffViewItemList items;
    bool linenos;
    bool marker;
    int textwidth;
    DiffView *partner;
    static const int BORDER;

    QColor diffChangeColor;
    QColor diffInsertColor;
    QColor diffDeleteColor;

    int m_tabWidth;
};


class DiffZoomWidget : public QFrame
{
    Q_OBJECT

public:
    DiffZoomWidget(QWidget *parent=0, const char *name=0);
    ~DiffZoomWidget();

    void setDiffView(DiffView *view);
    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *);
    bool eventFilter(QObject *, QEvent *e);

private:
    DiffView *diffview;
    
    QColor diffChangeColor;
    QColor diffInsertColor;
    QColor diffDeleteColor;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
