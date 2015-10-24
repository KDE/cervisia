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
#include <KComboBox>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qstyle.h>
#include <QGridLayout>
#include <QBoxLayout>

#include <klocale.h>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "misc.h"
#include "cvsserviceinterface.h"


MergeDialog::MergeDialog(OrgKdeCervisiaCvsserviceCvsserviceInterface* service,
                         QWidget *parent)
    : QDialog(parent),
      cvsService(service)
{
    setWindowTitle(i18n("CVS Merge"));
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

    int const iComboBoxMinWidth(30 * fontMetrics().width('0'));
    int const iWidgetIndent(style()->pixelMetric(QStyle::PM_ExclusiveIndicatorWidth) + 6);

    QFrame* mainWidget = new QFrame(this);
    mainLayout->addWidget(mainWidget);

    QBoxLayout *layout = new QVBoxLayout(mainWidget);
    mainLayout->addWidget(layout);
    layout->setSpacing(spacingHint());
    layout->setMargin(0);

    bybranch_button = new QRadioButton(i18n("Merge from &branch:"), mainWidget);
    mainLayout->addWidget(bybranch_button);
    bybranch_button->setChecked(true);
    layout->addWidget(bybranch_button);

    branch_combo = new KComboBox(mainWidget);
    mainLayout->addWidget(branch_combo);
    branch_combo->setEditable(true);
    branch_combo->setMinimumWidth(iComboBoxMinWidth);

    branch_button = new QPushButton(i18n("Fetch &List"), mainWidget);
    mainLayout->addWidget(branch_button);
    connect( branch_button, SIGNAL(clicked()),
             this, SLOT(branchButtonClicked()) );

    QBoxLayout *branchedit_layout = new QHBoxLayout();
    layout->addLayout(branchedit_layout);
    branchedit_layout->addSpacing(iWidgetIndent);
    branchedit_layout->addWidget(branch_combo, 2);
    branchedit_layout->addWidget(branch_button, 0);

    bytags_button = new QRadioButton(i18n("Merge &modifications:"), mainWidget);
    mainLayout->addWidget(bytags_button);
    layout->addWidget(bytags_button);

    QLabel *tag1_label = new QLabel(i18n("between tag: "), mainWidget);
    mainLayout->addWidget(tag1_label);
    tag1_combo = new KComboBox(mainWidget);
    mainLayout->addWidget(tag1_combo);
    tag1_combo->setEditable(true);
    tag1_combo->setMinimumWidth(iComboBoxMinWidth);

    QLabel *tag2_label = new QLabel(i18n("and tag: "), mainWidget);
    mainLayout->addWidget(tag2_label);
    tag2_combo = new KComboBox(mainWidget);
    mainLayout->addWidget(tag2_combo);
    tag2_combo->setEditable(true);
    tag2_combo->setMinimumWidth(iComboBoxMinWidth);

    tag_button = new QPushButton(i18n("Fetch L&ist"), mainWidget);
    mainLayout->addWidget(tag_button);
    connect( tag_button, SIGNAL(clicked()),
             this, SLOT(tagButtonClicked()) );

    QGridLayout *tagsedit_layout = new QGridLayout();
    layout->addLayout( tagsedit_layout );
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

    QButtonGroup* group = new QButtonGroup(mainWidget);
    mainLayout->addWidget(group);
    group->addButton(bybranch_button);
    group->addButton(bytags_button);
    connect( group, SIGNAL(buttonClicked(int)),
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
