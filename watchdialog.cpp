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


#include "watchdialog.h"

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <QGridLayout>
#include <QBoxLayout>

#include <klocale.h>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>


WatchDialog::WatchDialog(ActionType action, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle( (action==Add)? i18n("CVS Watch Add") : i18n("CVS Watch Remove") );
    setModal(true);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Help);
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

    QBoxLayout *layout = new QVBoxLayout(mainWidget);
    mainLayout->addWidget(layout);
    layout->setSpacing(spacingHint());
    layout->setMargin(0);

    QLabel *textlabel = new QLabel
	( (action==Add)? i18n("Add watches for the following events:")
          :  i18n("Remove watches for the following events:"), mainWidget );
    layout->addWidget(textlabel, 0);

    all_button = new QRadioButton(i18n("&All"), mainWidget);
    mainLayout->addWidget(all_button);
    all_button->setFocus();
    all_button->setChecked(true);
    layout->addWidget(all_button);

    only_button = new QRadioButton(i18n("&Only:"), mainWidget);
    mainLayout->addWidget(only_button);
    layout->addWidget(only_button);

    QGridLayout *eventslayout = new QGridLayout();
    layout->addLayout( eventslayout );
    eventslayout->addItem(new QSpacerItem(20, 0), 0, 0);
    eventslayout->setColumnStretch(0, 0);
    eventslayout->setColumnStretch(1, 1);

    commitbox = new QCheckBox(i18n("&Commits"), mainWidget);
    mainLayout->addWidget(commitbox);
    commitbox->setEnabled(false);
    eventslayout->addWidget(commitbox, 0, 1);

    editbox = new QCheckBox(i18n("&Edits"), mainWidget);
    mainLayout->addWidget(editbox);
    editbox->setEnabled(false);
    eventslayout->addWidget(editbox, 1, 1);

    uneditbox = new QCheckBox(i18n("&Unedits"), mainWidget);
    mainLayout->addWidget(uneditbox);
    uneditbox->setEnabled(false);
    eventslayout->addWidget(uneditbox, 2, 1);

    QButtonGroup* group = new QButtonGroup(mainWidget);
    mainLayout->addWidget(group);
    group->addButton(all_button);
    group->addButton(only_button);

    connect( only_button, SIGNAL(toggled(bool)),
             commitbox, SLOT(setEnabled(bool)) );
    connect( only_button, SIGNAL(toggled(bool)),
             editbox, SLOT(setEnabled(bool)) );
    connect( only_button, SIGNAL(toggled(bool)),
             uneditbox, SLOT(setEnabled(bool)) );

    setHelp("watches");
}


WatchDialog::Events WatchDialog::events() const
{
    Events res = None;
    if (all_button->isChecked())
        res = All;
    else
        {
            if (commitbox->isChecked())
                res = Events(res | Commits);
            if (editbox->isChecked())
                res = Events(res | Edits);
            if (uneditbox->isChecked())
                res = Events(res | Unedits);
        }
    return res;
}


// Local Variables:
// c-basic-offset: 4
// End:
