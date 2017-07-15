/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2002-2008 Christian Loose <christian.loose@kdemail.net>
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

#include "commitdialog.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QTextStream>
#include <QVBoxLayout>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QPushButton>

#include <KConfig>
#include <KConfigGroup>
#include <KGuiItem>
#include <KLocalizedString>
#include <KHelpClient>

#include "cvsserviceinterface.h"
#include "logmessageedit.h"
#include "diffdialog.h"


class CommitListItem : public QListWidgetItem
{
public:
    CommitListItem(const QString& text, const QString& fileName, QListWidget* parent = 0)
        : QListWidgetItem(text, parent)
        , m_fileName(fileName)
    {
    }

    QString fileName() const { return m_fileName; }

private:
    QString m_fileName;
};


CommitDialog::CommitDialog(KConfig& cfg, OrgKdeCervisia5CvsserviceCvsserviceInterface* service,
                           QWidget *parent)
    : QDialog(parent)
    , partConfig(cfg)
    , cvsService(service)
{
    setWindowTitle(i18n("CVS Commit"));
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);

    user1Button = new QPushButton;
    buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttonBox, &QDialogButtonBox::helpRequested, this, &CommitDialog::slotHelp);

    KGuiItem::assign(user1Button, KGuiItem(i18n("&Diff")));

    QLabel *textlabel = new QLabel( i18n("Commit the following &files:"));
    mainLayout->addWidget(textlabel);

    m_fileList = new QListWidget;
    m_fileList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    textlabel->setBuddy(m_fileList);
    mainLayout->addWidget(m_fileList);
    connect( m_fileList, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
             this, SLOT(fileSelected(QListWidgetItem*)));
    connect( m_fileList, SIGNAL(itemSelectionChanged()),
             this, SLOT(fileHighlighted()) );

    QLabel *archivelabel = new QLabel(i18n("Older &messages:"));
    mainLayout->addWidget(archivelabel);

    combo = new QComboBox;
    mainLayout->addWidget(combo);
    archivelabel->setBuddy(combo);
    connect( combo, SIGNAL(activated(int)), this, SLOT(comboActivated(int)) );
    // make sure that combobox is smaller than the screen
    combo->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed));

    QLabel *messagelabel = new QLabel(i18n("&Log message:"));
    mainLayout->addWidget(messagelabel);

    edit = new Cervisia::LogMessageEdit(this);
    messagelabel->setBuddy(edit);
    edit->setFocus();
    edit->setMinimumSize(400, 100);
    mainLayout->addWidget(edit, 10);

    m_useTemplateChk = new QCheckBox(i18n("Use log message &template"));
    mainLayout->addWidget(m_useTemplateChk);
    connect( m_useTemplateChk, SIGNAL(clicked()), this, SLOT(useTemplateClicked()) );

    mainLayout->addWidget(buttonBox);
    okButton->setDefault(true);

    checkForTemplateFile();

    user1Button->setEnabled(false);
    connect(user1Button, SIGNAL(clicked()), this, SLOT(diffClicked()) );

    KConfigGroup cg(&partConfig, "CommitDialog");
    restoreGeometry(cg.readEntry<QByteArray>("geometry", QByteArray()));
}


CommitDialog::~CommitDialog()
{
    KConfigGroup cg(&partConfig, "CommitDialog");
    cg.writeEntry("geometry", saveGeometry());
    cg.writeEntry("UseTemplate", m_useTemplateChk->isChecked());
}

void CommitDialog::slotHelp()
{
  KHelpClient::invokeHelp(QLatin1String("committingfiles"));
}


void CommitDialog::setFileList(const QStringList &list)
{
    QString currentDirName = QFileInfo(QLatin1String(".")).absoluteFilePath();

    QStringList::ConstIterator it = list.begin();
    for( ; it != list.end(); ++it )
    {
        // the dot for the root directory is hard to see, so
        // we convert it to the absolut path
        QString text = (*it != QLatin1String(".") ? *it : currentDirName);

        edit->compObj()->addItem(text);

        CommitListItem* item = new CommitListItem(text, *it, m_fileList);
        item->setCheckState(Qt::Checked);
    }
}


QStringList CommitDialog::fileList() const
{
    QStringList files;

    for( int i = 0; i < m_fileList->count(); ++i )
    {
        CommitListItem* item = static_cast<CommitListItem*>(m_fileList->item(i));
        if( item->checkState() & Qt::Checked )
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
    return edit->toPlainText();
}


void CommitDialog::setLogHistory(const QStringList &list)
{
    commits = list;

    combo->addItem(i18n("Current"));

    for ( QStringList::ConstIterator it = list.begin();
          it != list.end(); ++it )
        {
            if( (*it).isEmpty() )
                continue;

            QString txt = *it;
            int index = txt.indexOf('\n', 0);
            if ( index != -1 ) // Fetch first line
            {
                txt = txt.mid(0, index);
                txt += "...";
            }

            combo->addItem(txt);
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
            current_text = edit->toPlainText();

        // Show archived text
        edit->setText(commits[index-1]);
    }
    current_index = index;
}


void CommitDialog::fileSelected(QListWidgetItem* item)
{
    showDiffDialog(item->text());
}


void CommitDialog::fileHighlighted()
{
    bool isItemSelected = !m_fileList->selectedItems().isEmpty();
    user1Button->setEnabled(isItemSelected);
}


void CommitDialog::diffClicked()
{
    QListWidgetItem* item = m_fileList->selectedItems().first();
    if( !item )
        return;

    showDiffDialog(item->text());
}


void CommitDialog::showDiffDialog(const QString& fileName)
{
    DiffDialog *l = new DiffDialog(partConfig, this, "diffdialog");

    // disable diff button so user doesn't open the same diff several times (#83018)
    user1Button->setEnabled(false);

    if (l->parseCvsDiff(cvsService, fileName, "", ""))
        l->show();
    else
        delete l;

    // re-enable diff button
    user1Button->setEnabled(true);
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
    QString filename = QDir::current().absolutePath() + "/CVS/Template";
    if( QFile::exists(filename) )
    {
        QFile f(filename);
        if( f.open(QIODevice::ReadOnly) )
        {
            QTextStream stream(&f);
            m_templateText = stream.readAll();
            f.close();

            m_useTemplateChk->setEnabled(true);
            KConfigGroup cs(&partConfig, "CommitDialog");
            bool check = cs.readEntry("UseTemplate", true);
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
    edit->textCursor().movePosition(QTextCursor::Start);
    edit->ensureCursorVisible();
}


void CommitDialog::removeTemplateText()
{
    edit->setText(edit->toPlainText().remove(m_templateText));
}


// Local Variables:
// c-basic-offset: 4
// End:
