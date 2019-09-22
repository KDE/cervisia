/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2002-2004 Christian Loose <christian.loose@kdemail.net>
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


#include "repositorydialog.h"
#include "debug.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDebug>

#include <kconfig.h>
#include <kmessagebox.h>
#include <kconfiggroup.h>
#include <KConfigGroup>
#include <KHelpClient>
#include <KLocalizedString>

#include "addrepositorydialog.h"
#include "cvsserviceinterface.h"
#include "cvsloginjobinterface.h"
#include "misc.h"
#include "progressdialog.h"
#include "repositories.h"


class RepositoryListItem : public QTreeWidgetItem
{
public:
    RepositoryListItem(QTreeWidget* parent, const QString& repo, bool loggedin);

    void setRsh(const QString& rsh);
    void setServer(const QString& server) { m_server = server; }
    void setCompression(int compression);
    void setIsLoggedIn(bool isLoggedIn);
    void setRetrieveCvsignore(bool retrieve) { m_retrieveCvsignore = retrieve; }

    QString repository() const
    {
        return text(0);
    }
    QString rsh() const
    {
        QString str = text(1);
        return (str.startsWith(QLatin1String("ext ("))
                ? str.mid(5, str.length() - 6)
                : QString());
    }
    QString server() const { return m_server; }
    int compression() const
    {
        bool ok;
        int n = text(2).toInt(&ok);
        return ok ? n : -1;
    }
    bool isLoggedIn() const { return m_isLoggedIn; }
    bool retrieveCvsignore() const { return m_retrieveCvsignore; }

private:
    void changeLoginStatusColumn();

private:
    QString m_server;
    bool    m_isLoggedIn;
    bool    m_retrieveCvsignore;
};


static bool LoginNeeded(const QString& repository)
{
    return repository.startsWith(QLatin1String(":pserver:")) ||
           repository.startsWith(QLatin1String(":sspi:"));
}


RepositoryListItem::RepositoryListItem(QTreeWidget* parent, const QString& repo,
                                       bool loggedin)
    : QTreeWidgetItem(parent)
    , m_isLoggedIn(loggedin)
{
    qCDebug(log_cervisia) << "repo=" << repo;
    setText(0, repo);

    changeLoginStatusColumn();
}


void RepositoryListItem::setRsh(const QString& rsh)
{
    QString repo = repository();
    QString method;

    if( repo.startsWith(QLatin1String(":pserver:")) )
        method = "pserver";
    else if( repo.startsWith(QLatin1String(":sspi:")) )
        method = "sspi";
    else if( repo.contains(':') )
    {
        method = "ext";
        if( !rsh.isEmpty() )
        {
            method += " (";
            method += rsh;
            method += ')';
        }
    }
    else
        method = "local";

    setText(1, method);
}


void RepositoryListItem::setCompression(int compression)
{
    QString compressionStr = (compression >= 0) ? QString::number(compression)
                                                : i18n("Default");

    setText(2, compressionStr);
}


void RepositoryListItem::setIsLoggedIn(bool isLoggedIn)
{
    m_isLoggedIn = isLoggedIn;

    changeLoginStatusColumn();
}


void RepositoryListItem::changeLoginStatusColumn()
{
    QString loginStatus;

    if( LoginNeeded(repository()) )
        loginStatus = m_isLoggedIn ? i18n("Logged in") : i18n("Not logged in");
    else
        loginStatus = i18n("No login required");

    setText(3, loginStatus);
}


RepositoryDialog::RepositoryDialog(KConfig& cfg, OrgKdeCervisia5CvsserviceCvsserviceInterface* cvsService,
                                   const QString& cvsServiceInterfaceName, QWidget* parent)
    : QDialog(parent)
    , m_partConfig(cfg)
    , m_cvsService(cvsService)
    , m_cvsServiceInterfaceName(cvsServiceInterfaceName)
{
    setWindowTitle(i18n("Configure Access to Repositories"));
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(okButton, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QBoxLayout* hbox = new QHBoxLayout;
    hbox->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(hbox);

    m_repoList = new QTreeWidget;
    hbox->addWidget(m_repoList, 10);
    m_repoList->setMinimumWidth(fontMetrics().width('0') * 60);
    m_repoList->setAllColumnsShowFocus(true);
    m_repoList->setRootIsDecorated(false);
    m_repoList->setHeaderLabels(QStringList() << i18n("Repository") << i18n("Method")
                                              << i18n("Compression") << i18n("Status"));
    m_repoList->setFocus();

    connect(m_repoList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotDoubleClicked(QTreeWidgetItem*,int)));
    connect(m_repoList, SIGNAL(itemSelectionChanged()),
            this,       SLOT(slotSelectionChanged()));

    QDialogButtonBox* actionbox = new QDialogButtonBox(Qt::Vertical);
    QPushButton* addbutton = actionbox->addButton(i18n("Add..."), QDialogButtonBox::ActionRole);
    m_modifyButton = actionbox->addButton(i18n("Modify..."), QDialogButtonBox::ActionRole);
    m_removeButton = actionbox->addButton(i18n("Remove"), QDialogButtonBox::ActionRole);
    m_loginButton  = actionbox->addButton(i18n("Login..."), QDialogButtonBox::ActionRole);
    m_logoutButton = actionbox->addButton(i18n("Logout"), QDialogButtonBox::ActionRole);
    hbox->addWidget(actionbox, 0);

    m_loginButton->setEnabled(false);
    m_logoutButton->setEnabled(false);

    connect( addbutton, SIGNAL(clicked()),
             this, SLOT(slotAddClicked()) );
    connect( m_modifyButton, SIGNAL(clicked()),
             this, SLOT(slotModifyClicked()) );
    connect( m_removeButton, SIGNAL(clicked()),
             this, SLOT(slotRemoveClicked()) );
    connect( m_loginButton, SIGNAL(clicked()),
             this, SLOT(slotLoginClicked()) );
    connect( m_logoutButton, SIGNAL(clicked()),
             this, SLOT(slotLogoutClicked()) );

    // open cvs DBUS service configuration file
    m_serviceConfig = new KConfig("cvsservicerc");

    readCvsPassFile();
    readConfigFile();

    if (QTreeWidgetItem *item = m_repoList->topLevelItem(0))
    {
        m_repoList->setCurrentItem(item);
        item->setSelected(true);
    }
    else
    {
        // we have no item so disable modify and remove button
        slotSelectionChanged();
    }

    connect(buttonBox, &QDialogButtonBox::helpRequested, this, &RepositoryDialog::slotHelp);

    setAttribute(Qt::WA_DeleteOnClose, true);

    KConfigGroup cg(&m_partConfig, "RepositoryDialog");
    restoreGeometry(cg.readEntry<QByteArray>("geometry", QByteArray()));

    QByteArray state = cg.readEntry<QByteArray>("RepositoryListView", QByteArray());
    m_repoList->header()->restoreState(state);

    mainLayout->addWidget(buttonBox);
}


RepositoryDialog::~RepositoryDialog()
{
    KConfigGroup cg(&m_partConfig, "RepositoryDialog");
    cg.writeEntry("geometry", saveGeometry());

    cg.writeEntry("RepositoryListView", m_repoList->header()->saveState());

    delete m_serviceConfig;
}


void RepositoryDialog::readCvsPassFile()
{
    QStringList list = Repositories::readCvsPassFile();
    Q_FOREACH(const QString& it, list)
        (void) new RepositoryListItem(m_repoList, it, true);
}


void RepositoryDialog::readConfigFile()
{
    QStringList list = Repositories::readConfigFile();

    // Sort out all list elements which are already in the list view
    for (int i = 0; i < m_repoList->topLevelItemCount(); i++)
        list.removeAll(m_repoList->topLevelItem(i)->text(0));

    Q_FOREACH(const QString& it, list)
        new RepositoryListItem(m_repoList, it, false);

    // Now look for the used methods
    for (int i = 0; i < m_repoList->topLevelItemCount(); i++)
    {
        RepositoryListItem* ritem = static_cast<RepositoryListItem*>(m_repoList->topLevelItem(i));

        // read entries from cvs DBUS service configuration
        const KConfigGroup repoGroup = m_serviceConfig->group(QLatin1String("Repository-") +
                                                              ritem->repository());

        qCDebug(log_cervisia) << "repository=" << ritem->repository();

        QString rsh       = repoGroup.readEntry("rsh", QString());
        QString server    = repoGroup.readEntry("cvs_server", QString());
        int compression   = repoGroup.readEntry("Compression", -1);
        bool retrieveFile = repoGroup.readEntry("RetrieveCvsignore", false);

        ritem->setRsh(rsh);
        ritem->setServer(server);
        ritem->setCompression(compression);
        ritem->setRetrieveCvsignore(retrieveFile);
    }

    m_repoList->header()->resizeSections(QHeaderView::ResizeToContents);
}

void RepositoryDialog::slotHelp()
{
    KHelpClient::invokeHelp(QLatin1String("accessing-repository"));
}

void RepositoryDialog::slotOk()
{
    // Make list of repositories
    QStringList list;
    for (int i = 0; i < m_repoList->topLevelItemCount(); i++)
        list.append(m_repoList->topLevelItem(i)->text(0));

    KConfigGroup reposGroup = m_partConfig.group("Repositories");
    reposGroup.writeEntry("Repos", list);

    for (int i = 0; i < m_repoList->topLevelItemCount(); i++)
    {
        RepositoryListItem* ritem = static_cast<RepositoryListItem*>(m_repoList->topLevelItem(i));

        // write entries to cvs DBUS service configuration
        writeRepositoryData(ritem);
    }

    // write to disk so other services can reparse the configuration
    m_serviceConfig->sync();

    QDialog::accept();
}


void RepositoryDialog::slotAddClicked()
{
    AddRepositoryDialog dlg(m_partConfig, QString(), this);
    // default compression level
    dlg.setCompression(-1);
    if( dlg.exec() )
    {
        QString repo      = Cervisia::NormalizeRepository(dlg.repository());
        QString rsh       = dlg.rsh();
        QString server    = dlg.server();
        int compression   = dlg.compression();
        bool retrieveFile = dlg.retrieveCvsignoreFile();

        for (int i = 0; i < m_repoList->topLevelItemCount(); i++)
            if( m_repoList->topLevelItem(i)->text(0) == repo )
            {
                KMessageBox::information(this, i18n("This repository is already known."));
                return;
            }

        RepositoryListItem* ritem = new RepositoryListItem(m_repoList, repo, false);
        ritem->setRsh(rsh);
        ritem->setCompression(compression);
        ritem->setRetrieveCvsignore(retrieveFile);

        // write entries to cvs DBUS service configuration
        writeRepositoryData(ritem);

        // write to disk so other services can reparse the configuration
        m_serviceConfig->sync();
    }
}


void RepositoryDialog::slotModifyClicked()
{
    slotDoubleClicked(m_repoList->currentItem(), 0);
}


void RepositoryDialog::slotRemoveClicked()
{
    // logout from pserver accounts so that they don't
    // get re-added because of the .cvspass file. (BR #51129)
    if( m_logoutButton->isEnabled() )
        slotLogoutClicked();

    delete m_repoList->currentItem();
}


void RepositoryDialog::slotDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)

    if( !item )
        return;

    RepositoryListItem* ritem = static_cast<RepositoryListItem*>(item);
    QString repo      = ritem->repository();
    QString rsh       = ritem->rsh();
    QString server    = ritem->server();
    int compression   = ritem->compression();
    bool retrieveFile = ritem->retrieveCvsignore();

    AddRepositoryDialog dlg(m_partConfig, repo, this);
    dlg.setRepository(repo);
    dlg.setRsh(rsh);
    dlg.setServer(server);
    dlg.setCompression(compression);
    dlg.setRetrieveCvsignoreFile(retrieveFile);
    if( dlg.exec() )
    {
        ritem->setRsh(dlg.rsh());
        ritem->setServer(dlg.server());
        ritem->setCompression(dlg.compression());
        ritem->setRetrieveCvsignore(dlg.retrieveCvsignoreFile());

        // write entries to cvs DBUS service configuration
        writeRepositoryData(ritem);

        // write to disk so other services can reparse the configuration
        m_serviceConfig->sync();
    }
}


void RepositoryDialog::slotLoginClicked()
{
    RepositoryListItem* item = (RepositoryListItem*)m_repoList->currentItem();
    if( !item )
        return;

    qCDebug(log_cervisia) << "repo=" << item->repository();

    QDBusReply<QDBusObjectPath> job = m_cvsService->login(item->repository());
    if( !job.isValid() )
        // TODO: error handling
        return;
    QDBusObjectPath jobPath = job;
    OrgKdeCervisia5CvsserviceCvsloginjobInterface cvsloginjobinterface(m_cvsServiceInterfaceName,jobPath.path(),QDBusConnection::sessionBus(), this);
    QDBusReply<bool> reply = cvsloginjobinterface.execute();
    bool success = false;
    if(reply.isValid())
        success = reply;
    if( !success )
    {
        QDBusReply<QStringList> ret = cvsloginjobinterface.output();

        QStringList output = ret;
        KMessageBox::detailedError(this, i18n("Login failed."), output.join("\n"));
        return;
    }

    item->setIsLoggedIn(true);
    slotSelectionChanged();
}


void RepositoryDialog::slotLogoutClicked()
{
    RepositoryListItem* item = (RepositoryListItem*)m_repoList->currentItem();
    if( !item )
        return;

    QDBusReply<QDBusObjectPath> job = m_cvsService->logout(item->repository());
    if( !job.isValid() )
        // TODO: error handling
        return;

    ProgressDialog dlg(this, "Logout", m_cvsService->service(),job, "logout", i18n("CVS Logout"));
    if( !dlg.execute() )
        return;

    item->setIsLoggedIn(false);
    slotSelectionChanged();
}


void RepositoryDialog::slotSelectionChanged()
{
    // retrieve the selected item
    RepositoryListItem* item = (RepositoryListItem*)m_repoList->currentItem();

    // is an item in the list view selected?
    bool isItemSelected = (item != 0);
    m_modifyButton->setEnabled(isItemSelected);
    m_removeButton->setEnabled(isItemSelected);
    m_loginButton->setEnabled(isItemSelected);
    m_logoutButton->setEnabled(isItemSelected);

    if( !isItemSelected )
        return;

    // is this a pserver repository?
    if( !LoginNeeded(item->repository()) )
    {
        m_loginButton->setEnabled(false);
        m_logoutButton->setEnabled(false);
        return;
    }

    // are we logged in?
    bool isLoggedIn = item->isLoggedIn();
    m_loginButton->setEnabled(!isLoggedIn);
    m_logoutButton->setEnabled(isLoggedIn);
}


void RepositoryDialog::writeRepositoryData(RepositoryListItem* item)
{
    // write entries to cvs DBUS service configuration
    KConfigGroup repoGroup = m_serviceConfig->group(QLatin1String("Repository-") +
                              item->repository());

    qCDebug(log_cervisia) << "repository=" << item->repository();

    repoGroup.writeEntry("rsh", item->rsh());
    repoGroup.writeEntry("cvs_server", item->server());
    repoGroup.writeEntry("Compression", item->compression());
    repoGroup.writeEntry("RetrieveCvsignore", item->retrieveCvsignore());
}


// kate: space-indent on; indent-width 4; replace-tabs on;
