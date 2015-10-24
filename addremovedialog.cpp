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

// KDE
#include <KListWidget>
#include <KLocale>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>


AddRemoveDialog::AddRemoveDialog(ActionType action, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle( (action==Add)?       i18n("CVS Add") :
                (action==AddBinary)? i18n("CVS Add Binary") :
                                     i18n("CVS Remove") );
    setModal(true);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Help);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    //PORTING SCRIPT: WARNING mainLayout->addWidget(buttonBox) must be last item in layout. Please move it.
    mainLayout->addWidget(buttonBox);
    okButton->setDefault(true);
    // Also give the focus to the OK button, otherwise the Help button gets focus
    // and is activated by Key_Return
    okButton->setFocus();

    QFrame* mainWidget = new QFrame(this);
    mainLayout->addWidget(mainWidget);

    QBoxLayout *layout = new QVBoxLayout(mainWidget);
    mainLayout->addWidget(layout);
    layout->setSpacing(spacingHint());
    layout->setMargin(0);

    QLabel *textlabel = new QLabel
        ( (action==Add)?       i18n("Add the following files to the repository:") :
          (action==AddBinary)? i18n("Add the following binary files to the repository:") :
                               i18n("Remove the following files from the repository:") ,
          mainWidget );
    layout->addWidget(textlabel);

    m_listBox = new KListWidget(mainWidget);
    mainLayout->addWidget(m_listBox);
    m_listBox->setSelectionMode(QAbstractItemView::NoSelection);
    layout->addWidget(m_listBox, 5);

    // Add warning message to dialog when user wants to remove a file
    if (action==Remove)
    {
        QBoxLayout *warningLayout = new QHBoxLayout;

        QLabel *warningIcon = new QLabel(mainWidget);
        mainLayout->addWidget(warningIcon);
        warningIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(32));
        warningLayout->addWidget(warningIcon);

        QLabel *warningText = new QLabel(i18n("This will also remove the files from "
                                              "your local working copy."), mainWidget);
        warningLayout->addWidget(warningText);

        layout->addSpacing(5);
        layout->addLayout(warningLayout);
        layout->addSpacing(5);
    }

    if( action == Remove )
        setHelp("removingfiles");
    else
        setHelp("addingfiles");
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
