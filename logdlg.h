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

#include <dcopref.h>
#include <kdialogbase.h>

#include <qdatetime.h>
#include <qptrlist.h>


class LogListView;
class LogTreeView;

class KConfig;

class QComboBox;
class QLabel;
class QTabWidget;
class QTextEdit;


class RevisionInfo
{
public:
    QString rev;
    QString author;
    QDateTime date;
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


class LogDialog : public KDialogBase
{
    Q_OBJECT

public:

    explicit LogDialog( QWidget *parent=0, const char *name=0 );

    virtual ~LogDialog();

    bool parseCvsLog(DCOPRef& service, const QString& fileName);

    static void loadOptions(KConfig *config);
    static void saveOptions(KConfig *config);

private slots:
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
    QTabWidget *tabWidget;
    QLabel *revbox[2];
    QLabel *authorbox[2];
    QLabel *datebox[2];
    QTextEdit *commentbox[2];
    QTextEdit *tagsbox[2];
    QComboBox *tagcombo[2];

    DCOPRef cvsService;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
