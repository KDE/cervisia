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


#ifndef UPDATEVIEW_H
#define UPDATEVIEW_H


#include "listview.h"

#include <qptrlist.h>


class UpdateView : public ListView
{
    Q_OBJECT
    
public:

    enum Status { LocallyModified, LocallyAdded, LocallyRemoved,
                  NeedsUpdate, NeedsPatch, NeedsMerge,
                  UpToDate, Conflict,
                  Updated, Patched, Removed,
                  NotInCVS, Unknown };
    enum Filter { NoFilter=0, OnlyDirectories=1, NoUpToDate=2,
                  NoRemoved=4, NoNotInCVS=8 };
    enum Action { Add, Remove, Update, UpdateNoAct, Commit };
    
    UpdateView( QWidget *parent=0, const char *name=0 );
    ~UpdateView();

    void setFilter(Filter filter);
    Filter filter() const;
    
    static bool isDirItem(QListViewItem *item);

    bool hasSingleSelection() const;
    void getSingleSelection(QString *filename, QString *revision=0) const;
    /* Returns a list of all marked files and directories */
    QStringList multipleSelection() const;
    /* Returns a list of all marked files, excluding directories*/
    QStringList fileSelection() const;

    void openDirectory(const QString& dirname);
    void scanRecursive();
    void prepareJob(bool recursive, Action action);

    const QColor& conflictColor() const;
    const QColor& localChangeColor() const;
    const QColor& remoteChangeColor() const;

signals:
    void contextMenu();
    void fileOpened(QString filename);
    
public slots:
    void unfoldTree();
    void foldTree();
    void finishJob(bool normalExit, int exitStatus);
    void processUpdateLine(QString line);

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    
private slots:
    void itemExecuted(QListViewItem *item);
    
private:
    void updateItem(const QString &filename, Status status, bool isdir);
    void rememberSelection(bool recursive);
    void syncSelection();
    void markUpdated(bool laststage, bool success);

    void updateColors();

    Filter filt;
    Action act;
    QPtrList<QListViewItem> relevantSelection;

    QColor m_conflictColor;
    QColor m_localChangeColor;
    QColor m_remoteChangeColor;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
