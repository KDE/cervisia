/*
 *  Copyright (C) 1999-2001 Bernd Gehrmann
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


#include <stdlib.h>
#include <qdir.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qtextstream.h>
#include <kapp.h>
#include <kbuttonbox.h>
#include <klocale.h>

#include "repositories.h"
#include "listview.h"

#include "repositorydlg.h"
#include "repositorydlg.moc"

#include "repositorysettingsdlg.h"


RepositoryDialog::Options *RepositoryDialog::options = 0;
AddRepositoryDialog::Options *AddRepositoryDialog::options = 0;


RepositoryDialog::RepositoryDialog(QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption( i18n("Configure Access to Repositories") );

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
    repolist->addColumn("Repository");
    repolist->addColumn("Method");
    repolist->addColumn("Status");
    repolist->setFocus();

    connect(repolist,SIGNAL(selectionChanged ()),
            this,SLOT(slotSelectionChanged()));

    connect(repolist,SIGNAL(doubleClicked ( QListViewItem * )),
            this,SLOT(slotDoubleClicked(QListViewItem *)));

    KButtonBox *actionbox = new KButtonBox(this, KButtonBox::Vertical);
    actionbox->addStretch();
    QPushButton *addbutton = actionbox->addButton(i18n("&Add..."));
    removebutton = actionbox->addButton(i18n("&Remove"));
    settingsbutton = actionbox->addButton(i18n("&Settings..."));
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
    slotSelectionChanged();
}

void RepositoryDialog::slotDoubleClicked(QListViewItem *item)
{
    if(item)
        slotSettingsClicked();
}

void RepositoryDialog::slotSelectionChanged()
{
    bool state=(repolist->currentItem()!=0);
    removebutton->setEnabled(state);
    settingsbutton->setEnabled(state);
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

            KConfig *config = kapp->config();
            config->setGroup("Repositories");
            config->writeEntry("Repos", list);

            for (item = repolist->firstChild(); item; item = item->nextSibling())
                {
                    QString str = item->text(1);
                    if (str.left(5) != "ext (")
                        continue;

                    config->setGroup(QString("Repository-") + item->text(0));
                    config->writeEntry("rsh", str.mid(5,str.length()-6));
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
    QStrList list1;
    Repositories::readCvsPassFile(&list1);
    QStrListIterator it1(list1);
    for (; it1.current(); ++it1)
        new QListViewItem(repolist, it1.current(), "pserver", i18n("Logged in"));
}


void RepositoryDialog::readConfigFile()
{
    QStrList list;
    Repositories::readConfigFile(&list);

    // Sort out all list elements which are already in the list view
    QListViewItem *item = repolist->firstChild();
    for ( ; item; item = item->nextSibling())
        list.remove(item->text(0).latin1());

    // Now look for the used methods
    QStrListIterator it(list);
    for (; it.current(); ++it)
        {
            QString repo = it.current();
            QString method;
            QString status;
            if (repo.left(9) == ":pserver:")
                {
                    status = i18n("Not logged in");
                    method = "pserver";
                }
            else
                {
                    status = i18n("No login required");
                    if (repo.contains(':'))
                        {
                            method = "ext";
                            KConfig *config = kapp->config();
                            config->setGroup("Repository-" + repo);
                            QString rsh = config->readEntry("rsh");
                            if (!rsh.isEmpty())
                                {
                                    method += " (";
                                    method += rsh;
                                    method += ")";
                                }
                        }
                    else
                        method = "local";
                }

            new QListViewItem(repolist, repo, method, status);
        }
}


void RepositoryDialog::slotAddClicked()
{
    AddRepositoryDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted)
        {
            QString repo = dlg.repository();

            QListViewItem *item = repolist->firstChild();
            for ( ; item; item = item->nextSibling())
                if (item->text(0) == repo)
                    {
                        QMessageBox::information(this, "Cervisia",
                                                 i18n("This repository is already known."));
                        return;
                    }

            QString method;
            QString status;
            if (repo.left(9) == ":pserver:")
                {
                    status = i18n("Not logged in");
                    method = "pserver";
                }
            else
                {
                    status = i18n("No login required");
                    if (repo.contains(':'))
                        {
                            method = "ext";
                            if (!dlg.rsh().isEmpty())
                                {
                                    method += " (";
                                    method += dlg.rsh();
                                    method += ")";
                                }
                        }
                    else
                        method = "local";
                }

            new QListViewItem(repolist, repo, method, status);

        }
}


void RepositoryDialog::slotRemoveClicked()
{
    delete repolist->currentItem();
    slotSelectionChanged();
}


void RepositoryDialog::slotSettingsClicked()
{
    QListViewItem* item = repolist->currentItem();

    if (item)
        {
            // create a repository settings dialog for the chosen repository
            RepositorySettingsDialog dlg(this);
            dlg.setRepository(item->text(0));
            dlg.exec();
        }
}

void RepositoryDialog::slotLoginClicked()
{
}


void RepositoryDialog::slotLogoutClicked()
{
}


AddRepositoryDialog::AddRepositoryDialog(QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption( i18n("Add Repository") );

    QBoxLayout *layout = new QVBoxLayout(this, 10);

    QLabel *repo_label = new QLabel(i18n("&Repository:"), this);
    repo_label->setMinimumSize(repo_label->sizeHint());
    layout->addWidget(repo_label);

    repo_edit = new KLineEdit(this);
    repo_edit->setFocus();
    repo_label->setBuddy(repo_edit);
    repo_edit->setMinimumSize(repo_edit->sizeHint());
    layout->addWidget(repo_edit);

    QLabel *rsh_label = new QLabel(i18n("Use remote &shell: (only for :ext: repositories)"), this);
    rsh_label->setMinimumSize(rsh_label->sizeHint());
    layout->addWidget(rsh_label);

    rsh_edit = new KLineEdit(this);
    rsh_label->setBuddy(rsh_edit);
    rsh_edit->setMinimumSize(rsh_edit->sizeHint());
    layout->addWidget(rsh_edit);

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    frame->setMinimumHeight(frame->sizeHint().height());
    layout->addWidget(frame, 0);

    KButtonBox *buttonbox = new KButtonBox(this);
    buttonbox->addStretch();
    ok = buttonbox->addButton(i18n("OK"));
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
    ok->setEnabled(!repo_edit->text().isEmpty());
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


void AddRepositoryDialog::repoChanged()
{
    QString repo = repository();
    rsh_edit->setEnabled(repo.left(9) != ":pserver:"
                         && repo.contains(":"));
    ok->setEnabled(!repo.isEmpty());
}


// Local Variables:
// c-basic-offset: 4
// End:


