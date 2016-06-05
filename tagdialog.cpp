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


#include "tagdialog.h"

#include <KComboBox>
#include <KHelpClient>
#include <kmessagebox.h>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

#include "misc.h"
#include "cvsserviceinterface.h"

using Cervisia::TagDialog;

TagDialog::TagDialog(ActionType action, OrgKdeCervisia5CvsserviceCvsserviceInterface* service,
                     QWidget *parent)
    : QDialog(parent), 
      act(action),
      cvsService(service),
      branchtag_button(0),
      forcetag_button(0)
{
    setModal(true);
    setWindowTitle( (action==Delete)? i18n("CVS Delete Tag") : i18n("CVS Tag") );

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttonBox, &QDialogButtonBox::helpRequested, this, &TagDialog::slotHelp);

    if ( action == Delete )
    {
        tag_combo = new KComboBox;
        mainLayout->addWidget(tag_combo);
        tag_combo->setEditable(true);
        tag_combo->setFocus();
        tag_combo->setMinimumWidth(fontMetrics().width('0') * 30);

        QLabel *tag_label = new QLabel(i18n("&Name of tag:"));
        mainLayout->addWidget(tag_label);
        tag_label->setBuddy(tag_combo);

        QPushButton *tag_button = new QPushButton(i18n("Fetch &List"));
        mainLayout->addWidget(tag_button);
        connect(tag_button, SIGNAL(clicked()), this, SLOT(tagButtonClicked()));

        QBoxLayout *tagedit_layout = new QHBoxLayout();
        mainLayout->addLayout(tagedit_layout);
        tagedit_layout->addWidget(tag_label);
        tagedit_layout->addWidget(tag_combo);
        tagedit_layout->addWidget(tag_button);
    }
    else
    {
        tag_edit = new QLineEdit;
        mainLayout->addWidget(tag_edit);
        tag_edit->setFocus();
        tag_edit->setMinimumWidth(fontMetrics().width('0') * 30);

        QLabel *tag_label = new QLabel(i18n("&Name of tag:"));
        mainLayout->addWidget(tag_label);
        tag_label->setBuddy( tag_edit );

        QBoxLayout *tagedit_layout = new QHBoxLayout();
        mainLayout->addLayout(tagedit_layout);
        tagedit_layout->addWidget(tag_label);
        tagedit_layout->addWidget(tag_edit);

        branchtag_button = new QCheckBox(i18n("Create &branch with this tag"));
        mainLayout->addWidget(branchtag_button);
        mainLayout->addWidget(branchtag_button);

        forcetag_button = new QCheckBox(i18n("&Force tag creation even if tag already exists"));
        mainLayout->addWidget(forcetag_button);
        mainLayout->addWidget(forcetag_button);
    }

    connect(okButton, SIGNAL(clicked()), this, SLOT(slotOk()));

    mainLayout->addWidget(buttonBox);
}


bool TagDialog::branchTag() const
{
    return branchtag_button && branchtag_button->isChecked();
}


bool TagDialog::forceTag() const
{
    return forcetag_button && forcetag_button->isChecked();
}


QString TagDialog::tag() const
{
    return act==Delete? tag_combo->currentText() : tag_edit->text();
}

void TagDialog::slotHelp()
{
  KHelpClient::invokeHelp(QLatin1String("taggingbranching"));
}

void TagDialog::slotOk()
{
    QString const str(tag());

    if (str.isEmpty())
    {
        KMessageBox::sorry(this,
                           i18n("You must define a tag name."),
                           "Cervisia");
        return;
    }

    if (!Cervisia::IsValidTag(str))
    {
        KMessageBox::sorry(this,
                           i18n("Tag must start with a letter and may contain "
                                "letters, digits and the characters '-' and '_'."),
                           "Cervisia");
        return;
    }

    QDialog::accept();
}


void TagDialog::tagButtonClicked()
{
    tag_combo->clear();
    tag_combo->addItems(::fetchTags(cvsService, this));
}




// Local Variables:
// c-basic-offset: 4
// End:
