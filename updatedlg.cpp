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
#include <qcombobox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qstyle.h>
#include <klineedit.h>
#include <klocale.h>

#include "cvsprogressdlg.h"
#include "misc.h"


UpdateDialog::UpdateDialog(const QString &sbox, const QString &repo,
                           QWidget *parent, const char *name)
    : KDialogBase(parent, name, true, i18n("CVS Update"),
                  Ok | Cancel, Ok, true),
      sandbox(sbox),
      repository(repo)
{
    int const iComboBoxMinWidth(40 * fontMetrics().width('0'));
    int const iWidgetIndent(style().pixelMetric(QStyle::PM_ExclusiveIndicatorWidth, 0) + 6);

    QFrame* mainWidget = makeMainWidget();

    QBoxLayout *layout = new QVBoxLayout(mainWidget, 0, spacingHint());

    bybranch_button = new QRadioButton(i18n("Update to &branch: "), mainWidget);
    bybranch_button->setChecked(true);
    layout->addWidget(bybranch_button);

    branch_combo = new QComboBox(true, mainWidget);
    branch_combo->setMinimumWidth(iComboBoxMinWidth);
    
    branch_button = new QPushButton(i18n("Fetch &List"), mainWidget);
    connect( branch_button, SIGNAL(clicked()),
             this, SLOT(branchButtonClicked()) );
            
    QBoxLayout *branchedit_layout = new QHBoxLayout(layout);
    branchedit_layout->addSpacing(iWidgetIndent);
    branchedit_layout->addWidget(branch_combo);
    branchedit_layout->addWidget(branch_button);
    
    bytag_button = new QRadioButton(i18n("Update to &tag: "), mainWidget);
    layout->addWidget(bytag_button);

    tag_combo = new QComboBox(true, mainWidget);
    tag_combo->setMinimumWidth(iComboBoxMinWidth);
    
    tag_button = new QPushButton(i18n("Fetch L&ist"), mainWidget);
    connect( tag_button, SIGNAL(clicked()),
             this, SLOT(tagButtonClicked()) );
            
    QBoxLayout *tagedit_layout = new QHBoxLayout(layout);
    tagedit_layout->addSpacing(iWidgetIndent);
    tagedit_layout->addWidget(tag_combo);
    tagedit_layout->addWidget(tag_button);
    
    bydate_button = new QRadioButton(i18n("Update to &date:\n"
                                          "(Possible format: 'yyyy-mm-dd')"), mainWidget);
    layout->addWidget(bydate_button);

    date_edit = new KLineEdit(mainWidget);

    QBoxLayout *dateedit_layout = new QHBoxLayout(layout);
    dateedit_layout->addSpacing(iWidgetIndent);
    dateedit_layout->addWidget(date_edit);

    QButtonGroup* group = new QButtonGroup(mainWidget);
    group->hide();
    group->insert(bytag_button);
    group->insert(bybranch_button);
    group->insert(bydate_button);
    connect( group, SIGNAL(clicked(int)),
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
