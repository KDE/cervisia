/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@physik.hu-berlin.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "repositorydlg.h"

#include <stdlib.h>
#include <qhbuttongroup.h>
#include <qdir.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtextstream.h>
#include <kapplication.h>
#include <kbuttonbox.h>
#include <kconfig.h>
#include <klocale.h>

#include "repositories.h"
#include "listview.h"
#include "cervisiapart.h"


class RepositoryListItem : public QListViewItem
{
public:
    RepositoryListItem(QListView *parent, const QString &repo, bool loggedin);
    void setRsh(const QString &rsh);
    void setCompression(int compression);
    QString repository() const
    {
        return text(0);
    }
    QString rsh() const
    {
        QString str = text(1);
        return (str.left(5) == "ext (")? str.mid(5, str.length()-6) : QString::null;
    }
    int compression() const
    {
        bool ok; int n = text(2).toInt(&ok); return ok? n : -1;
    }
};


RepositoryListItem::RepositoryListItem(QListView *parent, const QString &repo, bool loggedin)
    : QListViewItem(parent)
{
    setText(0, repo);

    QString status;
    if (repo.left(9) == ":pserver:")
        status = loggedin? i18n("Logged in") : i18n("Not logged in");
    else
        status = i18n("No login required");
    setText(3, status);
}


void RepositoryListItem::setRsh(const QString &rsh)
{
    QString repo = repository();
    QString method;
    
    if (repo.left(9) == ":pserver:")
        method = "pserver";
    else if (repo.contains(':'))
        {
            method = "ext";
            if (!rsh.isEmpty())
                {
                    method += " (";
                    method += rsh;
                    method += ")";
                }
        }
    else
        method = "local";

    setText(1, method);
}


void RepositoryListItem::setCompression(int compression)
{
    QString compressionStr = (compression >= 0)?
        QString::number(compression) : i18n("Default");
    
    setText(2, compressionStr);
}


RepositoryDialog::Options *RepositoryDialog::options = 0;
AddRepositoryDialog::Options *AddRepositoryDialog::options = 0;


RepositoryDialog::RepositoryDialog(QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption(i18n("Configure access to repositories"));

    QBoxLayout *layout = new QVBoxLayout(this, 10);

    QBoxLayout *hbox = new QHBoxLayout(10);
    layout->addLayout(hbox, 10);

    repolist = new ListView(this);
    hbox->addWidget(repolist, 10);
    QFontMetrics fm(repolist->fontMetrics());
    repolist->setMinimumWidth(fm.width("X")*60);
    repolist->setMinimumHeight(repolist->sizeHint().height());
    repolist->setAllColumnsShowFocus(true);
    repolist->setPreferredColumn(0);
    repolist->addColumn(i18n("Repository"));
    repolist->addColumn(i18n("Method"));
    repolist->addColumn(i18n("Compression"));
    repolist->addColumn(i18n("Status"));
    repolist->setFocus();

    KButtonBox *actionbox = new KButtonBox(this, KButtonBox::Vertical);
    actionbox->addStretch();
    QPushButton *addbutton = actionbox->addButton(i18n("&Add..."));
    QPushButton *removebutton = actionbox->addButton(i18n("&Remove"));
    QPushButton *settingsbutton = actionbox->addButton(i18n("&Settings..."));
#if 0
    actionbox->addStretch();
    QPushButton *loginbutton = actionbox->addButton(i18n("Login..."));
    QPushButton *logoutbutton = actionbox->addButton(i18n("Logout..."));
#endif
    actionbox->addStretch();
    actionbox->layout();
    hbox->addWidget(actionbox, 0);


    connect( addbutton, SIGNAL(clicked()),
             this, SLOT(slotAddClicked()) );
    connect( removebutton, SIGNAL(clicked()),
             this, SLOT(slotRemoveClicked()) );
    connect( settingsbutton, SIGNAL(clicked()),
             this, SLOT(slotSettingsClicked()) );
#if 0
    connect( loginbutton, SIGNAL(clicked()),
             this, SLOT(slotLoginClicked()) );
    connect( logoutbutton, SIGNAL(clicked()),
             this, SLOT(slotLogoutClicked()) );
#endif

    readCvsPassFile();
    readConfigFile();
    
    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    frame->setMinimumHeight(frame->sizeHint().height());
    layout->addWidget(frame, 0);

    KButtonBox *buttonbox = new KButtonBox(this);
    buttonbox->addStretch();
    QPushButton *ok = buttonbox->addButton(i18n("OK"));
    QPushButton *cancel = buttonbox->addButton(i18n("Cancel"));
    ok->setDefault(true);
    connect( ok, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancel, SIGNAL(clicked()), this, SLOT(reject()) );
    buttonbox->layout();
    buttonbox->setFixedHeight(buttonbox->height());
    layout->addWidget(buttonbox, 0);

    layout->activate();
    resize(sizeHint());

    
    if (options)
        resize(options->size);
}


void RepositoryDialog::done(int r)
{
    if (r == Accepted)
        {
            // Make list of repositories
            QListViewItem *item;
            QStrList list;
            for (item = repolist->firstChild(); item; item = item->nextSibling())
                list.append(item->text(0).latin1());

            KConfig *config = CervisiaPart::config();
            config->setGroup("Repositories");
            config->writeEntry("Repos", list);

            for (item = repolist->firstChild(); item; item = item->nextSibling())
                {
                    RepositoryListItem *ritem = static_cast<RepositoryListItem*>(item);
                    config->setGroup(QString("Repository-") + ritem->repository());
                    config->writeEntry("rsh", ritem->rsh());
                    config->writeEntry("Compression", ritem->compression());
                }
        }
    
    if (!options)
        options = new Options;
    options->size = size();
    QDialog::done(r);
}


void RepositoryDialog::loadOptions(KConfig *config)
{
    if (!config->readEntry("Customized"))
        return;

    options = new Options;
    options->size = config->readSizeEntry("Size");
}


void RepositoryDialog::saveOptions(KConfig *config)
{
    if (!options)
        return;
    
    config->writeEntry("Customized", true);
    config->writeEntry("Size", options->size);
}


void RepositoryDialog::readCvsPassFile()
{
    QStringList list = Repositories::readCvsPassFile();
    QStringList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it)
        (void) new RepositoryListItem(repolist, (*it), true);
}


void RepositoryDialog::readConfigFile()
{
    QStringList list = Repositories::readConfigFile();
    
    // Sort out all list elements which are already in the list view
    QListViewItem *item = repolist->firstChild();
    for ( ; item; item = item->nextSibling())
        list.remove(item->text(0));

    QStringList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it)
        new RepositoryListItem(repolist, *it, false);
            
    // Now look for the used methods
    item = repolist->firstChild();
    for (; item; item = item->nextSibling())
        {
            RepositoryListItem *ritem = static_cast<RepositoryListItem*>(item);

            KConfig *config = CervisiaPart::config();
            config->setGroup(QString("Repository-") + ritem->repository());
            QString rsh = config->readEntry("rsh", QString());
            int compression = config->readNumEntry("Compression", -1);

            ritem->setRsh(rsh);
            ritem->setCompression(compression);
        }
}


void RepositoryDialog::slotAddClicked()
{
    AddRepositoryDialog dlg(QString::null, this);
    if (dlg.exec())
        {
            QString repo = dlg.repository();
            QString rsh = dlg.rsh();
            int compression = dlg.compression();
            
            QListViewItem *item = repolist->firstChild();
            for ( ; item; item = item->nextSibling())
                if (item->text(0) == repo)
                    {
                        QMessageBox::information(this, "Cervisia",
                                                 i18n("This repository is already known."));
                        return;
                    }
            
            RepositoryListItem *ritem = new RepositoryListItem(repolist, repo, false);
            ritem->setRsh(rsh);
            ritem->setCompression(compression);

            KConfig *config = CervisiaPart::config();
            config->setGroup(QString("Repository-") + repo);
            config->writeEntry("rsh", rsh);
            config->writeEntry("Compression", compression);
        }
}


void RepositoryDialog::slotRemoveClicked()
{
    delete repolist->currentItem();
}


void RepositoryDialog::slotDoubleClicked(QListViewItem *item)
{
    if (!item)
        return;

    RepositoryListItem *ritem = static_cast<RepositoryListItem*>(item);
    QString repo = ritem->repository();
    QString rsh = ritem->rsh();
    int compression = ritem->compression();
    
    AddRepositoryDialog dlg(repo, this);
    dlg.setRepository(repo);
    dlg.setRsh(rsh);
    dlg.setCompression(compression);
    if (dlg.exec())
        {
            ritem->setRsh(dlg.rsh());
            ritem->setCompression(dlg.compression());
            
            KConfig *config = CervisiaPart::config();
            config->setGroup(QString("Repository-") + repo);
            config->writeEntry("rsh", dlg.rsh());
            config->writeEntry("Compression", dlg.compression());
        }
}


void RepositoryDialog::slotSettingsClicked()
{
    slotDoubleClicked(repolist->currentItem());
}


void RepositoryDialog::slotLoginClicked()
{
}


void RepositoryDialog::slotLogoutClicked()
{
}


AddRepositoryDialog::AddRepositoryDialog(const QString &repo, QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption(i18n("Add Repository"));

    QBoxLayout *layout = new QVBoxLayout(this, 10);

    QLabel *repo_label = new QLabel(i18n("&Repository:"), this);
    layout->addWidget(repo_label);
    
    repo_edit = new KLineEdit(this);
    repo_edit->setFocus();
    repo_label->setBuddy(repo_edit);
    if (!repo.isNull())
        {
            repo_edit->setText(repo);
            repo_edit->setEnabled(false);
        }
    layout->addWidget(repo_edit);
    
    QLabel *rsh_label = new QLabel(i18n("Use remote &shell: (only for :ext: repositories)"), this);
    layout->addWidget(rsh_label);
    
    rsh_edit = new KLineEdit(this);
    rsh_label->setBuddy(rsh_edit);
    layout->addWidget(rsh_edit);

    compression_group = new QHButtonGroup(i18n("&Compression Level:"), this);
    layout->addWidget(compression_group);

    (void) new QRadioButton(i18n("Default"), compression_group);
    (void) new QRadioButton(i18n("0"), compression_group);
    (void) new QRadioButton(i18n("1"), compression_group);
    (void) new QRadioButton(i18n("2"), compression_group);
    (void) new QRadioButton(i18n("3"), compression_group);

    KButtonBox *buttonbox = new KButtonBox(this);
    buttonbox->addStretch();
    QPushButton *ok = buttonbox->addButton(i18n("OK"));
    QPushButton *cancel = buttonbox->addButton(i18n("Cancel"));
    ok->setDefault(true);
    connect( ok, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancel, SIGNAL(clicked()), this, SLOT(reject()) );
    buttonbox->layout();
    buttonbox->setFixedHeight(buttonbox->height());
    layout->addWidget(buttonbox, 0);

    connect( repo_edit, SIGNAL(textChanged(const QString&)),
             this, SLOT(repoChanged()) );
    repoChanged();

    layout->activate();
    resize(sizeHint());

    if (options)
        resize(options->size);
}


void AddRepositoryDialog::done(int r)
{
    if (!options)
        options = new Options;
    options->size = size();
    QDialog::done(r);
}


void AddRepositoryDialog::loadOptions(KConfig *config)
{
    if (!config->readEntry("Customized"))
        return;

    options = new Options;
    options->size = config->readSizeEntry("Size");
}


void AddRepositoryDialog::saveOptions(KConfig *config)
{
    if (!options)
        return;
    
    config->writeEntry("Customized", true);
    config->writeEntry("Size", options->size);
}


void AddRepositoryDialog::setRepository(const QString &repo)
{
    setCaption(i18n("Repository settings"));

    repo_edit->setText(repo);
    repo_edit->setEnabled(false);
}


void AddRepositoryDialog::repoChanged()
{
    QString repo = repository();
    rsh_edit->setEnabled(repo.left(9) != ":pserver:"
                         && repo.contains(":"));
    compression_group->setEnabled(repo.contains(":"));

    KConfig *config = CervisiaPart::config();
    config->setGroup(QString("Repository-") + repo);

    int n = config->readNumEntry("Compression", -1);
    compression_group->setButton(n+1);
}

#include "repositorydlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
