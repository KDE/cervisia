/*
 *  Copyright (c) 2004 Christian Loose <christian.loose@kdemail.net>
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


#include "cvsinitdialog.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QLineEdit>
#include <klocale.h>
#include <kurlcompletion.h>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>


using Cervisia::CvsInitDialog;


CvsInitDialog::CvsInitDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Create New Repository (cvs init)"));
    setModal(true);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
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

    QFrame* mainWidget = new QFrame(this);
    mainLayout->addWidget(mainWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->addWidget(mainLayout);
    mainLayout->setSpacing(spacingHint());
    mainLayout->setMargin(0);

    QLabel* dirLabel = new QLabel(i18n("Repository folder:"), mainWidget);
    mainLayout->addWidget(dirLabel);
    mainLayout->addWidget(dirLabel);

    QHBoxLayout* dirLayout = new QHBoxLayout();
    mainLayout->addLayout(dirLayout);
     
    m_directoryEdit = new QLineEdit(mainWidget);
    mainLayout->addWidget(m_directoryEdit);
    m_directoryEdit->setFocus();
        
    KUrlCompletion* comp = new KUrlCompletion();
    m_directoryEdit->setCompletionObject(comp);
    m_directoryEdit->setAutoDeleteCompletionObject(true);

    dirLabel->setBuddy(m_directoryEdit);
    dirLayout->addWidget(m_directoryEdit); 
    
    QPushButton* dirButton = new QPushButton("...", mainWidget);
    mainLayout->addWidget(dirButton);
    dirButton->setFixedWidth(30);
    dirLayout->addWidget(dirButton);
    
    connect( dirButton, SIGNAL(clicked()),
             this,      SLOT(dirButtonClicked()) );
    connect( m_directoryEdit, SIGNAL(textChanged(QString)),
             this,            SLOT(lineEditTextChanged(QString)));
             
    okButton->setEnabled(false);

    setMinimumWidth(350);
}


QString CvsInitDialog::directory() const
{
    return m_directoryEdit->text();
}


void CvsInitDialog::dirButtonClicked()
{
    QString dir = QFileDialog::getExistingDirectory(0, QString(), m_directoryEdit->text());
    if( !dir.isEmpty() )
        m_directoryEdit->setText(dir);
}


void CvsInitDialog::lineEditTextChanged(const QString& text)
{
    okButton->setEnabled(!text.trimmed(.isEmpty();
}
    
