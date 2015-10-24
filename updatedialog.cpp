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
#include <klocale.h>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "misc.h"
#include "cvsserviceinterface.h"


UpdateDialog::UpdateDialog(OrgKdeCervisiaCvsserviceCvsserviceInterface* service,
                           QWidget *parent)
    : QDialog(parent),
      cvsService(service)
{
    setWindowTitle(i18n("CVS Update"));
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

    int const iComboBoxMinWidth(40 * fontMetrics().width('0'));
    int const iWidgetIndent(style()->pixelMetric(QStyle::PM_ExclusiveIndicatorWidth) + 6);

    QFrame* mainWidget = new QFrame(this);
    mainLayout->addWidget(mainWidget);

    QBoxLayout *layout = new QVBoxLayout(mainWidget);
    mainLayout->addWidget(layout);
    layout->setSpacing(spacingHint());
    layout->setMargin(0);

    bybranch_button = new QRadioButton(i18n("Update to &branch: "), mainWidget);
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
    branchedit_layout->addWidget(branch_combo);
    branchedit_layout->addWidget(branch_button);
    
    bytag_button = new QRadioButton(i18n("Update to &tag: "), mainWidget);
    mainLayout->addWidget(bytag_button);
    layout->addWidget(bytag_button);

    tag_combo = new KComboBox(mainWidget);
    mainLayout->addWidget(tag_combo);
    tag_combo->setEditable(true);
    tag_combo->setMinimumWidth(iComboBoxMinWidth);
    
    tag_button = new QPushButton(i18n("Fetch L&ist"), mainWidget);
    mainLayout->addWidget(tag_button);
    connect( tag_button, SIGNAL(clicked()),
             this, SLOT(tagButtonClicked()) );
            
    QBoxLayout *tagedit_layout = new QHBoxLayout();
    layout->addLayout(tagedit_layout);
    tagedit_layout->addSpacing(iWidgetIndent);
    tagedit_layout->addWidget(tag_combo);
    tagedit_layout->addWidget(tag_button);
    
    bydate_button = new QRadioButton(i18n("Update to &date ('yyyy-mm-dd'):"), mainWidget);
    mainLayout->addWidget(bydate_button);
    layout->addWidget(bydate_button);

    date_edit = new QLineEdit(mainWidget);
    mainLayout->addWidget(date_edit);

    QBoxLayout *dateedit_layout = new QHBoxLayout();
    layout->addLayout(dateedit_layout);
    dateedit_layout->addSpacing(iWidgetIndent);
    dateedit_layout->addWidget(date_edit);

    QButtonGroup* group = new QButtonGroup(mainWidget);
    mainLayout->addWidget(group);
    group->addButton(bytag_button);
    group->addButton(bybranch_button);
    group->addButton(bydate_button);
    connect( group, SIGNAL(buttonClicked(int)),
             this, SLOT(toggled()) );

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
