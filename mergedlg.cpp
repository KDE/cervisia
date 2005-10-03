/* 
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
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


#include "mergedlg.h"

#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qstyle.h>
#include <klocale.h>

#include "misc.h"
#include "cvsservice_stub.h"


MergeDialog::MergeDialog(CvsService_stub* service,
                         QWidget *parent, const char *name)
    : KDialogBase(parent, name, true, i18n("CVS Merge"),
                  Ok | Cancel, Ok, true),
      cvsService(service)
{
    int const iComboBoxMinWidth(30 * fontMetrics().width('0'));
    int const iWidgetIndent(style().pixelMetric(QStyle::PM_ExclusiveIndicatorWidth, 0) + 6);

    QFrame* mainWidget = makeMainWidget();

    QBoxLayout *layout = new QVBoxLayout(mainWidget, 0, spacingHint());

    bybranch_button = new QRadioButton(i18n("Merge from &branch:"), mainWidget);
    bybranch_button->setChecked(true);
    layout->addWidget(bybranch_button);

    branch_combo = new QComboBox(true, mainWidget);
    branch_combo->setMinimumWidth(iComboBoxMinWidth);

    branch_button = new QPushButton(i18n("Fetch &List"), mainWidget);
    connect( branch_button, SIGNAL(clicked()),
             this, SLOT(branchButtonClicked()) );
            
    QBoxLayout *branchedit_layout = new QHBoxLayout(layout);
    branchedit_layout->addSpacing(iWidgetIndent);
    branchedit_layout->addWidget(branch_combo, 2);
    branchedit_layout->addWidget(branch_button, 0);
    
    bytags_button = new QRadioButton(i18n("Merge &modifications:"), mainWidget);
    layout->addWidget(bytags_button);

    QLabel *tag1_label = new QLabel(i18n("between tag: "), mainWidget);
    tag1_combo = new QComboBox(true, mainWidget);
    tag1_combo->setMinimumWidth(iComboBoxMinWidth);
    
    QLabel *tag2_label = new QLabel(i18n("and tag: "), mainWidget);
    tag2_combo = new QComboBox(true, mainWidget);
    tag2_combo->setMinimumWidth(iComboBoxMinWidth);

    tag_button = new QPushButton(i18n("Fetch L&ist"), mainWidget);
    connect( tag_button, SIGNAL(clicked()),
             this, SLOT(tagButtonClicked()) );

    QGridLayout *tagsedit_layout = new QGridLayout(layout);
    tagsedit_layout->addColSpacing(0, iWidgetIndent);
    tagsedit_layout->setColStretch(0, 0);
    tagsedit_layout->setColStretch(1, 1);
    tagsedit_layout->setColStretch(2, 2);
    tagsedit_layout->setColStretch(3, 0);
    tagsedit_layout->addWidget(tag1_label, 0, 1);
    tagsedit_layout->addWidget(tag1_combo, 0, 2);
    tagsedit_layout->addWidget(tag2_label, 1, 1);
    tagsedit_layout->addWidget(tag2_combo, 1, 2);
    tagsedit_layout->addMultiCellWidget(tag_button, 0, 1, 3, 3);
    
    QButtonGroup* group = new QButtonGroup(mainWidget);
    group->hide();
    group->insert(bybranch_button);
    group->insert(bytags_button);
    connect( group, SIGNAL(clicked(int)),
             this, SLOT(toggled()) );

    // dis-/enable the widgets
    toggled();
}


bool MergeDialog::byBranch() const
{
    return bybranch_button->isChecked();
}


QString MergeDialog::branch() const
{
    return branch_combo->currentText();
}


QString MergeDialog::tag1() const
{
    return tag1_combo->currentText();
}


QString MergeDialog::tag2() const
{
    return tag2_combo->currentText();
}


void MergeDialog::tagButtonClicked()
{
    QStringList const listTags(::fetchTags(cvsService, this));
    tag1_combo->clear();
    tag1_combo->insertStringList(listTags);
    tag2_combo->clear();
    tag2_combo->insertStringList(listTags);
}


void MergeDialog::branchButtonClicked()
{
    branch_combo->clear();
    branch_combo->insertStringList(::fetchBranches(cvsService, this));
}


void MergeDialog::toggled()
{
    bool bybranch = bybranch_button->isChecked();
    branch_combo->setEnabled(bybranch);
    branch_button->setEnabled(bybranch);
    tag1_combo->setEnabled(!bybranch);
    tag2_combo->setEnabled(!bybranch);
    tag_button->setEnabled(!bybranch);
    if (bybranch)
        branch_combo->setFocus();
    else
        tag1_combo->setFocus();
}

#include "mergedlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
