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


#include "mergedlg.h"

#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qstyle.h>
#include <klocale.h>

#include "cvsprogressdlg.h"
#include "misc.h"


MergeDialog::MergeDialog(const QString &sbox, const QString &repo,
                         QWidget *parent, const char *name)
    : KDialogBase(parent, name, true, i18n("CVS Merge"),
                  Ok | Cancel, Ok, true),
      sandbox(sbox),
      repository(repo)
{
    int const iComboBoxMinWidth(30 * fontMetrics().width('0'));
    int const iWidgetIndent(style().pixelMetric(QStyle::PM_ExclusiveIndicatorWidth, 0) + 6);

    QFrame* mainWidget = makeMainWidget();

    QBoxLayout *layout = new QVBoxLayout(mainWidget, 0, spacingHint());

    bybranch_button = new QRadioButton(i18n("Merge from &branch:"), mainWidget);
    bybranch_button->setChecked(true);
    layout->addWidget(bybranch_button);

    branch_combo = new QComboBox(true, mainWidget);
    branch_combo->setMinimumWidth(iComboBoxMinWidth);

    branch_button = new QPushButton(i18n("Fetch &List"), mainWidget);
    connect( branch_button, SIGNAL(clicked()),
             this, SLOT(branchButtonClicked()) );
            
    QBoxLayout *branchedit_layout = new QHBoxLayout(layout);
    branchedit_layout->addSpacing(iWidgetIndent);
    branchedit_layout->addWidget(branch_combo, 2);
    branchedit_layout->addWidget(branch_button, 0);
    
    bytags_button = new QRadioButton(i18n("Merge &modifications:"), mainWidget);
    layout->addWidget(bytags_button);

    QLabel *tag1_label = new QLabel(i18n("between tag: "), mainWidget);
    tag1_combo = new QComboBox(true, mainWidget);
    tag1_combo->setMinimumWidth(iComboBoxMinWidth);
    
    QLabel *tag2_label = new QLabel(i18n("and tag: "), mainWidget);
    tag2_combo = new QComboBox(true, mainWidget);
    tag2_combo->setMinimumWidth(iComboBoxMinWidth);

    tag_button = new QPushButton(i18n("Fetch L&ist"), mainWidget);
    connect( tag_button, SIGNAL(clicked()),
             this, SLOT(tagButtonClicked()) );

    QGridLayout *tagsedit_layout = new QGridLayout(layout);
    tagsedit_layout->addColSpacing(0, iWidgetIndent);
    tagsedit_layout->setColStretch(0, 0);
    tagsedit_layout->setColStretch(1, 1);
    tagsedit_layout->setColStretch(2, 2);
    tagsedit_layout->setColStretch(3, 0);
    tagsedit_layout->addWidget(tag1_label, 0, 1);
    tagsedit_layout->addWidget(tag1_combo, 0, 2);
    tagsedit_layout->addWidget(tag2_label, 1, 1);
    tagsedit_layout->addWidget(tag2_combo, 1, 2);
    tagsedit_layout->addMultiCellWidget(tag_button, 0, 1, 3, 3);
    
    QButtonGroup* group = new QButtonGroup(mainWidget);
    group->hide();
    group->insert(bybranch_button);
    group->insert(bytags_button);
    connect( group, SIGNAL(clicked(int)),
             this, SLOT(toggled()) );

    // dis-/enable the widgets
    toggled();
}


bool MergeDialog::byBranch() const
{
    return bybranch_button->isChecked();
}


QString MergeDialog::branch() const
{
    return branch_combo->currentText();
}


QString MergeDialog::tag1() const
{
    return tag1_combo->currentText();
}


QString MergeDialog::tag2() const
{
    return tag2_combo->currentText();
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

    QStringList tags;
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
            if (type == searchedtype && !tags.contains(tag))
                tags.append(tag);
        }

    if (branch)
        branch_combo->clear();
    else
        {
            tag1_combo->clear();
            tag2_combo->clear();
        }

    tags.sort();

    
    QStringList::ConstIterator it;
    for (it = tags.begin(); it != tags.end(); ++it)
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

#include "mergedlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
