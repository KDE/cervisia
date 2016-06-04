/*
 *  Copyright (c) 2003-2007 Christian Loose <christian.loose@kdemail.net>
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


#include "watchersdialog.h"

#include <QHeaderView>
#include <QTableView>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include "cvsserviceinterface.h"
#include "progressdialog.h"
#include "watchersmodel.h"


WatchersDialog::WatchersDialog(KConfig& cfg, QWidget* parent)
    : QDialog(parent), partConfig(cfg)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    m_tableView = new QTableView;
    mainLayout->addWidget(m_tableView);
    m_tableView->setSelectionMode(QAbstractItemView::NoSelection);
    m_tableView->setSortingEnabled(true);
    m_tableView->verticalHeader()->setVisible(false);

    mainLayout->addWidget(buttonBox);

    setAttribute(Qt::WA_DeleteOnClose, true);

    KConfigGroup cg(&partConfig, "WatchersDialog");
    restoreGeometry(cg.readEntry<QByteArray>("geometry", QByteArray()));
}


WatchersDialog::~WatchersDialog()
{
    KConfigGroup cg(&partConfig, "WatchersDialog");
    cg.writeEntry("geometry", saveGeometry());
}


bool WatchersDialog::parseWatchers(OrgKdeCervisia5CvsserviceCvsserviceInterface* cvsService,
                                   const QStringList& files)
{
    setWindowTitle(i18n("CVS Watchers"));

    QDBusReply<QDBusObjectPath> job = cvsService->watchers(files);
    if( !job.isValid() )
        return false;

    ProgressDialog dlg(this, "Watchers",cvsService->service(), job, "watchers", i18n("CVS Watchers"));
    if( !dlg.execute() )
        return false;

    WatchersSortModel* proxyModel = new WatchersSortModel(this);
    proxyModel->setSourceModel(new WatchersModel(dlg.getOutput()));
    m_tableView->setModel(proxyModel);
    m_tableView->sortByColumn(0, Qt::AscendingOrder);

    return true;
}
