/* 
 *  Copyright (C) 1999-2001 Bernd Gehrmann
 *                          bernd@physik.hu-berlin.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include <qlayout.h>
#include <qlabel.h>
#include <kapp.h>
#include <kbuttonbox.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <klocale.h>
#include "misc.h"

#include "watchdlg.h"
#include "watchdlg.moc"


WatchDialog::WatchDialog(ActionType action, QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption( (action==Add)? i18n("CVS Watch Add") : i18n("CVS Watch Remove") );

    QBoxLayout *layout = new QVBoxLayout(this, 10);
    
    QLabel *textlabel = new QLabel
	( (action==Add)? i18n("Add watches for the following events:")
          :  i18n("Remove watches for the following events:"), this );
    textlabel->setMinimumSize(textlabel->sizeHint());
    layout->addWidget(textlabel, 0);

    all_button = new QRadioButton(i18n("&All"), this);
    all_button->setFocus();
    all_button->setChecked(true);
    all_button->setMinimumSize(all_button->sizeHint());
    layout->addWidget(all_button);
    
    only_button = new QRadioButton(i18n("&Only:"), this);
    only_button->setMinimumSize(only_button->sizeHint());
    layout->addWidget(only_button);

    QGridLayout *eventslayout = new QGridLayout(3, 2);
    layout->addLayout(eventslayout);
    eventslayout->addColSpacing(0, 20);
    eventslayout->setColStretch(0, 0);
    eventslayout->setColStretch(1, 1);
    
    commitbox = new QCheckBox(i18n("&Commits"), this);
    commitbox->setEnabled(false);
    commitbox->setMinimumSize(commitbox->sizeHint());
    eventslayout->addWidget(commitbox, 0, 1, AlignLeft);
    
    editbox = new QCheckBox(i18n("&Edits"), this);
    editbox->setEnabled(false);
    editbox->setMinimumSize(editbox->sizeHint());
    eventslayout->addWidget(editbox, 1, 1, AlignLeft);

    uneditbox = new QCheckBox(i18n("&Unedits"), this);
    uneditbox->setEnabled(false);
    uneditbox->setMinimumSize(uneditbox->sizeHint());
    eventslayout->addWidget(uneditbox, 2, 1, AlignLeft);

    group = new QButtonGroup();
    group->insert(all_button);
    group->insert(only_button);
    connect( only_button, SIGNAL(toggled(bool)),
             commitbox, SLOT(setEnabled(bool)) );
    connect( only_button, SIGNAL(toggled(bool)),
             editbox, SLOT(setEnabled(bool)) );
    connect( only_button, SIGNAL(toggled(bool)),
             uneditbox, SLOT(setEnabled(bool)) );
    
    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    frame->setMinimumHeight(frame->sizeHint().height());
    layout->addWidget(frame, 0);

    KButtonBox *buttonbox = new KButtonBox(this);
    buttonbox->addStretch();
    QPushButton *ok = buttonbox->addButton(i18n("OK"));
    QPushButton *cancel = buttonbox->addButton(i18n("Cancel"));
    ok->setDefault(true);
    connect( ok, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancel, SIGNAL(clicked()), this, SLOT(reject()) );
    buttonbox->layout();
    layout->addWidget(buttonbox, 0);

    layout->activate();
    resize(sizeHint());
}


WatchDialog::Events WatchDialog::events()
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

    
