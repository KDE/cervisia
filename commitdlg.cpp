/* 
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2002-2003 Christian Loose <christian.loose@hamburg.de>
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

#include <qcombobox.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <ktextedit.h>
#include <kconfig.h>
#include <klocale.h>

#include "cvsservice_stub.h"
#include "diffdlg.h"


CommitDialog::CommitDialog(KConfig& cfg, CvsService_stub* service, 
                           QWidget *parent, const char *name)
    : KDialogBase(parent, name, true, i18n("CVS Commit"),
                  Ok | Cancel | Help | User1, Ok, true)
    , partConfig(cfg)
    , cvsService(service)
{
    QFrame* mainWidget = makeMainWidget();

    QBoxLayout *layout = new QVBoxLayout(mainWidget, 0, spacingHint());

    QLabel *textlabel = new QLabel( i18n("Commit the following &files:"), mainWidget );
    layout->addWidget(textlabel);

    listbox = new QListBox(mainWidget);
    textlabel->setBuddy(listbox);
    connect( listbox, SIGNAL(selected(int)), this, SLOT(fileSelected(int)));
    connect( listbox, SIGNAL(highlighted(int)), this, SLOT(fileHighlighted(int)));
    layout->addWidget(listbox, 5);

    QLabel *archivelabel = new QLabel(i18n("Older &messages:"), mainWidget);
    layout->addWidget(archivelabel);
            
    combo = new QComboBox(mainWidget);
    archivelabel->setBuddy(combo);
    connect( combo, SIGNAL(activated(int)), this, SLOT(comboActivated(int)) );
    // make sure that combobox is smaller than the screen
    combo->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    layout->addWidget(combo);
            
    QLabel *messagelabel = new QLabel(i18n("&Log message:"), mainWidget);
    layout->addWidget(messagelabel);

    edit = new KTextEdit(mainWidget);
    messagelabel->setBuddy(edit);
    edit->setCheckSpellingEnabled(true);
    edit->setFocus();
    edit->setMinimumSize(400, 100);
    layout->addWidget(edit, 10);

    setButtonText(User1, i18n("&Diff"));
    enableButton(User1, false);
    connect( this, SIGNAL(user1Clicked()),
             this, SLOT(diffClicked()) );            

    setHelp("commitingfiles");

    QSize size = configDialogSize(partConfig, "CommitDialog");
    resize(size);
}


CommitDialog::~CommitDialog()
{
    saveDialogSize(partConfig, "CommitDialog");
}


void CommitDialog::setFileList(const QStringList &list)
{
    listbox->insertStringList(list);

    // the dot for the root directory is hard to see, so
    // we convert it to the absolut path
    if (const QListBoxItem* item = listbox->findItem(QChar('.'), Qt::ExactMatch))
    {
        listbox->changeItem(QFileInfo(QChar('.')).absFilePath(),
                            listbox->index(item));
    }
}


void CommitDialog::setLogMessage(const QString &msg)
{
    edit->setText(msg);
}


QString CommitDialog::logMessage() const
{
    return edit->text();
}


void CommitDialog::setLogHistory(const QStringList &list)
{
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

    showDiffDialog(item->text());
}


void CommitDialog::fileHighlighted(int index)
{
    highlightedFile = index;
    enableButton(User1, true);
}


void CommitDialog::diffClicked()
{
    QListBoxItem *item = listbox->item(highlightedFile);
    if ( !item )
        return;

    showDiffDialog(item->text());
}


void CommitDialog::showDiffDialog(const QString& fileName)
{
    DiffDialog *l = new DiffDialog(partConfig, this, "diffdialog");
    if (l->parseCvsDiff(cvsService, fileName, "", ""))
        l->show();
    else
        delete l;
}


#include "commitdlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
