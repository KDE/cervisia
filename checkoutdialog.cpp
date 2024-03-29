/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2003-2007 Christian Loose <christian.loose@hamburg.de>
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

#include "checkoutdialog.h"

// Qt
#include <KComboBox>
#include <QBoxLayout>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

// KDE
#include <KConfigGroup>
#include <KHelpClient>
#include <KLineEdit>
#include <KLocalizedString>
#include <kmessagebox.h>
#include <kurlcompletion.h>

#include "cervisiasettings.h"
#include "cvsserviceinterface.h"
#include "misc.h"
#include "progressdialog.h"
#include "repositories.h"

using Cervisia::IsValidTag;

CheckoutDialog::CheckoutDialog(KConfig &cfg, OrgKdeCervisia5CvsserviceCvsserviceInterface *service, ActionType action, QWidget *parent)
    : QDialog(parent)
    , act(action)
    , partConfig(cfg)
    , cvsService(service)
{
    setWindowTitle((action == Checkout) ? i18n("CVS Checkout") : i18n("CVS Import"));
    setModal(true);

    auto mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttonBox, &QDialogButtonBox::helpRequested, this, &CheckoutDialog::slotHelp);

    auto grid = new QGridLayout;
    mainLayout->addLayout(grid);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 20);
    for (int i = 0; i < ((action == Checkout) ? 4 : 10); ++i)
        grid->setRowStretch(i, 0);

    repo_combo = new KComboBox;
    repo_combo->setEditable(true);
    repo_combo->setFocus();
    // make sure combobox is smaller than the screen
    repo_combo->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    grid->addWidget(repo_combo, 0, 1);

    auto repo_label = new QLabel(i18n("&Repository:"));
    repo_label->setBuddy(repo_combo);
    grid->addWidget(repo_label, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);

    if (action == Import) {
        module_edit = new QLineEdit;
        module_edit->setClearButtonEnabled(true);
        grid->addWidget(module_edit, 1, 1);
        auto module_label = new QLabel(i18n("&Module:"));
        module_label->setBuddy(module_edit);
        grid->addWidget(module_label, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
    } else {
        module_combo = new KComboBox;
        module_combo->setEditable(true);

        auto module_button = new QPushButton(i18n("Fetch &List"));
        connect(module_button, SIGNAL(clicked()), this, SLOT(moduleButtonClicked()));

        QBoxLayout *module_layout = new QHBoxLayout();
        grid->addLayout(module_layout, 1, 1);
        module_layout->addWidget(module_combo, 10);
        module_layout->addWidget(module_button, 0, Qt::AlignVCenter);

        auto module_label = new QLabel(i18n("&Module:"));
        module_label->setBuddy(module_combo);
        grid->addWidget(module_label, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);

        branchCombo = new KComboBox;
        branchCombo->setEditable(true);

        auto branchButton = new QPushButton(i18n("Fetch &List"));
        connect(branchButton, SIGNAL(clicked()), this, SLOT(branchButtonClicked()));

        QBoxLayout *branchLayout = new QHBoxLayout();
        grid->addLayout(branchLayout, 2, 1);
        branchLayout->addWidget(branchCombo, 10);
        branchLayout->addWidget(branchButton, 0, Qt::AlignVCenter);

        auto branch_label = new QLabel(i18n("&Branch tag:"));
        branch_label->setBuddy(branchCombo);
        grid->addWidget(branch_label, 2, 0, Qt::AlignLeft | Qt::AlignVCenter);

        connect(branchCombo, SIGNAL(editTextChanged(QString)), this, SLOT(branchTextChanged()));

        recursive_box = new QCheckBox(i18n("Re&cursive checkout"));
        grid->addWidget(recursive_box, 6, 0, 1, 2);
    }

    workdir_edit = new KLineEdit;
    workdir_edit->setClearButtonEnabled(true);
    workdir_edit->setText(QDir::homePath());
    workdir_edit->setMinimumWidth(fontMetrics().width('X') * 40);

    auto comp = new KUrlCompletion();
    workdir_edit->setCompletionObject(comp);
    workdir_edit->setAutoDeleteCompletionObject(true);
    connect(workdir_edit, SIGNAL(returnPressed(QString)), comp, SLOT(addItem(QString)));

    auto dir_button = new QPushButton("...");
    connect(dir_button, SIGNAL(clicked()), this, SLOT(dirButtonClicked()));
    dir_button->setFixedWidth(30);

    QBoxLayout *workdir_layout = new QHBoxLayout();
    grid->addLayout(workdir_layout, (action == Import) ? 2 : 3, 1);
    workdir_layout->addWidget(workdir_edit, 10);
    workdir_layout->addWidget(dir_button, 0, Qt::AlignVCenter);

    auto workdir_label = new QLabel(i18n("Working &folder:"));
    workdir_label->setBuddy(workdir_edit);
    grid->addWidget(workdir_label, (action == Import) ? 2 : 3, 0, Qt::AlignLeft | Qt::AlignVCenter);

    if (action == Import) {
        vendortag_edit = new QLineEdit;
        vendortag_edit->setClearButtonEnabled(true);
        grid->addWidget(vendortag_edit, 3, 1);

        auto vendortag_label = new QLabel(i18n("&Vendor tag:"));
        vendortag_label->setBuddy(vendortag_edit);
        grid->addWidget(vendortag_label, 3, 0, Qt::AlignLeft | Qt::AlignVCenter);

        releasetag_edit = new QLineEdit;
        releasetag_edit->setClearButtonEnabled(true);
        grid->addWidget(releasetag_edit, 4, 1);

        auto releasetag_label = new QLabel(i18n("&Release tag:"));
        releasetag_label->setBuddy(releasetag_edit);
        grid->addWidget(releasetag_label, 4, 0, Qt::AlignLeft | Qt::AlignVCenter);

        ignore_edit = new QLineEdit;
        ignore_edit->setClearButtonEnabled(true);
        grid->addWidget(ignore_edit, 5, 1);

        auto ignore_label = new QLabel(i18n("&Ignore files:"));
        ignore_label->setBuddy(ignore_edit);
        grid->addWidget(ignore_label, 5, 0, Qt::AlignLeft | Qt::AlignVCenter);

        comment_edit = new QLineEdit;
        comment_edit->setClearButtonEnabled(true);
        grid->addWidget(comment_edit, 6, 1);

        auto comment_label = new QLabel(i18n("&Comment:"));
        comment_label->setBuddy(comment_edit);
        grid->addWidget(comment_label, 6, 0, Qt::AlignLeft | Qt::AlignVCenter);

        binary_box = new QCheckBox(i18n("Import as &binaries"));
        grid->addWidget(binary_box, 7, 0, 1, 2);

        m_useModificationTimeBox = new QCheckBox(i18n("Use file's modification time as time of import"));
        grid->addWidget(m_useModificationTimeBox, 8, 0, 1, 2);
    } else {
        alias_edit = new QLineEdit;
        alias_edit->setClearButtonEnabled(true);
        grid->addWidget(alias_edit, 4, 1);

        auto alias_label = new QLabel(i18n("Chec&k out as:"));
        alias_label->setBuddy(alias_edit);
        grid->addWidget(alias_label, 4, 0, Qt::AlignLeft | Qt::AlignVCenter);

        export_box = new QCheckBox(i18n("Ex&port only"));
        grid->addWidget(export_box, 5, 0, 1, 2);
    }

    mainLayout->addWidget(buttonBox);

    QStringList list1 = Repositories::readCvsPassFile();
    QStringList::ConstIterator it1;
    for (it1 = list1.constBegin(); it1 != list1.constEnd(); ++it1)
        repo_combo->addItem(*it1);

    QStringList list2 = Repositories::readConfigFile();
    QStringList::ConstIterator it2;
    for (it2 = list2.constBegin(); it2 != list2.constEnd(); ++it2)
        if (!list1.contains(*it2))
            repo_combo->addItem(*it2);

    helpTopic = (act == Import) ? "importing" : "checkingout";

    restoreUserInput();
    connect(okButton, SIGNAL(clicked()), this, SLOT(slotOk()));
}

void CheckoutDialog::slotHelp()
{
    KHelpClient::invokeHelp(helpTopic);
}

QString CheckoutDialog::workingDirectory() const
{
    return workdir_edit->text();
}

QString CheckoutDialog::repository() const
{
    return repo_combo->currentText();
}

QString CheckoutDialog::module() const
{
    return act == Import ? module_edit->text() : module_combo->currentText();
}

QString CheckoutDialog::branch() const
{
    return branchCombo->currentText();
}

QString CheckoutDialog::vendorTag() const
{
    return vendortag_edit->text();
}

QString CheckoutDialog::releaseTag() const
{
    return releasetag_edit->text();
}

QString CheckoutDialog::ignoreFiles() const
{
    return ignore_edit->text();
}

QString CheckoutDialog::comment() const
{
    return comment_edit->text();
}

QString CheckoutDialog::alias() const
{
    return alias_edit->text();
}

bool CheckoutDialog::importBinary() const
{
    return binary_box->isChecked();
}

bool CheckoutDialog::useModificationTime() const
{
    return m_useModificationTimeBox->isChecked();
}

bool CheckoutDialog::exportOnly() const
{
    if (export_box->isEnabled())
        return export_box->isChecked();

    return false;
}

bool CheckoutDialog::recursive() const
{
    return recursive_box->isChecked();
}

void CheckoutDialog::slotOk()
{
    QFileInfo fi(workingDirectory());
    if (!fi.exists() || !fi.isDir()) {
        KMessageBox::information(this, i18n("Please choose an existing working folder."));
        return;
    }
    if (module().isEmpty()) {
        KMessageBox::information(this, i18n("Please specify a module name."));
        return;
    }

    if (act == Import) {
        if (vendorTag().isEmpty() || releaseTag().isEmpty()) {
            KMessageBox::information(this, i18n("Please specify a vendor tag and a release tag."));
            return;
        }
        if (!IsValidTag(vendorTag()) || !IsValidTag(releaseTag())) {
            KMessageBox::information(this,
                                     i18n("Tags must start with a letter and may contain\n"
                                          "letters, digits and the characters '-' and '_'."));
            return;
        }
    } else {
        if (branch().isEmpty() && exportOnly()) {
            KMessageBox::information(this, i18n("A branch must be specified for export."));
            return;
        }
    }

    saveUserInput();

    QDialog::accept();
}

void CheckoutDialog::dirButtonClicked()
{
    QString dir = QFileDialog::getExistingDirectory(0, QString(), workdir_edit->text());
    if (!dir.isEmpty())
        workdir_edit->setText(dir);
}

void CheckoutDialog::moduleButtonClicked()
{
    QDBusReply<QDBusObjectPath> cvsJob = cvsService->moduleList(repository());
    if (!cvsJob.isValid())
        return;

    ProgressDialog dlg(this, "Checkout", cvsService->service(), cvsJob, "checkout", i18n("CVS Checkout"));
    if (!dlg.execute())
        return;

    module_combo->clear();
    QString str;
    while (dlg.getLine(str)) {
        if (str.left(12) == "Unknown host")
            continue;

        int pos = str.indexOf(' ');
        if (pos == -1)
            pos = str.indexOf('\t');
        if (pos == -1)
            pos = str.length();
        QString module(str.left(pos).trimmed());
        if (!module.isEmpty())
            module_combo->addItem(module);
    }
}

void CheckoutDialog::branchButtonClicked()
{
    QStringList branchTagList;

    if (repository().isEmpty()) {
        KMessageBox::information(this, i18n("Please specify a repository."));
        return;
    }

    if (module().isEmpty()) {
        KMessageBox::information(this, i18n("Please specify a module name."));
        return;
    }

    QDBusReply<QDBusObjectPath> cvsJob = cvsService->rlog(repository(), module(), false /*recursive*/);
    if (!cvsJob.isValid())
        return;

    ProgressDialog dlg(this, "Remote Log", cvsService->service(), cvsJob, QString(), i18n("CVS Remote Log"));
    if (!dlg.execute())
        return;

    QString line;
    while (dlg.getLine(line)) {
        int colonPos;

        if (line.isEmpty() || line[0] != '\t')
            continue;
        if ((colonPos = line.indexOf(':', 1)) < 0)
            continue;

        const QString tag = line.mid(1, colonPos - 1);
        if (!branchTagList.contains(tag))
            branchTagList.push_back(tag);
    }

    branchTagList.sort();

    branchCombo->clear();
    branchCombo->addItems(branchTagList);
}

void CheckoutDialog::restoreUserInput()
{
    KConfigGroup cs(&partConfig, "CheckoutDialog");

    repo_combo->setEditText(CervisiaSettings::repository());
    workdir_edit->setText(CervisiaSettings::workingFolder());

    if (act == Import) {
        module_edit->setText(CervisiaSettings::module());
        vendortag_edit->setText(cs.readEntry("Vendor tag"));
        releasetag_edit->setText(cs.readEntry("Release tag"));
        ignore_edit->setText(cs.readEntry("Ignore files"));
        binary_box->setChecked(cs.readEntry("Import binary", false));
    } else {
        module_combo->setEditText(CervisiaSettings::module());
        branchCombo->setEditText(cs.readEntry("Branch"));
        alias_edit->setText(cs.readEntry("Alias"));
        export_box->setChecked(cs.readEntry("ExportOnly", false));
        recursive_box->setChecked(true);
    }
}

void CheckoutDialog::saveUserInput()
{
    KConfigGroup cs(&partConfig, "CheckoutDialog");

    CervisiaSettings::setRepository(repository());
    CervisiaSettings::setModule(module());
    CervisiaSettings::setWorkingFolder(workingDirectory());

    CervisiaSettings::self()->save();

    if (act == Import) {
        cs.writeEntry("Vendor tag", vendorTag());
        cs.writeEntry("Release tag", releaseTag());
        cs.writeEntry("Ignore files", ignoreFiles());
        cs.writeEntry("Import binary", importBinary());
    } else {
        cs.writeEntry("Branch", branch());
        cs.writeEntry("Alias", alias());
        cs.writeEntry("ExportOnly", exportOnly());
    }
}

void CheckoutDialog::branchTextChanged()
{
    if (branch().isEmpty()) {
        export_box->setEnabled(false);
        export_box->setChecked(false);
    } else {
        export_box->setEnabled(true);
    }
}

// Local Variables:
// c-basic-offset: 4
// End:
