/* 
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "commitdlg.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <kapplication.h>
#include <kbuttonbox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kconfig.h>
#include <kdebug.h>
#include "diffdlg.h"
#include "misc.h"


CommitDialog::Options *CommitDialog::options = 0;


CommitDialog::CommitDialog(ActionType action, QWidget *parent, const char *name)
    : QDialog(parent, name, true), edit(0)
{
    setCaption( (action==Add)?       i18n("CVS Add") :
                (action==AddBinary)? i18n("CVS Add Binary") :
                (action==Remove)?    i18n("CVS Remove") :
                                     i18n("CVS Commit") );

    QBoxLayout *layout = new QVBoxLayout(this, 10);
    
    QLabel *textlabel = new QLabel
        ( (action==Add)?       i18n("Add the following files to the repository:") :
          (action==AddBinary)? i18n("Add the following binary files to the repository:") :
          (action==Remove)?    i18n("Remove the following files from the repository:") :
                               i18n("Commit the following &files:"),
          this );
    layout->addWidget(textlabel, 0);

    listbox = new QListBox(this);
    textlabel->setBuddy(listbox);
    connect( listbox, SIGNAL(selected(int)), this, SLOT(fileSelected(int)));
    layout->addWidget(listbox, 5);

    if (action == Commit)
        {
            QLabel *archivelabel = new QLabel(i18n("Older &messages:"), this);
            layout->addWidget(archivelabel, 0);
            
            combo = new QComboBox(this);
            archivelabel->setBuddy(combo);
            connect( combo, SIGNAL(activated(int)), this, SLOT(comboActivated(int)) );
            combo->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
            layout->addWidget(combo, 0);
            
            QLabel *messagelabel = new QLabel(i18n("&Log message:"), this);
            layout->addWidget(messagelabel, 0);

            edit = new QMultiLineEdit(this);
            messagelabel->setBuddy(edit);
            edit->setFocus();
            edit->setMinimumSize(400, 100);
            layout->addWidget(edit, 10);
        }
    else
        {
            listbox->setSelectionMode(QListBox::NoSelection);
            listbox->setFocusPolicy(QWidget::NoFocus);
        }

    if (action == Remove)    
        {
            QBoxLayout *warningLayout = new QHBoxLayout;

            QLabel *warningIcon = new QLabel(this);
            KIconLoader *loader = kapp->iconLoader();
            warningIcon->setPixmap(loader->loadIcon("messagebox_warning", KIcon::NoGroup,
                                                    KIcon::SizeMedium, KIcon::DefaultState,
                                                    0, true));
            warningLayout->addWidget(warningIcon);

            QLabel *warningText = new QLabel(i18n("This will also remove the files from "
                                                  "your local working copy!"), this);
            warningLayout->addWidget(warningText);
    
            layout->addSpacing(5);
            layout->addLayout(warningLayout);
            layout->addSpacing(5);
        }
        
    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(frame, 0);

    KButtonBox *buttonbox = new KButtonBox(this);
    buttonbox->addStretch();
    QPushButton *ok = buttonbox->addButton(i18n("&OK"));
    QPushButton *cancel = buttonbox->addButton(i18n("Cancel"));
    ok->setDefault(true);
    connect( ok, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancel, SIGNAL(clicked()), this, SLOT(reject()) );
    buttonbox->layout();
    layout->addWidget(buttonbox, 0);

    layout->activate();

    if (options && edit) // Only for commits
        resize(options->size);
}


void CommitDialog::done(int res)
{
    if (edit) // Only for commits
        {
            if (!options)
                options = new Options;
            options->size = size();
        }
    
    QDialog::done(res);
}


void CommitDialog::loadOptions(KConfig *config)
{
    if (!config->readEntry("Customized"))
        return;

    options = new Options;
    options->size = config->readSizeEntry("Size");
}


void CommitDialog::saveOptions(KConfig *config)
{
    if (!options)
        return;

    config->writeEntry("Customized", true);
    config->writeEntry("Size", options->size);
}


void CommitDialog::setLogHistory(const QString &sbox, const QString &repo,
                                 const QStringList &list)
{
    sandbox = sbox;
    repository = repo;
    commits = list;

    combo->insertItem(i18n("Current"));

    for ( QStringList::ConstIterator it = list.begin();
          it != list.end(); ++it )
        {
            if( (*it).isEmpty() )
                continue;

            QString txt = *it;
            int index = txt.find('\n', 0);
            if ( index != -1 ) // Fetch first line
            {
                txt = txt.mid(0, index);
                txt += "...";
            }

            combo->insertItem(txt);
        }
}


void CommitDialog::comboActivated(int index)
{
    if (index == current_index)
        return;
    
    if (index == 0) // Handle current text
        edit->setText(current_text);
    else
        {
            if (current_index == 0) // Store current text
                current_text = edit->text();
            
            // Show archived text
            edit->setText(commits[index-1]);
        }
    current_index = index;
}


void CommitDialog::fileSelected(int index)
{
    QListBoxItem *item = listbox->item(index);
    if ( !item )
        return;
    QString filename = item->text();

    DiffDialog *l = new DiffDialog(this, "diffdialog", true);
    if (l->parseCvsDiff(sandbox, repository, filename, "", ""))
        l->show();
    else
        delete l;
}

#include "commitdlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
