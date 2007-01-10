/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2003-2004 Christian Loose <christian.loose@kdemail.net>
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


#ifndef REPOSITORYDLG_H
#define REPOSITORYDLG_H

#include <kdialog.h>


class Q3ListViewItem;
class QPushButton;
class K3ListView;
class CvsService_stub;
class RepositoryListItem;


class RepositoryDialog : public KDialog
{
    Q_OBJECT

public:
    RepositoryDialog(KConfigBase& cfg, CvsService_stub* cvsService,
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
    void slotDoubleClicked(Q3ListViewItem *item);
    void slotLoginClicked();
    void slotLogoutClicked();
    void slotSelectionChanged();
    
private:
    void writeRepositoryData(RepositoryListItem* item);

private:
    KConfigBase&     m_partConfig;
    CvsService_stub* m_cvsService;
    KConfigBase*     m_serviceConfig;
    K3ListView*       m_repoList;
    QPushButton*     m_modifyButton;
    QPushButton*     m_removeButton;
    QPushButton*     m_loginButton;
    QPushButton*     m_logoutButton;
};

#endif


// kate: space-indent on; indent-width 4; replace-tabs on;
