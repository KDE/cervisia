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


#include "checkoutdlg.h"

#include <qcombobox.h>
#include <qdir.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmultilinedit.h>
#include <qpushbutton.h>
#include <kapplication.h>
#include <kprocess.h>
#include <kbuttonbox.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "cvsprogressdlg.h"
#include "repositories.h"
#include "misc.h"


CheckoutDialog::Options *CheckoutDialog::options = 0;


CheckoutDialog::CheckoutDialog(ActionType action, QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption( (action==Checkout)? i18n("CVS Checkout") : i18n("CVS Import") );

    QBoxLayout *layout = new QVBoxLayout(this, 10);
    QFontMetrics fm(fontMetrics());

    QGridLayout *grid = new QGridLayout((action==Checkout)? 4 : 9, 2, 10);
    layout->addLayout(grid);
    grid->setColStretch(0, 1);
    grid->setColStretch(1, 20);
    for (int i = 0; i < ((action==Checkout)? 4 : 9); ++i)
        grid->setRowStretch(i, 0);

    repo_combo = new QComboBox(true, this);
    repo_combo->setFocus();
    grid->addWidget(repo_combo, 0, 1);
    
    QLabel *repo_label = new QLabel
	(repo_combo, i18n("&Repository:"), this);
    grid->addWidget(repo_label, 0, 0, AlignLeft | AlignVCenter);

    if (action == Import)
        {
            module_edit = new KLineEdit(this);
            grid->addWidget(module_edit, 1, 1);
            QLabel *module_label = new QLabel(module_edit, i18n("&Module:"), this);
            grid->addWidget(module_label, 1, 0, AlignLeft | AlignVCenter);
        }
    else
        {
            module_combo = new QComboBox(true, this);
            
            QPushButton *module_button = new QPushButton(i18n("Fetch &List"), this);
            connect( module_button, SIGNAL(clicked()),
                     this, SLOT(moduleButtonClicked()) );
            
            QBoxLayout *module_layout = new QHBoxLayout();
            grid->addLayout(module_layout, 1, 1);
            module_layout->addWidget(module_combo, 10);
            module_layout->addWidget(module_button, 0, AlignVCenter);
 
            QLabel *module_label = new QLabel(module_combo, i18n("&Module:"), this);
            grid->addWidget(module_label, 1, 0, AlignLeft | AlignVCenter);

            branch_edit = new KLineEdit(this);
            grid->addWidget(branch_edit, 2, 1);

            QLabel *branch_label = new QLabel(branch_edit, i18n("&Branch tag:"), this);
            branch_label->setFixedSize(branch_label->sizeHint());
            grid->addWidget(branch_label, 2, 0, AlignLeft | AlignVCenter);
        }
    
    workdir_edit = new KLineEdit(this);
    workdir_edit->setText(QDir::homeDirPath());
    workdir_edit->setMinimumSize(fm.width("X")*40, workdir_edit->sizeHint().height());

    QPushButton *dir_button = new QPushButton("...", this);
    connect( dir_button, SIGNAL(clicked()),
	     this, SLOT(dirButtonClicked()) );
    dir_button->setFixedHeight(workdir_edit->sizeHint().height());
    dir_button->setFixedWidth(30);

    QBoxLayout *workdir_layout = new QHBoxLayout();
    grid->addLayout(workdir_layout, (action==Import)? 2 : 3, 1);
    workdir_layout->addWidget(workdir_edit, 10);
    workdir_layout->addWidget(dir_button, 0, AlignVCenter);

    QLabel *workdir_label = new QLabel
	(workdir_edit, i18n("Working &directory:"), this);
    grid->addWidget(workdir_label, (action==Import)? 2 : 3, 0, AlignLeft | AlignVCenter);

    if (action == Import)
        {
            vendortag_edit = new KLineEdit(this);
            grid->addWidget(vendortag_edit, 3, 1);

            QLabel *vendortag_label = new QLabel
                (vendortag_edit, i18n("&Vendor tag:"), this);
            grid->addWidget(vendortag_label, 3, 0, AlignLeft | AlignVCenter);

            releasetag_edit = new KLineEdit(this);
            grid->addWidget(releasetag_edit, 4, 1);

            QLabel *releasetag_label = new QLabel
                (releasetag_edit, i18n("&Release tag:"), this);
            grid->addWidget(releasetag_label, 4, 0, AlignLeft | AlignVCenter);

            ignore_edit = new KLineEdit(this);
            grid->addWidget(ignore_edit, 5, 1);
            
            QLabel *ignore_label = new QLabel
                (ignore_edit, i18n("&Ignore files:"), this);
            grid->addWidget(ignore_label, 5, 0, AlignLeft | AlignVCenter);

            comment_edit = new KLineEdit(this);
            grid->addWidget(comment_edit, 6, 1);
            
            QLabel *comment_label = new QLabel
                (comment_edit, i18n("&Comment:"), this);
            grid->addWidget(comment_label, 6, 0, AlignLeft | AlignVCenter);
            
            binary_box = new QCheckBox(i18n("Import as &binaries"), this);
            grid->addMultiCellWidget(binary_box, 7, 7, 0, 1);
        }

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    frame->setMinimumHeight(frame->sizeHint().height());
    layout->addWidget(frame, 0);

    KButtonBox *buttonbox = new KButtonBox(this);
    QPushButton *helpbutton = buttonbox->addButton("&Help");
    helpbutton->setAutoDefault(false);
    buttonbox->addStretch();
    QPushButton *okbutton = buttonbox->addButton(i18n("OK"));
    QPushButton *cancelbutton = buttonbox->addButton(i18n("Cancel"));
    okbutton->setDefault(true);
    buttonbox->layout();
    buttonbox->setFixedHeight(buttonbox->height());
    layout->addWidget(buttonbox, 0);

    connect( helpbutton, SIGNAL(clicked()), SLOT(helpClicked()) );
    connect( okbutton, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancelbutton, SIGNAL(clicked()), this, SLOT(reject()) );

    layout->activate();
    resize(sizeHint());

    QStringList list1 = Repositories::readCvsPassFile();
    QStringList::ConstIterator it1;
    for (it1 = list1.begin(); it1 != list1.end(); ++it1)
        repo_combo->insertItem(*it1);
    
    QStringList list2 = Repositories::readConfigFile();
    QStringList::ConstIterator it2;
    for (it2 = list2.begin(); it2 != list2.end(); ++it2)
        if (!list1.contains(*it2))
            repo_combo->insertItem(*it2);
    
    act = action;
    
    if (options)
        {
            repo_combo->setEditText(options->repo);
            workdir_edit->setText(options->workdir);
            if (action == Import)
                {
                    module_edit->setText(options->module);
                    vendortag_edit->setText(options->vendortag);
                    releasetag_edit->setText(options->releasetag);
                    ignore_edit->setText(options->ignorefiles);
                    binary_box->setChecked(options->binary);
                }
            else
                {
                    module_combo->setEditText(options->module);
                    branch_edit->setText(options->branch);
                }
        }
    
}


void CheckoutDialog::done(int r)
{
    if (r == Accepted)
        {
            QFileInfo fi(workingDirectory());
            if (!fi.exists() || !fi.isDir())
                {
                    KMessageBox::information(this, i18n("Please choose an existing working directory."));
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
                    if (!isValidTag(vendorTag()) || !isValidTag(releaseTag()))
                        {
                            KMessageBox::information(this,  
                                                     i18n("Tags must start with a letter and may contain\n"
                                                          "letters, digits and the characters '-' and '_'."));
                            return;
                        }
                }

            if (!options)
                options = new Options;
            options->repo = repository();
            options->module = module();
            options->workdir = workingDirectory();
            if (act == Import)
                {
                    options->vendortag = vendorTag();
                    options->releasetag = releaseTag();
                    options->ignorefiles = ignoreFiles();
                    options->binary = importBinary();
                }
            else
                {
                    options->branch = branch();
                }
        }

    QDialog::done(r);
}


void CheckoutDialog::loadOptions(KConfig *config)
{
    if (!config->readEntry("Customized"))
        return;

    options = new Options;
    options->repo = config->readEntry("Repository");
    options->module = config->readEntry("Module");
    options->workdir = config->readEntry("Working directory");
    options->vendortag = config->readEntry("Vendor tag");
    options->releasetag = config->readEntry("Release tag");
    options->ignorefiles = config->readEntry("Ignore files");
    options->binary = config->readBoolEntry("Import binary");
}


void CheckoutDialog::saveOptions(KConfig *config)
{
    if (!options)
        return;
    
    config->writeEntry("Customized", true);
    config->writeEntry("Repository", options->repo);
    config->writeEntry("Module", options->module);
    config->writeEntry("Working directory", options->workdir);
    config->writeEntry("Vendor tag", options->vendortag);
    config->writeEntry("Release tag", options->releasetag);
    config->writeEntry("Ignore files", options->ignorefiles);
    config->writeEntry("Import binary", options->binary);
}


void CheckoutDialog::dirButtonClicked()
{
    QString dir = KFileDialog::getExistingDirectory(workdir_edit->text());
    if (!dir.isEmpty())
        workdir_edit->setText(dir);
}


void CheckoutDialog::moduleButtonClicked()
{
    QString cmdline = cvsClient(repository());
    cmdline += " -d ";
    cmdline += repository();
    cmdline += " checkout -c";

    CvsProgressDialog l("Checkout", this);
    l.setCaption(i18n("CVS Checkout"));
    if (!l.execCommand("", repository(), cmdline, "checkout"))
        return;

    module_combo->clear();
    QString str;
    while (l.getOneLine(&str))
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


void CheckoutDialog::helpClicked()
{
    QString anchor = (act==Import)? "importing" : "checkingout";
    kapp->invokeHelp(anchor, "cervisia");
}

#include "checkoutdlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
