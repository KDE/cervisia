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


#ifndef LOGTREE_H
#define LOGTREE_H

#include <qmemarray.h>
#include <qptrlist.h>

#include "qttableview.h"


class LogTreeItem;
class LogTreeConnection;
class TipLabel;

namespace Cervisia
{
struct LogInfo;
}


typedef QPtrList<LogTreeItem> LogTreeItemList;
typedef QPtrList<LogTreeConnection> LogTreeConnectionList;


class LogTreeView : public QtTableView
{
    Q_OBJECT

public:
    explicit LogTreeView( QWidget *parent=0, const char *name=0 );
    virtual ~LogTreeView();

    void addRevision(const Cervisia::LogInfo& logInfo);
    void setSelectedPair(QString selectionA, QString selectionB);
    void collectConnections();
    void recomputeCellSizes();

    virtual QSize sizeHint() const;

signals:
    void revisionClicked(QString rev, bool rmb);

protected:
    //    virtual bool eventFilter(QObject *o, QEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void windowActivationChange(bool oldActive);
    virtual void leaveEvent(QEvent *e);
    virtual void setupPainter(QPainter *p);
    virtual void paintCell(QPainter *p, int row, int col);
    virtual int cellWidth(int col);
    virtual int cellHeight(int row);

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
