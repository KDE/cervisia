/*
 *  Copyright (c) 2004 Christian Loose <christian.loose@kdemail.net>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "cvsinitdlg.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <kfiledialog.h>
#include <klineedit.h>
#include <klocale.h>

using Cervisia::CvsInitDialog;


CvsInitDialog::CvsInitDialog(QWidget* parent, const char* name)
    : KDialogBase(parent, name, true, i18n("Create a New Repository (cvs init)"),
                  Ok | Cancel, Ok, true)
{
    QFrame* mainWidget = makeMainWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(mainWidget, 0, spacingHint());

    QLabel* dirLabel = new QLabel(i18n("Repository folder:"), mainWidget);
    mainLayout->addWidget(dirLabel);

    QHBoxLayout* dirLayout = new QHBoxLayout(mainLayout);
     
    m_directoryEdit = new KLineEdit(mainWidget);
    m_directoryEdit->setFocus();
    dirLabel->setBuddy(m_directoryEdit);
    dirLayout->addWidget(m_directoryEdit); 
    
    QPushButton* dirButton = new QPushButton("...", mainWidget);
    dirButton->setFixedWidth(30);
    dirLayout->addWidget(dirButton);
    
    connect( dirButton, SIGNAL(clicked()),
             this,      SLOT(dirButtonClicked()) );
    connect( m_directoryEdit, SIGNAL(textChanged(const QString&)),
             this,            SLOT(lineEditTextChanged(const QString&)));
             
    enableButton(Ok, false);

    setMinimumWidth(350);
}


QString CvsInitDialog::directory() const
{
    return m_directoryEdit->text();
}


void CvsInitDialog::dirButtonClicked()
{
    QString dir = KFileDialog::getExistingDirectory(m_directoryEdit->text());
    if( !dir.isEmpty() )
        m_directoryEdit->setText(dir);
}


void CvsInitDialog::lineEditTextChanged(const QString& text)
{
    enableButton(Ok, !text.stripWhiteSpace().isEmpty());
}
    
#include "cvsinitdlg.moc"
