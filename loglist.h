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


#ifndef _LOGLIST_H_
#define _LOGLIST_H_

#include <qarray.h>
#include <qlistview.h>
#include <qheader.h>

#include "listview.h"

class TipLabel;


class LogListViewItem : public QListViewItem
{
public:
    
    LogListViewItem(QListView *list,
                    const QString &rev, const QString &author, const QString &date,
                    const QString &comment, const QString &tagcomment);

    virtual QString key(int column, bool) const;

private:
    static QString truncateLine(const QString &s);
    static QString extractOrdinaryTags(const QString &s);
    static QString extractBranchName(const QString &s);

    QString mrev, mauthor, mdate;
    QString mcomment, mtagcomment;
    friend class LogListView;
};


class LogListView : public ListView
{
    Q_OBJECT
    
public:
    LogListView( QWidget *parent=0, const char *name=0 );
    ~LogListView();
    
    void addRevision(const QString &rev, const QString &author, const QString &date,
                     const QString &comment, const QString &tagcomment);
    void setSelectedPair(const QString &selectionA, const QString &selectionB);

    static void loadOptions(KConfig *config);
    static void saveOptions(KConfig *config);

signals:
    void revisionClicked(QString rev, bool rmb);

protected:
    virtual void contentsMousePressEvent(QMouseEvent *e);
    virtual void contentsMouseMoveEvent(QMouseEvent *e);
    virtual void leaveEvent(QEvent *);
    virtual void keyPressEvent(QKeyEvent *e);

private slots:
//    void headerClicked(int column);

private:
    struct Options {
        int sortColumn;
        bool sortAscending;
        QArray<int> indexToColumn;
        QArray<int> columnSizes;
    };
    static Options *options;
    
    //    int sortColumn;
    //    bool sortAscending;
    TipLabel *currentLabel;
    LogListViewItem *currentTipItem;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
