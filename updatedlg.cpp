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


#include "updatedlg.h"

#include <qbuttongroup.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <kapplication.h>
#include <kbuttonbox.h>
#include <klocale.h>

#include "cvsprogressdlg.h"
#include "misc.h"


UpdateDialog::UpdateDialog(const QString &sbox, const QString &repo,
                           QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption(i18n("CVS Update"));

    QBoxLayout *layout = new QVBoxLayout(this, 10, 4);
    QFontMetrics fm(fontMetrics());

    bybranch_button = new QRadioButton(i18n("Update to &branch: "), this);
    bybranch_button->setChecked(true);
    layout->addWidget(bybranch_button);

    branch_combo = new QComboBox(true, this);
    branch_combo->setMinimumSize(fm.width("0")*40, branch_combo->sizeHint().height());
    
    branch_button = new QPushButton(i18n("Fetch &List"), this);
    connect( branch_button, SIGNAL(clicked()),
             this, SLOT(branchButtonClicked()) );
            
    QBoxLayout *branchedit_layout = new QHBoxLayout();
    layout->addLayout(branchedit_layout);
    branchedit_layout->addSpacing(15);
    branchedit_layout->addWidget(branch_combo);
    branchedit_layout->addWidget(branch_button);
    
    bytag_button = new QRadioButton(i18n("Update to &tag: "), this);
    layout->addWidget(bytag_button);

    tag_combo = new QComboBox(true, this);
    tag_combo->setMinimumSize(fm.width("0")*40, tag_combo->sizeHint().height());
    
    tag_button = new QPushButton(i18n("Fetch L&ist"), this);
    connect( tag_button, SIGNAL(clicked()),
             this, SLOT(tagButtonClicked()) );
            
    QBoxLayout *tagedit_layout = new QHBoxLayout();
    layout->addLayout(tagedit_layout);
    tagedit_layout->addSpacing(15);
    tagedit_layout->addWidget(tag_combo);
    tagedit_layout->addWidget(tag_button);
    
    bydate_button = new QRadioButton(i18n("Update to &date:\n"
                                          "(Possible format: 'yyyy-mm-dd')"), this);
    bydate_button->setMinimumSize(bydate_button->sizeHint());
    layout->addWidget(bydate_button);

    QBoxLayout *dateedit_layout = new QHBoxLayout();
    layout->addLayout(dateedit_layout);
    date_edit = new KLineEdit(this);
    date_edit->setEnabled(false);
    dateedit_layout->addSpacing(15);
    dateedit_layout->addWidget(date_edit);

    group = new QButtonGroup();
    group->insert(bytag_button);
    group->insert(bybranch_button);
    group->insert(bydate_button);
    connect( bytag_button, SIGNAL(toggled(bool)),
             this, SLOT(toggled()) );
    connect( bybranch_button, SIGNAL(toggled(bool)),
             this, SLOT(toggled()) );
    toggled();

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addSpacing(10);
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

    sandbox = sbox;
    repository = repo;
}


UpdateDialog::~UpdateDialog()
{
    delete group;
}


void UpdateDialog::buttonClicked(bool branch)
{
    QString cmdline = cvsClient(repository);
    cmdline += " status -v";

    CvsProgressDialog l("Status", this);
    l.setCaption(i18n("CVS Status"));
    if (!l.execCommand(sandbox, repository, cmdline, ""))
        return;

    QComboBox *combo = branch? branch_combo : tag_combo;
    QString searchedtype = QString::fromLatin1(branch? "branch" : "revision");

    QStringList tags;
    QString str;
    while (l.getOneLine(&str))
        {
            int pos1, pos2, pos3;
            if (str.length() < 1 || str[0] != '\t')
                continue;
            if (((pos1 = str.find(' ', 2)) == -1) || ((pos1 = str.find('\t', 2)) == -1))
               continue;
            if ((pos2 = str.find('(', pos1+1)) == -1)
               continue;
            if ((pos3 = str.find(':', pos2+1)) == -1)
                continue;
            
            QString tag = str.mid(1, pos1-1);
            QString type = str.mid(pos2+1, pos3-pos2-1);
            if (type == searchedtype && !tags.contains(tag))
                tags.append(tag);
        }

    combo->clear();
    tags.sort();

    QStringList::ConstIterator it;
    for (it = tags.begin(); it != tags.end(); ++it)
        combo->insertItem(*it);
}


void UpdateDialog::tagButtonClicked()
{
    buttonClicked(false);
}


void UpdateDialog::branchButtonClicked()
{
    buttonClicked(true);
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

#include "updatedlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
