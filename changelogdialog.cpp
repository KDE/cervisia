/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2002-2007 Christian Loose <christian.loose@hamburg.de>
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


#include "changelogdialog.h"

#include <QDate>
#include <qfile.h>
#include <qtextstream.h>

#include <KSharedConfig>
#include <kmessagebox.h>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QPlainTextEdit>
#include <QTextBlock>
#include <QScrollBar>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "cervisiasettings.h"
#include "misc.h"


ChangeLogDialog::Options *ChangeLogDialog::options = 0;


ChangeLogDialog::ChangeLogDialog(KConfig& cfg, QWidget *parent)
    : QDialog(parent)
    , partConfig(cfg)
{
    setWindowTitle(i18n("Edit ChangeLog"));
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    edit = new QPlainTextEdit(this);
    edit->setFont(CervisiaSettings::changeLogFont());
    edit->setFocus();
    edit->setLineWrapMode(QPlainTextEdit::NoWrap);
    QFontMetrics const fm(edit->fontMetrics());
    edit->setMinimumSize(fm.width('0') * 80, fm.lineSpacing() * 20);

    mainLayout->addWidget(edit);
    mainLayout->addWidget(buttonBox);

    KConfigGroup cg(&partConfig, "ChangeLogDialog");
    restoreGeometry(cg.readEntry<QByteArray>("geometry", QByteArray()));
    connect(okButton,SIGNAL(clicked()),this,SLOT(slotOk()));
}


ChangeLogDialog::~ChangeLogDialog()
{
    KConfigGroup cg(&partConfig, "ChangeLogDialog");
    cg.writeEntry("geometry", saveGeometry());
}


void ChangeLogDialog::slotOk()
{
    // Write changelog
    QFile f(fname);
    if (!f.open(QIODevice::ReadWrite))
    {
        KMessageBox::sorry(this,
                           i18n("The ChangeLog file could not be written."),
                           "Cervisia");
        return;
    }

    QTextStream stream(&f);
    stream << edit->toPlainText();
    f.close();

    QDialog::accept();
}


bool ChangeLogDialog::readFile(const QString &filename)
{
    fname = filename;

    if (!QFile::exists(filename))
    {
        if (KMessageBox::warningContinueCancel(this,
                                     i18n("A ChangeLog file does not exist. Create one?"),
                                     i18n("Create")) != KMessageBox::Continue)
            return false;
    }
    else
    {
        QFile f(filename);
        if (!f.open(QIODevice::ReadWrite))
        {
            KMessageBox::sorry(this,
                               i18n("The ChangeLog file could not be read."),
                               "Cervisia");
            return false;
        }
        QTextStream stream(&f);
        edit->setPlainText(stream.readAll());
        f.close();
    }

    KConfigGroup cs(&partConfig, "General");
    const QString username = cs.readEntry("Username", Cervisia::UserName());

    edit->insertPlainText(QDate::currentDate().toString(Qt::ISODate) + "  " + username + "\n\n\t* \n\n");
    QTextCursor cursor = edit->textCursor();
    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 2);
    edit->setTextCursor(cursor);

    edit->verticalScrollBar()->setValue(0);  // show top line

    return true;
}


QString ChangeLogDialog::message()
{
     int no = 0;
     // Find first line which begins with non-whitespace
     while (no < edit->document()->lineCount())
     {
       QString str = edit->document()->findBlockByLineNumber(no).text();

       if ( !str.isEmpty() && !str[0].isSpace() )
         break;

       ++no;
     }
     ++no;
     // Skip empty lines
     while (no < edit->document()->lineCount())
     {
       QString str = edit->document()->findBlockByLineNumber(no).text();

       if ( str.isEmpty() || str == " " )
         break;

       ++no;
     }
     QString res;
     // Use all lines until one which begins with non-whitespace
     // Remove tabs or 8 whitespace at beginning of each line
     while (no < edit->document()->lineCount())
     {
         QString str = edit->document()->findBlockByLineNumber(no).text();

         if (!str.isEmpty() && !str[0].isSpace())
             break;
         if (!str.isEmpty() && str[0] == '\t')
             str.remove(0, 1);
         else
             {
                 int j;
                 for (j = 0; j < (int)str.length(); ++j)
                     if (!str[j].isSpace())
                         break;
                 str.remove(0, qMin(j, 8));
             }
         res += str;
         res += '\n';
         ++no;
     }
     // Remove newlines at end
     int l;
     for (l = res.length()-1; l > 0; --l)
         if (res[l] != '\n')
             break;
     res.truncate(l+1);
     return res;
}


// Local Variables:
// c-basic-offset: 4
// End:
