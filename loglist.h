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


#ifndef LOGLIST_H
#define LOGLIST_H

#include <qarray.h>
#include <qlistview.h>
#include <qheader.h>

#include "listview.h"


class KConfig;
class TipLabel;
class LogListViewItem;


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
    virtual void windowActivationChange(bool oldActive);
    virtual void leaveEvent(QEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);

private slots:
    void hideLabel();

private:
    struct Options {
        int sortColumn;
        bool sortAscending;
        QArray<int> indexToColumn;
        QArray<int> columnSizes;
    };
    static Options *options;
    
    TipLabel *currentLabel;
    LogListViewItem *currentTipItem;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
