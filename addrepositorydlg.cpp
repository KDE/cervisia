/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2002 Christian Loose <christian.loose@hamburg.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "addrepositorydlg.h"

#include <qhbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <kconfig.h>
#include <klineedit.h>
#include <klocale.h>


AddRepositoryDialog::Options *AddRepositoryDialog::options = 0;


AddRepositoryDialog::AddRepositoryDialog(const QString &repo, QWidget *parent, const char *name)
    : KDialogBase(parent, name, true, i18n("Add Repository"),
                  Ok | Cancel, Ok, true)
{
    QFrame* mainWidget = makeMainWidget();

    QBoxLayout *layout = new QVBoxLayout(mainWidget, 0, spacingHint());

    QLabel *repo_label = new QLabel(i18n("&Repository:"), mainWidget);
    layout->addWidget(repo_label);
    
    repo_edit = new KLineEdit(mainWidget);
    repo_edit->setFocus();
    repo_label->setBuddy(repo_edit);
    if (!repo.isNull())
        {
            repo_edit->setText(repo);
            repo_edit->setEnabled(false);
        }
    layout->addWidget(repo_edit);
    
    QLabel *rsh_label = new QLabel(i18n("Use remote &shell (only for :ext: repositories):"), mainWidget);
    layout->addWidget(rsh_label);
    
    rsh_edit = new KLineEdit(mainWidget);
    rsh_label->setBuddy(rsh_edit);
    layout->addWidget(rsh_edit);

    compression_group = new QHButtonGroup(i18n("&Compression Level"), mainWidget);
    layout->addWidget(compression_group);

    (void) new QRadioButton(i18n("Default"), compression_group);
    (void) new QRadioButton(i18n("0"), compression_group);
    (void) new QRadioButton(i18n("1"), compression_group);
    (void) new QRadioButton(i18n("2"), compression_group);
    (void) new QRadioButton(i18n("3"), compression_group);

    // open cvs DCOP service configuration file
    serviceConfig = new KConfig("cvsservicerc");

    connect( repo_edit, SIGNAL(textChanged(const QString&)),
             this, SLOT(repoChanged()) );
    repoChanged();

    if (options)
        resize(options->size);
}


AddRepositoryDialog::~AddRepositoryDialog()
{
    if (!options)
        options = new Options;
    options->size = size();
    
    delete serviceConfig;
}


void AddRepositoryDialog::setRsh(const QString &rsh)
{
    rsh_edit->setText(rsh);
}


void AddRepositoryDialog::setCompression(int compression)
{
    compression_group->setButton(compression + 1);
}


QString AddRepositoryDialog::repository() const
{
    return repo_edit->text();
}


QString AddRepositoryDialog::rsh() const
{
    return rsh_edit->text();
}


int AddRepositoryDialog::compression() const
{
    return compression_group->id(compression_group->selected()) - 1;
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
    setCaption(i18n("Repository Settings"));

    repo_edit->setText(repo);
    repo_edit->setEnabled(false);
}


void AddRepositoryDialog::repoChanged()
{
    QString repo = repository();
    rsh_edit->setEnabled((!repo.startsWith(":pserver:"))
                         && repo.contains(":"));
    compression_group->setEnabled(repo.contains(":"));

    // read compression level from cvs DCOP service configuration
    serviceConfig->setGroup(QString::fromLatin1("Repository-") + repo);
    int n = serviceConfig->readNumEntry("Compression", -1);
    
    compression_group->setButton(n+1);
}

#include "addrepositorydlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
