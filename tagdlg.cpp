/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "tagdlg.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "misc.h"
#include "cvsservice_stub.h"


TagDialog::TagDialog(ActionType action, CvsService_stub* service,
                     QWidget *parent, const char *name)
    : KDialogBase(parent, name, true, QString::null,
                  Ok | Cancel | Help, Ok, true),
      act(action),
      cvsService(service),
      branchtag_button(0),
      forcetag_button(0)
{
    setCaption( (action==Delete)? i18n("CVS Delete Tag") : i18n("CVS Tag") );

    QFrame* mainWidget = makeMainWidget();

    QBoxLayout *layout = new QVBoxLayout(mainWidget, 0, spacingHint());

    if (action == Delete)
        {
            tag_combo = new QComboBox(true, mainWidget);
            tag_combo->setFocus();
            tag_combo->setMinimumWidth(fontMetrics().width('0') * 30);

            QLabel *tag_label = new QLabel(tag_combo, i18n("&Name of tag:"), mainWidget);

            QPushButton *tag_button = new QPushButton(i18n("Fetch &List"), mainWidget);
            connect( tag_button, SIGNAL(clicked()),
                     this, SLOT(tagButtonClicked()) );

            QBoxLayout *tagedit_layout = new QHBoxLayout(layout);
            tagedit_layout->addWidget(tag_label);
            tagedit_layout->addWidget(tag_combo);
            tagedit_layout->addWidget(tag_button);
        }
    else
        {
            tag_edit = new QLineEdit(mainWidget);
            tag_edit->setFocus();
            tag_edit->setMinimumWidth(fontMetrics().width('0') * 30);

            QLabel *tag_label = new QLabel(tag_edit, i18n("&Name of tag:"), mainWidget);

            QBoxLayout *tagedit_layout = new QHBoxLayout(layout);
            tagedit_layout->addWidget(tag_label);
            tagedit_layout->addWidget(tag_edit);

            branchtag_button = new QCheckBox(i18n("Create &branch with this tag"), mainWidget);
            layout->addWidget(branchtag_button);

            forcetag_button = new QCheckBox(i18n("&Force tag creation even if tag already exists"), mainWidget);
            layout->addWidget(forcetag_button);
	}

    setHelp("taggingbranching");
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

    KDialogBase::slotOk();
}


void TagDialog::tagButtonClicked()
{
    tag_combo->clear();
    tag_combo->insertStringList(::fetchTags(cvsService, this));
}


#include "tagdlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
