/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef REPOSITORYDLG_H
#define REPOSITORYDLG_H

#include <kdialogbase.h>


class QListViewItem;
class QPushButton;
class KConfig;
class KListView;
class CvsService_stub;


class RepositoryDialog : public KDialogBase
{
    Q_OBJECT

public:
    RepositoryDialog(KConfig& cfg, CvsService_stub* cvsService,
                     QWidget* parent = 0, const char* name = 0);
    virtual ~RepositoryDialog();

    void readConfigFile();
    void readCvsPassFile();

protected:
    virtual void slotOk();

private slots:
    void slotAddClicked();
    void slotModifyClicked();
    void slotRemoveClicked();
    void slotDoubleClicked(QListViewItem *item);
    void slotLoginClicked();
    void slotLogoutClicked();
    void slotSelectionChanged();

private:
    KConfig&         m_partConfig;
    CvsService_stub* m_cvsService;
    KConfig*         m_serviceConfig;
    KListView*       m_repoList;
    QPushButton*     m_modifyButton;
    QPushButton*     m_removeButton;
    QPushButton*     m_loginButton;
    QPushButton*     m_logoutButton;
};

#endif


// kate: space-indent on; indent-width 4; replace-tabs on;
