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


#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qmultilinedit.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qdir.h>
#include <kapp.h>
#include <kprocess.h>
#include <kbuttonbox.h>
#include <kfiledialog.h>
#include <klocale.h>

#include "cvsprogressdlg.h"
#include "repositories.h"
#include "misc.h"

#include "checkoutdlg.h"
#include "checkoutdlg.moc"


CheckoutDialog::Options *CheckoutDialog::options = 0;


CheckoutDialog::CheckoutDialog(ActionType action, QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption( (action==Checkout)? i18n("CVS Checkout") : i18n("CVS Import") );

    QBoxLayout *layout = new QVBoxLayout(this, 10);

    QGridLayout *grid = new QGridLayout((action==Checkout)? 3 : 7, 2, 10);
    layout->addLayout(grid);
    grid->setColStretch(0, 1);
    grid->setColStretch(1, 20);
    for (int i = 0; i < ((action==Checkout)? 3 : 7); ++i)
        grid->setRowStretch(i, 0);

    repo_combo = new QComboBox(true, this);
    repo_combo->adjustSize();
    repo_combo->setMinimumWidth(repo_combo->size().width());
    repo_combo->setFixedHeight(repo_combo->height());
    repo_combo->setFocus();
    grid->addWidget(repo_combo, 0, 1);
    
    QLabel *repo_label = new QLabel
	(repo_combo, i18n("&Repository:"), this);
    repo_label->setFixedSize(repo_label->sizeHint());
    grid->addWidget(repo_label, 0, 0, AlignLeft | AlignVCenter);

    if (action == Import)
        {
            module_edit = new KLineEdit(this);
            module_edit->setMinimumSize(module_edit->sizeHint());
            grid->addWidget(module_edit, 1, 1);
            QLabel *module_label = new QLabel(module_edit, i18n("&Module:"), this);
            module_label->setFixedSize(module_label->sizeHint());
            grid->addWidget(module_label, 1, 0, AlignLeft | AlignVCenter);
        }
    else
        {
            module_combo = new QComboBox(true, this);
            module_combo->setMinimumSize(module_combo->sizeHint());
            
            QPushButton *module_button = new QPushButton("Fetch &List", this);
            module_button->setMinimumWidth(module_button->sizeHint().width());
            connect( module_button, SIGNAL(clicked()),
                     this, SLOT(moduleButtonClicked()) );
            
            QBoxLayout *module_layout = new QHBoxLayout();
            grid->addLayout(module_layout, 1, 1);
            module_layout->addWidget(module_combo, 10);
            module_layout->addWidget(module_button, 0, AlignVCenter);
 
            QLabel *module_label = new QLabel(module_combo, i18n("&Module:"), this);
            module_label->setFixedSize(module_label->sizeHint());
            grid->addWidget(module_label, 1, 0, AlignLeft | AlignVCenter);
        }
    
    workdir_edit = new KLineEdit(this);
    workdir_edit->setText(QDir::homeDirPath());
    QFontMetrics fm(workdir_edit->fontMetrics());
    workdir_edit->setMinimumSize(fm.width("X")*30,
				 workdir_edit->sizeHint().height());

    QPushButton *dir_button = new QPushButton("...", this);
    connect( dir_button, SIGNAL(clicked()),
	     this, SLOT(dirButtonClicked()) );
    dir_button->setFixedHeight(workdir_edit->sizeHint().height());
    dir_button->setFixedWidth(30);

    QBoxLayout *workdir_layout = new QHBoxLayout();
    grid->addLayout(workdir_layout, 2, 1);
    workdir_layout->addWidget(workdir_edit, 10);
    workdir_layout->addWidget(dir_button, 0, AlignVCenter);

    QLabel *workdir_label = new QLabel
	(workdir_edit, i18n("Working &directory:"), this);
    workdir_label->setFixedSize(workdir_label->sizeHint());
    grid->addWidget(workdir_label, 2, 0, AlignLeft | AlignVCenter);

    if (action == Import)
        {
            vendortag_edit = new KLineEdit(this);
            vendortag_edit->setMinimumSize(vendortag_edit->sizeHint());
            grid->addWidget(vendortag_edit, 3, 1);
            
            QLabel *vendortag_label = new QLabel
                (vendortag_edit, i18n("&Vendor Tag:"), this);
            vendortag_label->setFixedSize(vendortag_label->sizeHint());
            grid->addWidget(vendortag_label, 3, 0, AlignLeft | AlignVCenter);

            releasetag_edit = new KLineEdit(this);
            releasetag_edit->setMinimumSize(releasetag_edit->sizeHint());
            grid->addWidget(releasetag_edit, 4, 1);
            
            QLabel *releasetag_label = new QLabel
                (releasetag_edit, i18n("&Release Tag:"), this);
            releasetag_label->setFixedSize(releasetag_label->sizeHint());
            grid->addWidget(releasetag_label, 4, 0, AlignLeft | AlignVCenter);

            ignore_edit = new KLineEdit(this);
            ignore_edit->setMinimumSize(ignore_edit->sizeHint());
            grid->addWidget(ignore_edit, 5, 1);
            
            QLabel *ignore_label = new QLabel
                (ignore_edit, i18n("&Ignore files:"), this);
            ignore_label->setFixedSize(ignore_label->sizeHint());
            grid->addWidget(ignore_label, 5, 0, AlignLeft | AlignVCenter);

            binary_box = new QCheckBox(i18n("Import as &binaries"), this);
            binary_box->setMinimumSize(binary_box->sizeHint());
            grid->addMultiCellWidget(binary_box, 6, 6, 0, 1);
            
        }

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

    QStrList list1;
    Repositories::readCvsPassFile(&list1);
    QStrListIterator it1(list1);
    for (; it1.current(); ++it1)
        repo_combo->insertItem(it1.current());
    
    QStrList list2;
    Repositories::readConfigFile(&list2);
    QStrListIterator it2(list2);
    for (; it2.current(); ++it2)
        if (!list1.contains(it2.current()))
            repo_combo->insertItem(it2.current());
    
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
                module_combo->setEditText(options->module);
        }
    
}


void CheckoutDialog::done(int r)
{
    if (r == Accepted)
        {
            QFileInfo fi(workingDirectory());
            if (!fi.exists() || !fi.isDir())
                {
                    QMessageBox::information(this, "Cervisia", i18n
                                             ("Please choose an existing working directory."));
                    return;
                }
            if (module().isEmpty())
                {
                    QMessageBox::information(this, "Cervisia", i18n
                                             ("Please specify a module name."));
                    return;
                }
                                                                    
            if (act==Import)
                {
                    if (vendorTag().isEmpty() || releaseTag().isEmpty())
                        {
                            QMessageBox::information(this, "Cervisia", i18n
                                                     ("Please specify a vendor tag and a release tag."));
                            return;
                        }
                    if (!isValidTag(vendorTag()) || !isValidTag(releaseTag()))
                        {
                            QMessageBox::information(this, "Cervisia", 
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
    QString path=KFileDialog::getExistingDirectory(workdir_edit->text());
    if(!path.isEmpty())
	workdir_edit->setText(path);    
}


void CheckoutDialog::moduleButtonClicked()
{
    QString cmdline = cvsClient( options->repo );
    cmdline += " -d ";
    cmdline += repository();
    cmdline += " checkout -c";

    CvsProgressDialog l("Checkout", this);
    l.setCaption(i18n("CVS Checkout"));
    if (!l.execCommand("", repository(), cmdline, "checkout"))
        return;

    module_combo->clear();
    QCString str;
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

// Local Variables:
// c-basic-offset: 4
// End:
