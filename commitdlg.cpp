/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2002-2005 Christian Loose <christian.loose@kdemail.net>
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

#include "commitdlg.h"

#include <qcombobox.h>
#include <qcheckbox.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qheader.h>
#include <klistview.h>
#include <kconfig.h>
#include <klocale.h>

#include "cvsservice_stub.h"
#include "logmessageedit.h"
#include "diffdlg.h"


class CommitListItem : public QCheckListItem
{
public:
    CommitListItem(QListView* parent, const QString& text, const QString fileName)
        : QCheckListItem(parent, text, QCheckListItem::CheckBox)
        , m_fileName(fileName)
    {
    }

    QString fileName() const { return m_fileName; }

private:
    QString m_fileName;
};


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

    m_fileList = new KListView(mainWidget);
    m_fileList->addColumn("");
    m_fileList->setFullWidth(true);
    m_fileList->header()->hide();
    textlabel->setBuddy(m_fileList);
    connect( m_fileList, SIGNAL(doubleClicked(QListViewItem*)),
             this, SLOT(fileSelected(QListViewItem*)));
    connect( m_fileList, SIGNAL(selectionChanged()),
             this, SLOT(fileHighlighted()) );
    layout->addWidget(m_fileList, 5);

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

    edit = new Cervisia::LogMessageEdit(mainWidget);
    messagelabel->setBuddy(edit);
    edit->setCheckSpellingEnabled(true);
    edit->setFocus();
    edit->setMinimumSize(400, 100);
    layout->addWidget(edit, 10);

    m_useTemplateChk = new QCheckBox(i18n("Use log message &template"), mainWidget);
    layout->addWidget(m_useTemplateChk);
    connect( m_useTemplateChk, SIGNAL(clicked()), this, SLOT(useTemplateClicked()) );

    checkForTemplateFile();

    setButtonGuiItem(User1, KGuiItem(i18n("&Diff"), "vcs_diff"));
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

    KConfigGroupSaver cs(&partConfig, "CommitDialog");
    partConfig.writeEntry("UseTemplate", m_useTemplateChk->isChecked());
}


void CommitDialog::setFileList(const QStringList &list)
{
    QString currentDirName = QFileInfo(QChar('.')).absFilePath();

    QStringList::ConstIterator it = list.begin();
    for( ; it != list.end(); ++it )
    {
        // the dot for the root directory is hard to see, so
        // we convert it to the absolut path
        QString text = (*it != "." ? *it : currentDirName);

        edit->compObj()->addItem(text);
        CommitListItem* item = new CommitListItem(m_fileList, text, *it);
        item->setOn(true);
    }
}


QStringList CommitDialog::fileList() const
{
    QStringList files;

    QListViewItemIterator it(m_fileList, QListViewItemIterator::Checked);
    for( ; it.current(); ++it )
    {
        CommitListItem* item = static_cast<CommitListItem*>(it.current());
        files.append(item->fileName());
    }

    return files;
}


void CommitDialog::setLogMessage(const QString &msg)
{
    edit->setText(msg);

    if( m_useTemplateChk->isChecked() )
        addTemplateText();
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


void CommitDialog::fileSelected(QListViewItem* item)
{
    // double click on empty space?
    if( !item )
        return;

    showDiffDialog(item->text(0));
}


void CommitDialog::fileHighlighted()
{
    bool isItemSelected = (m_fileList->selectedItem() != 0);
    enableButton(User1, isItemSelected);
}


void CommitDialog::diffClicked()
{
    QListViewItem* item = m_fileList->selectedItem();
    if( !item )
        return;

    showDiffDialog(item->text(0));
}


void CommitDialog::showDiffDialog(const QString& fileName)
{
    DiffDialog *l = new DiffDialog(partConfig, this, "diffdialog");

    // disable diff button so user doesn't open the same diff several times (#83018)
    enableButton(User1, false);

    if (l->parseCvsDiff(cvsService, fileName, "", ""))
        l->show();
    else
        delete l;

    // re-enable diff button
    enableButton(User1, true);
}


void CommitDialog::useTemplateClicked()
{
    if( m_useTemplateChk->isChecked() )
    {
        addTemplateText();
    }
    else
    {
        removeTemplateText();
    }
}


void CommitDialog::checkForTemplateFile()
{
    QString filename = QDir::current().absPath() + "/CVS/Template";
    if( QFile::exists(filename) )
    {
        QFile f(filename);
        if( f.open(IO_ReadOnly) )
        {
            QTextStream stream(&f);
            m_templateText = stream.read();
            f.close();

            m_useTemplateChk->setEnabled(true);
            KConfigGroupSaver cs(&partConfig, "CommitDialog");
            bool check = partConfig.readBoolEntry("UseTemplate", true);
            m_useTemplateChk->setChecked(check);

            addTemplateText();
        }
        else
        {
            m_useTemplateChk->setEnabled(false);
        }
    }
    else
    {
        m_useTemplateChk->setEnabled(false);
    }
}


void CommitDialog::addTemplateText()
{
    edit->append(m_templateText);
    edit->moveCursor(QTextEdit::MoveHome, false);
    edit->ensureCursorVisible();
}


void CommitDialog::removeTemplateText()
{
    edit->setText(edit->text().remove(m_templateText));
}


#include "commitdlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
