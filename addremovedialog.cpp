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

#include "addremovedialog.h"

// Qt
#include <QBoxLayout>
#include <QFileInfo>
#include <QLabel>
#include <QStringList>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QListWidget>

// KDE
#include <KLocalizedString>
#include <KConfigGroup>
#include <KHelpClient>
#include <KMessageWidget>


AddRemoveDialog::AddRemoveDialog(ActionType action, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle((action == Add) ?       i18n("CVS Add") :
                   (action == AddBinary) ? i18n("CVS Add Binary") :
                                           i18n("CVS Remove") );
    setModal(true);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttonBox, &QDialogButtonBox::helpRequested, this, &AddRemoveDialog::slotHelp);

    // Also give the focus to the OK button, otherwise the Help button gets focus
    // and is activated by Key_Return
    okButton->setFocus();

    QLabel *textlabel = new QLabel
        ((action == Add) ?       i18n("Add the following files to the repository:") :
         (action == AddBinary) ? i18n("Add the following binary files to the repository:") :
                                 i18n("Remove the following files from the repository:"));

    mainLayout->addWidget(textlabel);

    m_listBox = new QListWidget;
    m_listBox->setSelectionMode(QAbstractItemView::NoSelection);

    mainLayout->addWidget(m_listBox);

    // Add warning message to dialog when user wants to remove a file
    if ( action == Remove )
    {
        KMessageWidget *warning =
            new KMessageWidget(i18n("This will also remove the files from "
                                    "your local working copy."));

        warning->setIcon(QIcon::fromTheme("dialog-warning").pixmap(32));
        warning->setCloseButtonVisible(false);

        mainLayout->addSpacing(5);
        mainLayout->addWidget(warning);
        mainLayout->addSpacing(5);
    }

    if ( action == Remove )
        helpTopic = "removingfiles";
    else
        helpTopic = "addingfiles";

    mainLayout->addWidget(buttonBox);
    okButton->setDefault(true);
}

void AddRemoveDialog::slotHelp()
{
  KHelpClient::invokeHelp(helpTopic);
}


void AddRemoveDialog::setFileList(const QStringList& files)
{
    // the dot for the root directory is hard to see, so
    // we convert it to the absolut path
    if( files.contains(".") )
    {
        QStringList copy(files);
        int idx = copy.indexOf(".");
        copy[idx] = QFileInfo(".").absoluteFilePath();

        m_listBox->addItems(copy);
    }
    else
        m_listBox->addItems(files);
}


// kate: space-indent on; indent-width 4; replace-tabs on;
