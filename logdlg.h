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


#ifndef LOGDLG_H
#define LOGDLG_H

#include <qdialog.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qlistview.h>
#include <qcombobox.h>
#include <qtabbar.h>
#include "loglist.h"
#include "logtree.h"


class RevisionInfo
{
public:
    QString rev;
    QString author;
    QString date;
    QString comment;
    QString tagcomment;
};


class TagInfo
{
public:
    QString rev;
    QString tag;
    QString branchpoint;
};


class LogDialog : public QDialog
{
    Q_OBJECT

public:
    LogDialog( QWidget *parent=0, const char *name=0 );

    bool parseCvsLog(const QString &sbox, const QString &repo, const QString &fname);

    static void loadOptions(KConfig *config);
    static void saveOptions(KConfig *config);

protected:
    virtual void done(int res);

private slots:
    void helpClicked();
    void diffClicked();
    void annotateClicked();
    void revisionSelected(QString rev, bool rmb);
    void tagSelected(QString rev, bool rmb);
    void tagASelected(int n);
    void tagBSelected(int n);

private:
    struct Options {
        QSize size;
        bool showlisttab;
    };
    static Options *options;

    QString sandbox;
    QString repository;
    QString filename;
    QPtrList<RevisionInfo> items;
    QPtrList<TagInfo> tags;
    QString selectionA;
    QString selectionB;
    LogTreeView *tree;
    LogListView *list;
    QTabBar *tabbar;
    QLabel *revbox[2];
    QLabel *authorbox[2];
    QLabel *datebox[2];
    QTextEdit *commentbox[2];
    QTextEdit *tagsbox[2];
    QComboBox *tagcombo[2];
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
