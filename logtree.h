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


#ifndef _LOGTREE_H_
#define _LOGTREE_H_

#include <qobject.h>
#if QT_VERSION < 300
#include <qlist.h>
#include <qtableview.h>
#else
#include <qarray.h>
#include <qptrlist.h>
#include <qttableview.h>
#endif

class LogTreeItem;
class LogTreeConnection;
class TipLabel;

#if QT_VERSION < 300
typedef QList<LogTreeItem> LogTreeItemList;
typedef QList<LogTreeConnection> LogTreeConnectionList;
#else
typedef QtTableView QTableView; // XXX: Fix this!
typedef QPtrList<LogTreeItem> LogTreeItemList;
typedef QPtrList<LogTreeConnection> LogTreeConnectionList;
#endif

class LogTreeView : public QTableView
{
    Q_OBJECT

public:
    LogTreeView( QWidget *parent=0, const char *name=0 );
    ~LogTreeView();
    
    void addRevision(const QString &rev, const QString &author, const QString &date,
                     const QString &comment, const QString &taglist, const QString &tagcomment);
    void setSelectedPair(QString selectionA, QString selectionB);
    void collectConnections();
    void recomputeCellSizes();

    virtual QSize sizeHint() const;

signals:
    void revisionClicked(QString rev, bool rmb);

protected:
    virtual bool eventFilter(QObject * o, QEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void setupPainter(QPainter *p);
    virtual void paintCell(QPainter *p, int row, int col);
    virtual int cellWidth(int col);
    virtual int cellHeight(int row);

private:
    void paintRevisionCell(QPainter *p, int row, int col, QString line1, QString line2, QString line3,
			   bool followed, bool branched, bool selected);
    void paintConnector(QPainter *p, int row, int col, bool followed, bool branched);

    LogTreeItemList items;
    LogTreeConnectionList connections;
    int currentRow, currentCol;
    TipLabel *currentLabel;
    QArray<int> colWidths;
    QArray<int> rowHeights;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
