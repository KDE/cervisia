/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
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


#ifndef LOGDLG_H
#define LOGDLG_H

#include <kdialogbase.h>

#include "loginfo.h"

#include <qptrlist.h>


class LogListView;
class LogTreeView;
class LogPlainView;

class KConfig;

class QComboBox;
class QLabel;
class QTabWidget;
class QTextEdit;
class CvsService_stub;

class LogDialogTagInfo
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
    explicit LogDialog( KConfig& cfg, QWidget *parent=0, const char *name=0 );

    virtual ~LogDialog();

    bool parseCvsLog(CvsService_stub* service, const QString& fileName);

protected slots:
    void slotOk();
    void slotApply();

private slots:
    void findClicked();
    void diffClicked();
    void annotateClicked();
    void revisionSelected(QString rev, bool rmb);
    void tagASelected(int n);
    void tagBSelected(int n);
    void tabChanged(QWidget* w);

private:
    void tagSelected(LogDialogTagInfo* tag, bool rmb);
    void updateButtons();

    QString filename;
    QPtrList<Cervisia::LogInfo> items;
    QPtrList<LogDialogTagInfo> tags;
    QString selectionA;
    QString selectionB;
    LogTreeView *tree;
    LogListView *list;
    LogPlainView *plain;
    QTabWidget *tabWidget;
    QLabel *revbox[2];
    QLabel *authorbox[2];
    QLabel *datebox[2];
    QTextEdit *commentbox[2];
    QTextEdit *tagsbox[2];
    QComboBox *tagcombo[2];

    CvsService_stub* cvsService;
    KConfig&         partConfig;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
