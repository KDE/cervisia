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


#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <kapp.h>
#include <kbuttonbox.h>
#include <klocale.h>

#include "cvsprogressdlg.h"
#include "misc.h"

#include "mergedlg.h"
#include "mergedlg.moc"


MergeDialog::MergeDialog(const QString &sbox, const QString &repo,
                         QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption(i18n("CVS Merge"));

    QBoxLayout *layout = new QVBoxLayout(this, 10);
    QFontMetrics fm(fontMetrics());

    bybranch_button = new QRadioButton(i18n("Merge from &branch:"), this);
    bybranch_button->setChecked(true);
    layout->addWidget(bybranch_button);

    branch_combo = new QComboBox(true, this);
    branch_combo->setMinimumSize(fm.width("0")*30, branch_combo->sizeHint().height());
    //    branch_combo->setFocus();

    branch_button = new QPushButton(i18n("Fetch &List"), this);
    connect( branch_button, SIGNAL(clicked()),
             this, SLOT(branchButtonClicked()) );
            
    QBoxLayout *branchedit_layout = new QHBoxLayout();
    layout->addLayout(branchedit_layout);
    branchedit_layout->addSpacing(15);
    branchedit_layout->addWidget(branch_combo, 2);
    branchedit_layout->addWidget(branch_button, 0);
    
    bytags_button = new QRadioButton(i18n("Merge &modifications:"), this);
    layout->addWidget(bytags_button);

    QLabel *tag1_label = new QLabel(i18n("between tag: "), this);
    tag1_combo = new QComboBox(true, this);
    tag1_combo->setMinimumSize(fm.width("0")*30, tag1_combo->sizeHint().height());
    tag1_combo->setEnabled(false);
    
    QLabel *tag2_label = new QLabel(i18n("and tag: "), this);
    tag2_combo = new QComboBox(true, this);
    tag2_combo->setMinimumSize(fm.width("0")*30, tag2_combo->sizeHint().height());
    tag2_combo->setEnabled(false);

    tag_button = new QPushButton(i18n("Fetch L&ist"), this);
    connect( tag_button, SIGNAL(clicked()),
             this, SLOT(tagButtonClicked()) );

    QGridLayout *tagsedit_layout = new QGridLayout(2, 4);
    layout->addLayout(tagsedit_layout);
    tagsedit_layout->addColSpacing(0, 15);
    tagsedit_layout->setColStretch(0, 0);
    tagsedit_layout->setColStretch(1, 1);
    tagsedit_layout->setColStretch(2, 2);
    tagsedit_layout->setColStretch(3, 0);
    tagsedit_layout->addWidget(tag1_label, 0, 1);
    tagsedit_layout->addWidget(tag1_combo, 0, 2);
    tagsedit_layout->addWidget(tag2_label, 1, 1);
    tagsedit_layout->addWidget(tag2_combo, 1, 2);
    tagsedit_layout->addMultiCellWidget(tag_button, 0, 1, 3, 3);
    
    group = new QButtonGroup();
    group->insert(bybranch_button);
    group->insert(bytags_button);
    connect( bybranch_button, SIGNAL(toggled(bool)),
             this, SLOT(toggled()) );
    toggled();

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
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


MergeDialog::~MergeDialog()
{
    delete group;
}


// Merge with UpdateDialog code in later version
void MergeDialog::buttonClicked(bool branch)
{
    QString cmdline = cvsClient(repository);
    cmdline += " status -v";

    CvsProgressDialog l("Status", this);
    l.setCaption(i18n("CVS Status"));
    if (!l.execCommand(sandbox, repository, cmdline, ""))
        return;

    QString searchedtype = QString::fromLatin1(branch? "branch" : "revision");

    QStrList tags(true);
    QCString str;
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
            if (type == searchedtype && !tags.contains(tag.latin1()))
                tags.inSort(tag.latin1());
        }

    if (branch)
        branch_combo->clear();
    else
        {
            tag1_combo->clear();
            tag2_combo->clear();
        }
    QStrListIterator it(tags);
    for (; it.current(); ++it)
        if (branch)
            branch_combo->insertItem(*it);
        else
            {
                tag1_combo->insertItem(*it);
                tag2_combo->insertItem(*it);
            }
}


void MergeDialog::tagButtonClicked()
{
    buttonClicked(false);
}


void MergeDialog::branchButtonClicked()
{
    buttonClicked(true);
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

    
