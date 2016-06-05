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


#include "mergedialog.h"

#include <qbuttongroup.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qstyle.h>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <KComboBox>
#include <KConfigGroup>
#include <KLocalizedString>

#include "misc.h"
#include "cvsserviceinterface.h"


MergeDialog::MergeDialog(OrgKdeCervisia5CvsserviceCvsserviceInterface* service,
                         QWidget *parent)
    : QDialog(parent),
      cvsService(service)
{
    setWindowTitle(i18n("CVS Merge"));
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    const int iComboBoxMinWidth(30 * fontMetrics().width('0'));
    const int iWidgetIndent(style()->pixelMetric(QStyle::PM_ExclusiveIndicatorWidth) + 6);

    bybranch_button = new QRadioButton(i18n("Merge from &branch:"));
    bybranch_button->setChecked(true);
    mainLayout->addWidget(bybranch_button);

    branch_combo = new KComboBox;
    branch_combo->setEditable(true);
    branch_combo->setMinimumWidth(iComboBoxMinWidth);
    mainLayout->addWidget(branch_combo);

    branch_button = new QPushButton(i18n("Fetch &List"));
    mainLayout->addWidget(branch_button);
    connect(branch_button, SIGNAL(clicked()), this, SLOT(branchButtonClicked()));

    QBoxLayout *branchedit_layout = new QHBoxLayout();
    branchedit_layout->addSpacing(iWidgetIndent);
    branchedit_layout->addWidget(branch_combo, 2);
    branchedit_layout->addWidget(branch_button, 0);
    mainLayout->addLayout(branchedit_layout);

    bytags_button = new QRadioButton(i18n("Merge &modifications:"));
    mainLayout->addWidget(bytags_button);

    QLabel *tag1_label = new QLabel(i18n("between tag: "));

    tag1_combo = new KComboBox;
    tag1_combo->setEditable(true);
    tag1_combo->setMinimumWidth(iComboBoxMinWidth);

    QLabel *tag2_label = new QLabel(i18n("and tag: "));

    tag2_combo = new KComboBox;
    tag2_combo->setEditable(true);
    tag2_combo->setMinimumWidth(iComboBoxMinWidth);

    tag_button = new QPushButton(i18n("Fetch L&ist"));
    connect(tag_button, SIGNAL(clicked()), this, SLOT(tagButtonClicked()));

    QGridLayout *tagsedit_layout = new QGridLayout();
    tagsedit_layout->addItem(new QSpacerItem(iWidgetIndent, 0), 0, 0);
    tagsedit_layout->setColumnStretch(0, 0);
    tagsedit_layout->setColumnStretch(1, 1);
    tagsedit_layout->setColumnStretch(2, 2);
    tagsedit_layout->setColumnStretch(3, 0);
    tagsedit_layout->addWidget(tag1_label, 0, 1);
    tagsedit_layout->addWidget(tag1_combo, 0, 2);
    tagsedit_layout->addWidget(tag2_label, 1, 1);
    tagsedit_layout->addWidget(tag2_combo, 1, 2);
    tagsedit_layout->addWidget(tag_button, 0, 3, 2, 1);
    mainLayout->addLayout( tagsedit_layout );

    QButtonGroup* group = new QButtonGroup(this);
    group->addButton(bybranch_button);
    group->addButton(bytags_button);
    connect(group, SIGNAL(buttonClicked(int)), this, SLOT(toggled()));

    mainLayout->addWidget(buttonBox);

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
    tag1_combo->addItems(listTags);
    tag2_combo->clear();
    tag2_combo->addItems(listTags);
}


void MergeDialog::branchButtonClicked()
{
    branch_combo->clear();
    branch_combo->addItems(::fetchBranches(cvsService, this));
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



// Local Variables:
// c-basic-offset: 4
// End:
