/* 
 *  Copyright (C) 1999-2002 Bernd Gehrmann
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


#include "tagdlg.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <kapplication.h>
#include <kbuttonbox.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "cvsprogressdlg.h"
#include "misc.h"


TagDialog::TagDialog(ActionType action, const QString &sbox, const QString &repo,
                     QWidget *parent, const char *name)
    : QDialog(parent, name, true), branchtag_button(0), forcetag_button(0)
{
    setCaption( (action==Delete)? i18n("CVS Delete Tag") : i18n("CVS Tag") );

    QBoxLayout *layout = new QVBoxLayout(this, 10);

    if (action == Delete)
        {
            tag_combo = new QComboBox(true, this);
            tag_combo->setFocus();
            QFontMetrics fm(fontMetrics());
            tag_combo->setMinimumSize(fm.width("0")*30, tag_combo->sizeHint().height());
            
            QLabel *tag_label = new QLabel(tag_combo, i18n("&Name of tag:"), this);

            QPushButton *tag_button = new QPushButton(i18n("Fetch &List"), this);
            tag_button->setMinimumWidth(tag_button->sizeHint().width());
            connect( tag_button, SIGNAL(clicked()),
                     this, SLOT(tagButtonClicked()) );
            
            QBoxLayout *tagedit_layout = new QHBoxLayout();
            layout->addLayout(tagedit_layout);
            tagedit_layout->addWidget(tag_label);
            tagedit_layout->addWidget(tag_combo);
            tagedit_layout->addWidget(tag_button);
        }
    else
        {
            tag_edit = new KLineEdit(this);
            tag_edit->setFocus();
            QFontMetrics fm(fontMetrics());
            tag_edit->setMinimumSize(fm.width("0")*30, tag_edit->sizeHint().height());
            
            QLabel *tag_label = new QLabel(tag_edit, i18n("&Name of tag:"), this);

            QBoxLayout *tagedit_layout = new QHBoxLayout();
            layout->addLayout(tagedit_layout);
            tagedit_layout->addWidget(tag_label);
            tagedit_layout->addWidget(tag_edit);

            branchtag_button = new QCheckBox(i18n("Create &branch with this tag"), this);
            layout->addWidget(branchtag_button);

            forcetag_button = new QCheckBox(i18n("&Force tag creation even if tag already exists"), this);
            layout->addWidget(forcetag_button);
	}
    
    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    frame->setMinimumHeight(frame->sizeHint().height());
    layout->addWidget(frame, 0);

    KButtonBox *buttonbox = new KButtonBox(this);
    QPushButton *helpbutton = buttonbox->addButton(i18n("&Help"));
    helpbutton->setAutoDefault(false);
    buttonbox->addStretch();
    QPushButton *okbutton = buttonbox->addButton(i18n("OK"));
    QPushButton *cancelbutton = buttonbox->addButton(i18n("Cancel"));
    okbutton->setDefault(true);

    connect( helpbutton, SIGNAL(clicked()), SLOT(helpClicked()) );
    connect( okbutton, SIGNAL(clicked()), SLOT(accept()) );
    connect( cancelbutton, SIGNAL(clicked()), SLOT(reject()) );
    
    buttonbox->layout();
    layout->addWidget(buttonbox, 0);

    layout->activate();
    resize(sizeHint());

    act = action;
    sandbox = sbox;
    repository = repo;
}


void TagDialog::done(int r)
{
    if (r == Accepted)
        {
            QString str = tag();
            
            if (str.length() == 0)
                {
                    KMessageBox::sorry(this,
                                       i18n("You must define a tag name."),
                                       "Cervisia");
                    return;
                }

            if (!isValidTag(str))
                {
                    KMessageBox::sorry(this,
                                      i18n("Tag must start with a letter and may contain "
                                           "letters, digits and the characters '-' and '_'."),
                                      "Cervisia");
                    return;
                }
        }
    
    QDialog::done(r);
}


void TagDialog::tagButtonClicked()
{
    QString cmdline = cvsClient(repository);
    cmdline += " status -v";

    CvsProgressDialog l("Status", this);
    l.setCaption(i18n("CVS Status"));
    if (!l.execCommand(sandbox, repository, cmdline, ""))
        return;

    QStrList tags(true);
    QString str;
    while (l.getOneLine(&str))
        {
            int pos1, pos2, pos3;
            if (str.length() < 1 || str[0] != '\t')
                continue;
            if ((pos1 = str.find(' ', 2)) == -1)
                continue;
            if ((pos2 = str.find('(', pos1+1)) == -1)
                continue;
            if ((pos3 = str.find(':', pos2+1)) == -1)
                continue;
            
            QString tag = str.mid(1, pos1-1);
            QString type = str.mid(pos2+1, pos3-pos2-1);
            if (type == QString::fromLatin1("revision") && !tags.contains(tag.latin1()))
                tags.inSort(tag.latin1());
        }

    tag_combo->clear();
    QStrListIterator it(tags);
    for (; it.current(); ++it)
        tag_combo->insertItem(*it);
}


void TagDialog::helpClicked()
{
    kapp->invokeHelp("taggingbranching", "cervisia");
}

#include "tagdlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
