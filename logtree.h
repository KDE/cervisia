/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2004 Christian Loose <christian.loose@hamburg.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef LOGTREE_H
#define LOGTREE_H

#include <qmemarray.h>
#include <qptrlist.h>

#include <qtable.h>


class LogTreeItem;
class LogTreeConnection;
class TipLabel;

namespace Cervisia
{
struct LogInfo;
}


typedef QPtrList<LogTreeItem> LogTreeItemList;
typedef QPtrList<LogTreeConnection> LogTreeConnectionList;


class LogTreeView : public QTable
{
    Q_OBJECT

public:
    explicit LogTreeView( QWidget *parent=0, const char *name=0 );
    virtual ~LogTreeView();

    void addRevision(const Cervisia::LogInfo& logInfo);
    void setSelectedPair(QString selectionA, QString selectionB);
    void collectConnections();
    void recomputeCellSizes();
    virtual void paintCell(QPainter *p, int row, int col, const QRect& cr,
                           bool selected, const QColorGroup& cg);

    virtual QSize sizeHint() const;

signals:
    void revisionClicked(QString rev, bool rmb);

protected:
    virtual void contentsMousePressEvent(QMouseEvent *e);
    virtual void contentsMouseMoveEvent(QMouseEvent *e);
    virtual void windowActivationChange(bool oldActive);
    virtual void leaveEvent(QEvent *e);
    virtual int columnWidth(int col);
    virtual int rowHeight(int row);

private:
    void paintRevisionCell(QPainter *p, int row, int col, const Cervisia::LogInfo& logInfo,
                           bool followed, bool branched, bool selected);
    void paintConnector(QPainter *p, int row, int col, bool followed, bool branched);
    void hideLabel();

    LogTreeItemList items;
    LogTreeConnectionList connections;
    int currentRow, currentCol;
    TipLabel *currentLabel;
    QMemArray<int> colWidths;
    QMemArray<int> rowHeights;

    static const int BORDER;
    static const int INSPACE;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
