/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2003-2004 Christian Loose <christian.loose@hamburg.de>
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


#include "checkoutdlg.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <kprocess.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurlcompletion.h>

#include "progressdlg.h"
#include "repositories.h"
#include "misc.h"
#include "cvsservice_stub.h"

using Cervisia::IsValidTag;


CheckoutDialog::CheckoutDialog(KConfig& cfg, CvsService_stub* service,
                               ActionType action, QWidget* parent,
                               const char* name)
    : KDialogBase(parent, name, true, QString::null,
                  Ok | Cancel | Help, Ok, true)
    , act(action)
    , partConfig(cfg)
    , cvsService(service)
{
    setCaption( (action==Checkout)? i18n("CVS Checkout") : i18n("CVS Import") );

    QFrame* mainWidget = makeMainWidget();

    QBoxLayout* layout = new QVBoxLayout(mainWidget, 0, spacingHint());

    QGridLayout* grid = new QGridLayout(layout);
    grid->setColStretch(0, 1);
    grid->setColStretch(1, 20);
    for( int i = 0; i < ((action==Checkout)? 4 : 10); ++i )
        grid->setRowStretch(i, 0);

    repo_combo = new QComboBox(true, mainWidget);
    repo_combo->setFocus();
    // make sure combobox is smaller than the screen
    repo_combo->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    grid->addWidget(repo_combo, 0, 1);

    QLabel* repo_label = new QLabel(repo_combo, i18n("&Repository:"), mainWidget);
    grid->addWidget(repo_label, 0, 0, AlignLeft | AlignVCenter);

    if( action == Import )
    {
        module_edit = new KLineEdit(mainWidget);
        grid->addWidget(module_edit, 1, 1);
        QLabel* module_label = new QLabel(module_edit, i18n("&Module:"), mainWidget);
        grid->addWidget(module_label, 1, 0, AlignLeft | AlignVCenter);
    }
    else
    {
        module_combo = new QComboBox(true, mainWidget);

        QPushButton* module_button = new QPushButton(i18n("Fetch &List"), mainWidget);
        connect( module_button, SIGNAL(clicked()),
                 this, SLOT(moduleButtonClicked()) );

        QBoxLayout* module_layout = new QHBoxLayout();
        grid->addLayout(module_layout, 1, 1);
        module_layout->addWidget(module_combo, 10);
        module_layout->addWidget(module_button, 0, AlignVCenter);

        QLabel* module_label = new QLabel(module_combo, i18n("&Module:"), mainWidget);
        grid->addWidget(module_label, 1, 0, AlignLeft | AlignVCenter);

        branchCombo = new QComboBox(true, mainWidget);

        QPushButton* branchButton = new QPushButton(i18n("Fetch &List"), mainWidget);
        connect( branchButton, SIGNAL(clicked()),
                 this, SLOT(branchButtonClicked()) );

        QBoxLayout* branchLayout = new QHBoxLayout();
        grid->addLayout(branchLayout, 2, 1);
        branchLayout->addWidget(branchCombo, 10);
        branchLayout->addWidget(branchButton, 0, AlignVCenter);

        QLabel* branch_label = new QLabel(branchCombo, i18n("&Branch tag:"), 
                                          mainWidget);
        grid->addWidget(branch_label, 2, 0, AlignLeft | AlignVCenter);

        connect( branchCombo, SIGNAL( textChanged( const QString&)),
                 this, SLOT( branchTextChanged() ));

        recursive_box = new QCheckBox(i18n("Re&cursive checkout"), mainWidget);
        grid->addMultiCellWidget(recursive_box, 6, 6, 0, 1);
    }

    workdir_edit = new KLineEdit(mainWidget);
    workdir_edit->setText(QDir::homeDirPath());
    workdir_edit->setMinimumWidth(fontMetrics().width('X') * 40);
    
    KURLCompletion* comp = new KURLCompletion();
    workdir_edit->setCompletionObject(comp);
    workdir_edit->setAutoDeleteCompletionObject(true);
    connect( workdir_edit, SIGNAL(returnPressed(const QString&)),
             comp, SLOT(addItem(const QString&)) );

    QPushButton* dir_button = new QPushButton("...", mainWidget);
    connect( dir_button, SIGNAL(clicked()),
             this, SLOT(dirButtonClicked()) );
    dir_button->setFixedWidth(30);

    QBoxLayout* workdir_layout = new QHBoxLayout();
    grid->addLayout(workdir_layout, (action==Import)? 2 : 3, 1);
    workdir_layout->addWidget(workdir_edit, 10);
    workdir_layout->addWidget(dir_button, 0, AlignVCenter);

    QLabel* workdir_label = new QLabel(workdir_edit, i18n("Working &folder:"), 
                                       mainWidget);
    grid->addWidget(workdir_label, (action==Import)? 2 : 3, 0, AlignLeft | AlignVCenter);

    if( action == Import )
    {
        vendortag_edit = new KLineEdit(mainWidget);
        grid->addWidget(vendortag_edit, 3, 1);

        QLabel* vendortag_label = new QLabel(vendortag_edit, i18n("&Vendor tag:"), 
                                             mainWidget);
        grid->addWidget(vendortag_label, 3, 0, AlignLeft | AlignVCenter);

        releasetag_edit = new KLineEdit(mainWidget);
        grid->addWidget(releasetag_edit, 4, 1);

        QLabel* releasetag_label = new QLabel(releasetag_edit, i18n("&Release tag:"),
                                              mainWidget);
        grid->addWidget(releasetag_label, 4, 0, AlignLeft | AlignVCenter);

        ignore_edit = new KLineEdit(mainWidget);
        grid->addWidget(ignore_edit, 5, 1);

        QLabel* ignore_label = new QLabel(ignore_edit, i18n("&Ignore files:"), 
                                          mainWidget);
        grid->addWidget(ignore_label, 5, 0, AlignLeft | AlignVCenter);

        comment_edit = new KLineEdit(mainWidget);
        grid->addWidget(comment_edit, 6, 1);

        QLabel* comment_label = new QLabel(comment_edit, i18n("&Comment:"), 
                                           mainWidget);
        grid->addWidget(comment_label, 6, 0, AlignLeft | AlignVCenter);

        binary_box = new QCheckBox(i18n("Import as &binaries"), mainWidget);
        grid->addMultiCellWidget(binary_box, 7, 7, 0, 1);

        m_useModificationTimeBox = new QCheckBox(
                i18n("Use file's modification time as time of import"), mainWidget);
        grid->addMultiCellWidget(m_useModificationTimeBox, 8, 8, 0, 1);
    }
    else
    {
        alias_edit = new KLineEdit(mainWidget);
        grid->addWidget(alias_edit, 4, 1);

        QLabel* alias_label = new QLabel(alias_edit, i18n("Chec&k out as:"), mainWidget);
        grid->addWidget(alias_label, 4, 0, AlignLeft | AlignVCenter);

        export_box = new QCheckBox(i18n("Ex&port only"), mainWidget);
        grid->addMultiCellWidget(export_box, 5, 5, 0, 1);
    }

    QStringList list1 = Repositories::readCvsPassFile();
    QStringList::ConstIterator it1;
    for (it1 = list1.begin(); it1 != list1.end(); ++it1)
        repo_combo->insertItem(*it1);

    QStringList list2 = Repositories::readConfigFile();
    QStringList::ConstIterator it2;
    for (it2 = list2.begin(); it2 != list2.end(); ++it2)
        if (!list1.contains(*it2))
            repo_combo->insertItem(*it2);

    setHelp((act == Import) ? "importing" : "checkingout");
    
    restoreUserInput();
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
    return act==Import? module_edit->text() : module_combo->currentText();
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
    if( export_box->isEnabled() )
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
    if (!fi.exists() || !fi.isDir())
    {
        KMessageBox::information(this, i18n("Please choose an existing working folder."));
        return;
    }
    if (module().isEmpty())
    {
        KMessageBox::information(this, i18n("Please specify a module name."));
        return;
    }

    if (act==Import)
    {
        if (vendorTag().isEmpty() || releaseTag().isEmpty())
        {
            KMessageBox::information(this,
                                     i18n("Please specify a vendor tag and a release tag."));
            return;
        }
        if (!IsValidTag(vendorTag()) || !IsValidTag(releaseTag()))
        {
            KMessageBox::information(this,
                                     i18n("Tags must start with a letter and may contain\n"
                                          "letters, digits and the characters '-' and '_'."));
            return;
        }
    }
    else
    {
        if( branch().isEmpty() && exportOnly() )
        {
            KMessageBox::information(this,
                            i18n("A branch must be specified for export."));
            return;
        }
    }
    
    saveUserInput();
    
    KDialogBase::slotOk();
}


void CheckoutDialog::dirButtonClicked()
{
    QString dir = KFileDialog::getExistingDirectory(workdir_edit->text());
    if (!dir.isEmpty())
        workdir_edit->setText(dir);
}


void CheckoutDialog::moduleButtonClicked()
{
    DCOPRef cvsJob = cvsService->moduleList(repository());
    if( !cvsService->ok() )
        return;

    ProgressDialog dlg(this, "Checkout", cvsJob, "checkout", i18n("CVS Checkout"));
    if( !dlg.execute() )
        return;

    module_combo->clear();
    QString str;
    while (dlg.getLine(str))
    {
        if (str.left(12) == "Unknown host")
            continue;

        int pos = str.find(' ');
        if (pos == -1)
            pos = str.find('\t');
        if (pos == -1)
            pos = str.length();
        QString module( str.left(pos).stripWhiteSpace() );
        if ( !module.isEmpty() )
            module_combo->insertItem(module);
    }
}


void CheckoutDialog::branchButtonClicked()
{
    QStringList branchTagList;

    if( repository().isEmpty() )
    {
        KMessageBox::information(this, i18n("Please specify a repository."));
        return;
    }

    if( module().isEmpty() )
    {
        KMessageBox::information(this, i18n("Please specify a module name."));
        return;
    }

    DCOPRef cvsJob = cvsService->rlog(repository(), module(), 
                                      false/*recursive*/);
    if( !cvsService->ok() )
        return;

    ProgressDialog dlg(this, "Remote Log", cvsJob, QString::null, 
                       i18n("CVS Remote Log"));
    if( !dlg.execute() )
        return;

    QString line;
    while( dlg.getLine(line) )
    {
        int colonPos;

        if( line.isEmpty() || line[0] != '\t' )
            continue;
        if( (colonPos = line.find(':', 1)) < 0 )
           continue;

        const QString tag  = line.mid(1, colonPos - 1);
        if( !branchTagList.contains(tag) )
            branchTagList.push_back(tag);
    }

    branchTagList.sort();

    branchCombo->clear();
    branchCombo->insertStringList(branchTagList);
}


void CheckoutDialog::restoreUserInput()
{
    KConfigGroupSaver cs(&partConfig, "CheckoutDialog");

    repo_combo->setEditText(partConfig.readEntry("Repository"));
    workdir_edit->setText(partConfig.readPathEntry("Working directory"));

    if (act == Import)
    {
        module_edit->setText(partConfig.readEntry("Module"));
        vendortag_edit->setText(partConfig.readEntry("Vendor tag"));
        releasetag_edit->setText(partConfig.readEntry("Release tag"));
        ignore_edit->setText(partConfig.readEntry("Ignore files"));
        binary_box->setChecked(partConfig.readBoolEntry("Import binary"));
    }
    else
    {
        module_combo->setEditText(partConfig.readEntry("Module"));
        branchCombo->setCurrentText(partConfig.readEntry("Branch"));
        alias_edit->setText(partConfig.readEntry("Alias"));
        export_box->setChecked(partConfig.readBoolEntry("ExportOnly"));
        recursive_box->setChecked(true);
    }
}


void CheckoutDialog::saveUserInput()
{
    KConfigGroupSaver cs(&partConfig, "CheckoutDialog");

    partConfig.writeEntry("Repository", repository());
    partConfig.writeEntry("Module", module());
    partConfig.writeEntry("Working directory", workingDirectory());

    if (act == Import)
    {
        partConfig.writeEntry("Vendor tag", vendorTag());
        partConfig.writeEntry("Release tag", releaseTag());
        partConfig.writeEntry("Ignore files", ignoreFiles());
        partConfig.writeEntry("Import binary", importBinary());
    }
    else
    {
        partConfig.writeEntry("Branch", branch());
        partConfig.writeEntry("Alias", alias());
        partConfig.writeEntry("ExportOnly", exportOnly());
    }
}

void CheckoutDialog::branchTextChanged()
{
    if( branch().isEmpty() )
    {
        export_box->setEnabled(false);
        export_box->setChecked(false);
    }
    else
    {
        export_box->setEnabled(true);
    }
}


#include "checkoutdlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
