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


#include "updatedialog.h"

#include <qbuttongroup.h>
#include <KComboBox>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qstyle.h>
#include <QBoxLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <KConfigGroup>
#include <KLocalizedString>

#include "misc.h"
#include "cvsserviceinterface.h"


UpdateDialog::UpdateDialog(OrgKdeCervisia5CvsserviceCvsserviceInterface* service,
                           QWidget *parent)
    : QDialog(parent),
      cvsService(service)
{
    setWindowTitle(i18n("CVS Update"));
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    const int iComboBoxMinWidth(40 * fontMetrics().width('0'));
    const int iWidgetIndent(style()->pixelMetric(QStyle::PM_ExclusiveIndicatorWidth) + 6);

    bybranch_button = new QRadioButton(i18n("Update to &branch: "));
    mainLayout->addWidget(bybranch_button);
    bybranch_button->setChecked(true);

    branch_combo = new KComboBox;
    mainLayout->addWidget(branch_combo);
    branch_combo->setEditable(true);
    branch_combo->setMinimumWidth(iComboBoxMinWidth);
    
    branch_button = new QPushButton(i18n("Fetch &List"));
    mainLayout->addWidget(branch_button);
    connect(branch_button, SIGNAL(clicked()), this, SLOT(branchButtonClicked()));
            
    QBoxLayout *branchedit_layout = new QHBoxLayout;
    branchedit_layout->addSpacing(iWidgetIndent);
    branchedit_layout->addWidget(branch_combo);
    branchedit_layout->addWidget(branch_button);
    mainLayout->addLayout(branchedit_layout);
    
    bytag_button = new QRadioButton(i18n("Update to &tag: "));
    mainLayout->addWidget(bytag_button);

    tag_combo = new KComboBox;
    tag_combo->setEditable(true);
    tag_combo->setMinimumWidth(iComboBoxMinWidth);
    mainLayout->addWidget(tag_combo);
    
    tag_button = new QPushButton(i18n("Fetch L&ist"));
    mainLayout->addWidget(tag_button);
    connect(tag_button, SIGNAL(clicked()), this, SLOT(tagButtonClicked()));
            
    QBoxLayout *tagedit_layout = new QHBoxLayout();
    tagedit_layout->addSpacing(iWidgetIndent);
    tagedit_layout->addWidget(tag_combo);
    tagedit_layout->addWidget(tag_button);
    mainLayout->addLayout(tagedit_layout);
    
    bydate_button = new QRadioButton(i18n("Update to &date ('yyyy-mm-dd'):"));
    mainLayout->addWidget(bydate_button);

    date_edit = new QLineEdit;
    mainLayout->addWidget(date_edit);

    QBoxLayout *dateedit_layout = new QHBoxLayout;
    dateedit_layout->addSpacing(iWidgetIndent);
    dateedit_layout->addWidget(date_edit);
    mainLayout->addLayout(dateedit_layout);

    QButtonGroup* group = new QButtonGroup(this);
    group->addButton(bytag_button);
    group->addButton(bybranch_button);
    group->addButton(bydate_button);
    connect(group, SIGNAL(buttonClicked(int)), this, SLOT(toggled()));

    mainLayout->addWidget(buttonBox);

    // dis-/enable the widgets
    toggled();
}


bool UpdateDialog::byTag() const
{
    return bybranch_button->isChecked() || bytag_button->isChecked();
}


QString UpdateDialog::tag() const
{
    return bybranch_button->isChecked()
        ? branch_combo->currentText()
        : tag_combo->currentText();
}


QString UpdateDialog::date() const
{
    return date_edit->text();
}


void UpdateDialog::tagButtonClicked()
{
    tag_combo->clear();
    tag_combo->addItems(::fetchTags(cvsService, this));
}


void UpdateDialog::branchButtonClicked()
{
    branch_combo->clear();
    branch_combo->addItems(::fetchBranches(cvsService, this));
}


void UpdateDialog::toggled()
{
    bool bytag = bytag_button->isChecked();
    tag_combo->setEnabled(bytag);
    tag_button->setEnabled(bytag);
    if (bytag)
        tag_combo->setFocus();

    bool bybranch = bybranch_button->isChecked();
    branch_combo->setEnabled(bybranch);
    branch_button->setEnabled(bybranch);
    if (bybranch)
        branch_combo->setFocus();

    bool bydate = bydate_button->isChecked();
    date_edit->setEnabled(bydate);
    if (bydate)
        date_edit->setFocus();
}



// Local Variables:
// c-basic-offset: 4
// End:
