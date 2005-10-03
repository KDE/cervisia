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


#include "watchdlg.h"

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <klocale.h>


WatchDialog::WatchDialog(ActionType action, QWidget *parent, const char *name)
    : KDialogBase(parent, name, true, QString::null,
                  Ok | Cancel | Help, Ok, true)
{
    setCaption( (action==Add)? i18n("CVS Watch Add") : i18n("CVS Watch Remove") );

    QFrame* mainWidget = makeMainWidget();

    QBoxLayout *layout = new QVBoxLayout(mainWidget, 0, spacingHint());

    QLabel *textlabel = new QLabel
	( (action==Add)? i18n("Add watches for the following events:")
          :  i18n("Remove watches for the following events:"), mainWidget );
    layout->addWidget(textlabel, 0);

    all_button = new QRadioButton(i18n("&All"), mainWidget);
    all_button->setFocus();
    all_button->setChecked(true);
    layout->addWidget(all_button);
    
    only_button = new QRadioButton(i18n("&Only:"), mainWidget);
    layout->addWidget(only_button);

    QGridLayout *eventslayout = new QGridLayout(layout);
    eventslayout->addColSpacing(0, 20);
    eventslayout->setColStretch(0, 0);
    eventslayout->setColStretch(1, 1);
    
    commitbox = new QCheckBox(i18n("&Commits"), mainWidget);
    commitbox->setEnabled(false);
    eventslayout->addWidget(commitbox, 0, 1);
    
    editbox = new QCheckBox(i18n("&Edits"), mainWidget);
    editbox->setEnabled(false);
    eventslayout->addWidget(editbox, 1, 1);

    uneditbox = new QCheckBox(i18n("&Unedits"), mainWidget);
    uneditbox->setEnabled(false);
    eventslayout->addWidget(uneditbox, 2, 1);

    QButtonGroup* group = new QButtonGroup(mainWidget);
    group->hide();
    group->insert(all_button);
    group->insert(only_button);

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
