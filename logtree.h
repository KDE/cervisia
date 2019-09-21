/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2004 Christian Loose <christian.loose@hamburg.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef LOGTREE_H
#define LOGTREE_H


#include <qlist.h>

#include <QTableView>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>


class LogTreeItem;
class LogTreeConnection;

namespace Cervisia
{
struct LogInfo;
}


typedef QList<LogTreeItem*> LogTreeItemList;
typedef QList<LogTreeConnection*> LogTreeConnectionList;


enum SelectedRevision
{
    NoRevision,
    RevisionA,
    RevisionB
};


class LogTreeView : public QTableView
{
    Q_OBJECT

public:
    explicit LogTreeView( QWidget *parent=0, const char *name=0 );

    ~LogTreeView() override;

    void addRevision(const Cervisia::LogInfo& logInfo);
    void setSelectedPair(QString selectionA, QString selectionB);
    void collectConnections();
    void recomputeCellSizes();
    void paintCell(QPainter *p, int row, int col);

    QSize sizeHint() const override;
    
    virtual QString text(int row, int col) const;

signals:
    void revisionClicked(QString rev, bool rmb);

private slots:
    void mousePressed(const QModelIndex &index);
    void slotQueryToolTip(const QPoint&, QRect&, QString&);

private:
    QSize computeSize(const Cervisia::LogInfo&, int* = 0, int* = 0) const;
    void paintRevisionCell(QPainter *p, int row, int col, const Cervisia::LogInfo& logInfo,
                           bool followed, bool branched, SelectedRevision selected);
    void paintConnector(QPainter *p, int row, int col, bool followed, bool branched);

    LogTreeItemList items;
    LogTreeConnectionList connections;
    int rowCount, columnCount;

    static const int BORDER;
    static const int INSPACE;

    friend class LogTreeModel;

    class LogTreeModel *model;
};


class LogTreeModel : public QAbstractTableModel
{
public:
    explicit LogTreeModel(LogTreeView *t) : logView(t) { }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return logView->rowCount;
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return logView->columnCount;
    }

    QVariant data(const QModelIndex &, int) const override { return QVariant(); }

private:
    LogTreeView *logView;

    friend class LogTreeView;
};


class LogTreeDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit LogTreeDelegate(LogTreeView *t) : logView(t) { }
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    LogTreeView *logView;
};

#endif


// vim: set sw=4
// Local Variables:
// c-basic-offset: 4
// End:
