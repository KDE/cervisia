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


#ifndef LOGLIST_H
#define LOGLIST_H


#include <klistview.h>


class KConfig;

class TipLabel;
class LogListViewItem;

namespace Cervisia
{
struct LogInfo;
}


class LogListView : public KListView
{
    Q_OBJECT
    
public:
    explicit LogListView( KConfig& cfg, QWidget *parent=0, const char *name=0 );
    virtual ~LogListView();
    
    void addRevision(const Cervisia::LogInfo& logInfo);
    void setSelectedPair(const QString &selectionA, const QString &selectionB);

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
    LogListViewItem* selectedItem(QMouseEvent* e);

    KConfig& partConfig;
    TipLabel *currentLabel;
    LogListViewItem *currentTipItem;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
