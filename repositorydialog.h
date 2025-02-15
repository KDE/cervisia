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

#ifndef REPOSITORYDIALOG_H
#define REPOSITORYDIALOG_H

#include <QDialog>

class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;
class OrgKdeCervisia5CvsserviceCvsserviceInterface;
class RepositoryListItem;
class KConfig;

class RepositoryDialog : public QDialog
{
    Q_OBJECT

public:
    RepositoryDialog(KConfig &cfg, OrgKdeCervisia5CvsserviceCvsserviceInterface *cvsService, const QString &cvsServiceInterfaceName, QWidget *parent = nullptr);
    ~RepositoryDialog() override;

    void readConfigFile();
    void readCvsPassFile();

protected Q_SLOTS:
    void slotOk();

private Q_SLOTS:
    void slotHelp();
    void slotAddClicked();
    void slotModifyClicked();
    void slotRemoveClicked();
    void slotDoubleClicked(QTreeWidgetItem *item, int column);
    void slotLoginClicked();
    void slotLogoutClicked();
    void slotSelectionChanged();

private:
    void writeRepositoryData(RepositoryListItem *item);

private:
    KConfig &m_partConfig;
    OrgKdeCervisia5CvsserviceCvsserviceInterface *m_cvsService;
    KConfig *m_serviceConfig;
    QTreeWidget *m_repoList;
    QPushButton *m_modifyButton;
    QPushButton *m_removeButton;
    QPushButton *m_loginButton;
    QPushButton *m_logoutButton;
    QString m_cvsServiceInterfaceName;
};

#endif // REPOSITORYDIALOG_H

// kate: space-indent on; indent-width 4; replace-tabs on;
