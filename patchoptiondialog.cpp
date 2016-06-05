/*
 *  Copyright (C) 2004 Christian Loose <christian.loose@kdemail.net>
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

#include "patchoptiondialog.h"

using Cervisia::PatchOptionDialog;

#include <KHelpClient>
#include <KConfigGroup>
#include <KLocalizedString>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qgroupbox.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QPushButton>


PatchOptionDialog::PatchOptionDialog(QWidget* parent)
    : QDialog(parent)
{
    setModal(false);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttonBox, &QDialogButtonBox::helpRequested, this, &PatchOptionDialog::slotHelp);

    { // format
      m_formatBtnGroup = new QButtonGroup(this);

      connect(m_formatBtnGroup, SIGNAL(buttonClicked(int)),
              this,             SLOT(formatChanged(int)));

      m_formatBtnGroup->addButton(new QRadioButton(i18n("Context")), 0);
      m_formatBtnGroup->addButton(new QRadioButton(i18n("Normal")), 1);
      QRadioButton* unifiedFormatBtn = new QRadioButton(i18n("Unified"));
      unifiedFormatBtn->setChecked(true);
      m_formatBtnGroup->addButton(unifiedFormatBtn, 2);

      QGroupBox *box = new QGroupBox(i18n("Output Format"));
      mainLayout->addWidget(box);
      QVBoxLayout *v = new QVBoxLayout(box);
      v->addWidget(m_formatBtnGroup->button(0));
      v->addWidget(m_formatBtnGroup->button(1));
      v->addWidget(m_formatBtnGroup->button(2));

      mainLayout->addWidget(box);
    }

    QLabel* contextLinesLbl = new QLabel(i18n("&Number of context lines:"));
    m_contextLines = new QSpinBox;
    m_contextLines->setValue(3);
    mainLayout->addWidget(m_contextLines);
    m_contextLines->setRange(2, 65535);
    contextLinesLbl->setBuddy(m_contextLines);

    QBoxLayout* contextLinesLayout = new QHBoxLayout();
    mainLayout->addLayout(contextLinesLayout);
    contextLinesLayout->addWidget(contextLinesLbl);
    contextLinesLayout->addWidget(m_contextLines);

    { // ignore options
      QButtonGroup *group = new QButtonGroup(this);
      group->setExclusive(false);

      m_blankLineChk   = new QCheckBox(i18n("Ignore added or removed empty lines"));
      m_spaceChangeChk = new QCheckBox(i18n("Ignore changes in the amount of whitespace"));
      m_allSpaceChk    = new QCheckBox(i18n("Ignore all whitespace"));
      m_caseChangesChk = new QCheckBox(i18n("Ignore changes in case"));

      group->addButton(m_blankLineChk);
      group->addButton(m_spaceChangeChk);
      group->addButton(m_allSpaceChk);
      group->addButton(m_caseChangesChk);

      QGroupBox *box = new QGroupBox(i18n("Ignore Options"));
      mainLayout->addWidget(box);
      QVBoxLayout *v = new QVBoxLayout(box);
      v->addWidget(m_blankLineChk);
      v->addWidget(m_spaceChangeChk);
      v->addWidget(m_allSpaceChk);
      v->addWidget(m_caseChangesChk);

      mainLayout->addWidget(box);
    }

    mainLayout->addWidget(buttonBox);
}


PatchOptionDialog::~PatchOptionDialog()
{
}

void PatchOptionDialog::slotHelp()
{
  KHelpClient::invokeHelp(QLatin1String("creatingpatches"));
}


QString PatchOptionDialog::diffOptions() const
{
    QString options;

    if( m_blankLineChk->isChecked() )
        options += " -B ";

    if( m_spaceChangeChk->isChecked() )
        options += " -b ";

    if( m_allSpaceChk->isChecked() )
        options += " -w ";

    if( m_caseChangesChk->isChecked() )
        options += " -i ";

    return options;
}


QString PatchOptionDialog::formatOption() const
{
    switch( m_formatBtnGroup->checkedId() )
    {
        case 0: return "-C " + QString::number(m_contextLines->value());
        case 1: return "";
        case 2: return "-U " + QString::number(m_contextLines->value());
    }

    return "";
}


void PatchOptionDialog::formatChanged(int buttonId)
{
    bool enabled = ( buttonId == 0 || buttonId == 2 );
    m_contextLines->setEnabled(enabled);
}

