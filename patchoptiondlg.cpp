/*
 *  Copyright (C) 2004 Christian Loose <christian.loose@kdemail.net>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "patchoptiondlg.h"
using Cervisia::PatchOptionDialog;

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>
#include <knuminput.h>
#include <klocale.h>


PatchOptionDialog::PatchOptionDialog(QWidget* parent, const char* name)
    : KDialogBase(parent, name, true/*modal*/, QString::null,
                  Ok | Cancel | Help, Ok, true/*separator*/)
{
    QFrame* mainWidget = makeMainWidget();
    QBoxLayout* topLayout = new QVBoxLayout(mainWidget, 0, spacingHint());
    
    m_formatBtnGroup = new QVButtonGroup("Output Format", mainWidget, "");
    topLayout->addWidget(m_formatBtnGroup);
 
    connect(m_formatBtnGroup, SIGNAL(clicked(int)),
            this,             SLOT(formatChanged(int)));   
            
    QRadioButton* contextFormatBtn = new QRadioButton("Context", m_formatBtnGroup);
    QRadioButton* normalFormatBtn  = new QRadioButton("Normal", m_formatBtnGroup);
    QRadioButton* unifiedFormatBtn = new QRadioButton("Unified", m_formatBtnGroup);
    unifiedFormatBtn->setChecked(true);
                                               
    QLabel* contextLinesLbl = new QLabel(i18n("&Number of context lines:"),
                                         mainWidget);
    m_contextLines = new KIntNumInput(3, mainWidget);
    m_contextLines->setRange(2, 65535, 1, false);
    contextLinesLbl->setBuddy(m_contextLines); 
    
    QBoxLayout* contextLinesLayout = new QHBoxLayout(topLayout);
    contextLinesLayout->addWidget(contextLinesLbl);
    contextLinesLayout->addWidget(m_contextLines);
    
    QVButtonGroup* ignoreBtnGroup = new QVButtonGroup("Ignore Options", mainWidget);
    topLayout->addWidget(ignoreBtnGroup);
    
    m_blankLineChk = new QCheckBox("Ignore added or removed empty lines",
                                   ignoreBtnGroup);
    m_spaceChangeChk = new QCheckBox("Ignore changes in the amount of whitespace",
                                  ignoreBtnGroup);
    m_allSpaceChk = new QCheckBox("Ignore all whitespace",
                                  ignoreBtnGroup);
}


PatchOptionDialog::~PatchOptionDialog()
{
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

    return options;
}


QString PatchOptionDialog::formatOption() const
{
    switch( m_formatBtnGroup->selectedId() )
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

#include "patchoptiondlg.moc"
