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

#ifndef LOGDIALOG_H
#define LOGDIALOG_H

#include <QDialog>

#include "loginfo.h"

#include <qlist.h>

class LogListView;
class LogTreeView;
class LogPlainView;

class KComboBox;
class QLabel;
class QSplitter;
class QTabWidget;
class QPushButton;
class QDialogButtonBox;
class KTextEdit;
class OrgKdeCervisia5CvsserviceCvsserviceInterface;
class KConfig;

class LogDialogTagInfo
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
    explicit LogDialog(KConfig &cfg, QWidget *parent = nullptr);

    ~LogDialog() override;

    bool parseCvsLog(OrgKdeCervisia5CvsserviceCvsserviceInterface *service, const QString &fileName);

protected Q_SLOTS:
    void slotOk();
    void slotPatch();
    void slotHelp();

private Q_SLOTS:
    void findClicked();
    void diffClicked();
    void annotateClicked();
    void revisionSelected(QString rev, bool rmb);
    void tagASelected(int n);
    void tagBSelected(int n);
    void tabChanged(int index);

private:
    void tagSelected(LogDialogTagInfo *tag, bool rmb);
    void updateButtons();

    QSplitter *splitter;
    QString filename;
    QList<Cervisia::LogInfo *> items;
    QList<LogDialogTagInfo *> tags;
    QString selectionA;
    QString selectionB;
    LogTreeView *tree;
    LogListView *list;
    LogPlainView *plain;
    QTabWidget *tabWidget;
    QLabel *revbox[2];
    QLabel *authorbox[2];
    QLabel *datebox[2];
    KTextEdit *commentbox[2];
    KTextEdit *tagsbox[2];
    KComboBox *tagcombo[2];
    QPushButton *user1Button, *user2Button, *user3Button, *okButton;
    QDialogButtonBox *buttonBox;

    OrgKdeCervisia5CvsserviceCvsserviceInterface *cvsService;
    KConfig &partConfig;
};

#endif // LOGDIALOG_H

// Local Variables:
// c-basic-offset: 4
// End:
