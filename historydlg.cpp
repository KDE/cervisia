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


#include <qcheckbox.h>
#include <qlayout.h>
#include <qregexp.h>
#include <kapp.h>
#include <kbuttonbox.h>
#include <kconfig.h>
#include <klineedit.h>
#include <klocale.h>

#include "cvsprogressdlg.h"
#include "listview.h"
#include "misc.h"

#include "historydlg.h"
#include <qpushbutton.h>
#include "historydlg.moc"


class HistoryItem : public QListViewItem
{
public:
    HistoryItem(QListView *parent, QString idx)
        : QListViewItem(parent)
        {
            index = idx;
        }
    HistoryItem(QListView *parent, HistoryDialog::ItemCopy *copy)
        : QListViewItem(parent)
        {
            for (int i = 0; i < 7; ++i)
                setText(i, copy->text[i]);
            index = copy->index;
        }
    // Argl... will be much simpler with Qt 2.x which has QListView::takeItem()
    // ... will be even simpler with Qt 3.x which has QListViewitem::setVisible() ;-)
    HistoryDialog::ItemCopy *itemCopy() const
        {
            HistoryDialog::ItemCopy *copy = new HistoryDialog::ItemCopy;
            for (int i = 0; i < 7; ++i)
                copy->text[i] = text(i);
            copy->index = index;
            return copy;
        }

    virtual QString key(int col, bool) const;

private:
    QString index;
};


QString HistoryItem::key(int col, bool) const
{
    return (col == 0)? index : text(col);
}


bool HistoryDialog::ItemCopy::isCommit()
{
    return text[1] == i18n("Commit, Modified ")
        || text[1] == i18n("Commit, Added ")
        || text[1] == i18n("Commit, Removed ");
}


bool HistoryDialog::ItemCopy::isCheckout()
{
    return text[1] == i18n("Checkout ");
}


bool HistoryDialog::ItemCopy::isTag()
{
    return text[1] == i18n("Tag");
}


bool HistoryDialog::ItemCopy::isOther()
{
    return !isCommit() && !isCheckout() && !isTag();
}


HistoryDialog::Options *HistoryDialog::options = 0;


HistoryDialog::HistoryDialog(QWidget *parent, const char *name)
    : QDialog(parent, name, false, WStyle_MinMax)
{
    QBoxLayout *layout = new QVBoxLayout(this, 10, 0);

    hiddenitems.setAutoDelete(true);
    
    listview = new ListView(this);
    listview->setSelectionMode(QListView::NoSelection);
    listview->setAllColumnsShowFocus(true);
    listview->setShowSortIndicator(true);
    listview->setSorting(0, false); 
    listview->addColumn(i18n("Date"));
    listview->addColumn(i18n("Event"));
    listview->addColumn(i18n("Author"));
    listview->addColumn(i18n("Revision"));
    listview->addColumn(i18n("File"));
    listview->addColumn(i18n("Repo path"));
    listview->setPreferredColumn(5);
    listview->setFocus();
    layout->addWidget(listview, 1);
    layout->addSpacing(10);

    commit_box = new QCheckBox(i18n("Show &commit events"), this);
    commit_box->setChecked(true);
    commit_box->setMinimumSize(commit_box->sizeHint());
    checkout_box = new QCheckBox(i18n("Show c&heckout events"), this);
    checkout_box->setChecked(true);
    checkout_box->setMinimumSize(checkout_box->sizeHint());
    tag_box = new QCheckBox(i18n("Show &tag events"), this);
    tag_box->setChecked(true);
    tag_box->setMinimumSize(tag_box->sizeHint());
    other_box = new QCheckBox(i18n("Show &other events"), this);
    other_box->setChecked(true);
    other_box->setMinimumSize(tag_box->sizeHint());
    onlyuser_box = new QCheckBox(i18n("Only &user:"), this);
    onlyuser_box->setMinimumSize(onlyuser_box->sizeHint());
    onlyfilenames_box = new QCheckBox(i18n("Only &filenames matching:"), this);
    onlyfilenames_box->setMinimumSize(onlyfilenames_box->sizeHint());
    onlydirnames_box = new QCheckBox(i18n("Only &directories matching:"), this);
    onlydirnames_box->setMinimumSize(onlydirnames_box->sizeHint());
    user_edit = new KLineEdit(this);
    user_edit->setEnabled(false);
    user_edit->setMinimumSize(user_edit->sizeHint());
    filename_edit = new KLineEdit(this);
    filename_edit->setEnabled(false);
    filename_edit->setMinimumSize(filename_edit->sizeHint());
    dirname_edit = new KLineEdit(this);
    dirname_edit->setEnabled(false);
    dirname_edit->setMinimumSize(dirname_edit->sizeHint());

    connect( onlyuser_box, SIGNAL(toggled(bool)),
             this, SLOT(toggled(bool)) );
    connect( onlyfilenames_box, SIGNAL(toggled(bool)),
             this,  SLOT(toggled(bool)) );
    connect( onlydirnames_box, SIGNAL(toggled(bool)),
             this, SLOT(toggled(bool)) );
    
    connect( commit_box, SIGNAL(toggled(bool)),
             this, SLOT(choiceChanged()) );
    connect( checkout_box, SIGNAL(toggled(bool)),
             this, SLOT(choiceChanged()) );
    connect( tag_box, SIGNAL(toggled(bool)),
             this, SLOT(choiceChanged()) );
    connect( other_box, SIGNAL(toggled(bool)),
             this, SLOT(choiceChanged()) );
    connect( onlyuser_box, SIGNAL(toggled(bool)),
             this, SLOT(choiceChanged()) );
    connect( onlyfilenames_box, SIGNAL(toggled(bool)),
             this, SLOT(choiceChanged()) );
    connect( onlydirnames_box, SIGNAL(toggled(bool)),
             this, SLOT(choiceChanged()) );
    connect( user_edit, SIGNAL(returnPressed()),
             this, SLOT(choiceChanged()) );
    connect( filename_edit, SIGNAL(returnPressed()),
             this, SLOT(choiceChanged()) );
    connect( dirname_edit, SIGNAL(returnPressed()),
             this, SLOT(choiceChanged()) );
            
    QGridLayout *grid = new QGridLayout(4, 4, 10);
    layout->addLayout(grid);
    grid->setColStretch(0, 1);
    grid->setColStretch(1, 0);
    grid->setColStretch(2, 4);
    grid->setColStretch(3, 1);
    grid->addWidget(commit_box,        0, 0);
    grid->addWidget(checkout_box,      1, 0);
    grid->addWidget(tag_box,           2, 0);
    grid->addWidget(other_box,         3, 0);
    grid->addWidget(onlyuser_box,      0, 1);
    grid->addWidget(user_edit,         0, 2);
    grid->addWidget(onlyfilenames_box, 1, 1);
    grid->addWidget(filename_edit,     1, 2);
    grid->addWidget(onlydirnames_box,  2, 1);
    grid->addWidget(dirname_edit,      2, 2);

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    frame->setMinimumHeight(frame->sizeHint().height());
    layout->addSpacing(8);
    layout->addWidget(frame, 0);
    layout->addSpacing(8);

    KButtonBox *buttonbox = new KButtonBox(this);
    buttonbox->addStretch();
    QPushButton *closebutton = buttonbox->addButton(i18n("&Close"));
    closebutton->setAutoDefault(false);
    connect( closebutton, SIGNAL(clicked()), SLOT(reject()) );
    buttonbox->layout();
    buttonbox->setFixedHeight(buttonbox->height());
    layout->addWidget(buttonbox, 0);

    if (options)
        {
            resize(options->size);
        }
}


void HistoryDialog::done(int res)
{
    if (!options)
        options = new Options;
    options->size = size();
    
    QDialog::done(res);
    delete this;
}


void HistoryDialog::loadOptions(KConfig *config)
{
    if (!config->readEntry("Customized"))
        return;

    options = new Options;
    options->size = config->readSizeEntry("Size");
}


void HistoryDialog::saveOptions(KConfig *config)
{
    if (!options)
        return;

    config->writeEntry("Customized", true);
    config->writeEntry("Size", options->size);
}


void HistoryDialog::choiceChanged()
{
    // Collect all items in hidden list
    QListViewItem *shownitem;
    while ( (shownitem = listview->firstChild()) != 0)
        {
            hiddenitems.append(static_cast<HistoryItem*>(shownitem)->itemCopy());
            delete shownitem;
        }
            
    // Now move all items which fulfill our criteria
    // back into the listview.
    QPtrList<ItemCopy> shownitems;
    QPtrListIterator<ItemCopy> it(hiddenitems);
    for (; it.current(); ++it)
        {
            ItemCopy *item = it.current();
            if ( !( commit_box->isChecked() && item->isCommit() ) &&
                 !( checkout_box->isChecked() && item->isCheckout() ) &&
                 !( tag_box->isChecked() && item->isTag() ) &&
                 !( other_box->isChecked() && item->isOther() ) )
                continue;
            if ( onlyuser_box->isChecked() &&
                 !QString(user_edit->text()).isEmpty() &&
                 user_edit->text() != item->text[2] )
                continue;
            if ( onlyfilenames_box->isChecked() &&
                 !QString(filename_edit->text()).isEmpty() &&
                 QRegExp(filename_edit->text(), true, true).match(item->text[4]) != 0 )
                continue;
            if ( onlydirnames_box->isChecked() &&
                 !QString(dirname_edit->text()).isEmpty() &&
                 QRegExp(dirname_edit->text(), true, true).match(item->text[5]) != 0)
                continue;

            (void) new HistoryItem(listview, item);
            shownitems.append(item);
        }

    // Delete shown items from hidden list
    QPtrListIterator<ItemCopy> it2(shownitems);
    for (; it2.current(); ++it2)
        hiddenitems.remove(it2.current());
                           
    listview->triggerUpdate();
}


void HistoryDialog::toggled(bool b)
{
    KLineEdit *edit = 0;

    if (sender() == onlyuser_box)
        edit = user_edit;
    else if (sender() == onlyfilenames_box)
        edit = filename_edit;
    else if (sender() == onlydirnames_box)
        edit = dirname_edit;

    edit->setEnabled(b);
    if (b)
        edit->setFocus();
}


bool HistoryDialog::parseHistory(const QString &sandbox, const QString &repository)
{
    setCaption(i18n("CVS History"));

    QString cmdline = cvsClient( repository ) + " history -e -a";
    
    CvsProgressDialog l("History", this);
    l.setCaption(i18n("CVS History"));
    if (!l.execCommand(sandbox, repository, cmdline, "history"))
        return false;

    QCString line;
    int index = 0;
    while (l.getOneLine(&line) )
        {
            QStringList list = splitLine(line);
            QString cmd = list[0];
            if (cmd.length() != 1)
                continue;

            int ncol;
            int cmd_code = cmd[0].latin1();
            switch (cmd_code)
                {
                case 'O':
                case 'F':
                case 'E': ncol = 8;
                default:  ncol = 10;
                }
            if (ncol != (int)list.count())
                continue;

            QString event;
            switch (cmd_code)
                {
                case 'O': event = i18n("Checkout ");         break;
                case 'T': event = i18n("Tag ");              break;
                case 'F': event = i18n("Release ");          break;
                case 'W': event = i18n("Update, Deleted ");  break;
                case 'U': event = i18n("Update, Copied ");   break;
                case 'G': event = i18n("Update, Merged ");   break;
                case 'C': event = i18n("Update, Conflict "); break;
                case 'M': event = i18n("Commit, Modified "); break;
                case 'A': event = i18n("Commit, Added ");    break;
                case 'R': event = i18n("Commit, Removed ");  break;
                default:  event = i18n("Unknown ");
                }

            HistoryItem *item = new HistoryItem(listview, QString().sprintf("%5d", index));
            item->setText(0, list[1] + " " + list[2] + " " + list[3] + "  ");
            item->setText(1, event);
            item->setText(2, list[4]);
            if (ncol == 10)
                {
                    item->setText(3, list[5]);
                    item->setText(4, list[6]);
                    item->setText(5, list[7]);
                }
            else
                {
                    item->setText(5, list[5]);
                }
            
            ++index;
        }

    return true;
}


// Local Variables:
// c-basic-offset: 4
// End:
