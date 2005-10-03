/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2002-2003 Christian Loose <christian.loose@hamburg.de>
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


#include "changelogdlg.h"

#include <qfile.h>
#include <qtextstream.h>
#include <kconfig.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktextedit.h>
#include "misc.h"


static inline QString DateStringISO8601()
{
    return QDate::currentDate().toString(Qt::ISODate);
}


ChangeLogDialog::Options *ChangeLogDialog::options = 0;


ChangeLogDialog::ChangeLogDialog(KConfig& cfg, QWidget *parent, const char *name)
    : KDialogBase(parent, name, true, i18n("Edit ChangeLog"),
                  Ok | Cancel, Ok, true)
    , partConfig(cfg)
{
    edit = new KTextEdit(this);

    cfg.setGroup("LookAndFeel");
    edit->setFont(cfg.readFontEntry("ChangeLogFont"));

    edit->setFocus();
    edit->setWordWrap(QTextEdit::NoWrap);
    edit->setTextFormat(QTextEdit::PlainText);
    edit->setCheckSpellingEnabled(true);
    QFontMetrics const fm(edit->fontMetrics());
    edit->setMinimumSize(fm.width('0') * 80,
                         fm.lineSpacing() * 20);

    setMainWidget(edit);

    QSize size = configDialogSize(partConfig, "ChangeLogDialog");
    resize(size);
}


ChangeLogDialog::~ChangeLogDialog()
{
    saveDialogSize(partConfig, "ChangeLogDialog");
}


void ChangeLogDialog::slotOk()
{
    // Write changelog
    QFile f(fname);
    if (!f.open(IO_ReadWrite))
    {
        KMessageBox::sorry(this,
                           i18n("The ChangeLog file could not be written."),
                           "Cervisia");
        return;
    }

    QTextStream stream(&f);
    stream << edit->text();
    f.close();

    KDialogBase::slotOk();
}


bool ChangeLogDialog::readFile(const QString &filename)
{
    fname = filename;

    if (!QFile::exists(filename))
        {
            if (KMessageBox::warningContinueCancel(this,
                                         i18n("A ChangeLog file does not exist. Create one?"),
                                         "Cervisia",
                                         i18n("Create")) != KMessageBox::Continue)
                return false;
        }
    else
        {
            QFile f(filename);
            if (!f.open(IO_ReadWrite))
                {
                    KMessageBox::sorry(this,
                                       i18n("The ChangeLog file could not be read."),
                                       "Cervisia");
                    return false;
                }
            QTextStream stream(&f);
            edit->setText(stream.read());
            f.close();
        }

    KConfigGroupSaver cs(&partConfig, "General");
    const QString username = partConfig.readEntry("Username", Cervisia::UserName());

    edit->insertParagraph("", 0);
    edit->insertParagraph("\t* ", 0);
    edit->insertParagraph("", 0);
    edit->insertParagraph(DateStringISO8601() + "  " + username, 0);
    edit->setCursorPosition(2, 10);

    return true;
}


QString ChangeLogDialog::message()
{
    int no = 0;
    // Find first line which begins with non-whitespace
    while (no < edit->lines())
        {
            QString str = edit->text(no);
            if (!str.isEmpty() && !str[0].isSpace())
                break;
            ++no;
        }
    ++no;
    // Skip empty lines
    while (no < edit->lines())
        {
            QString str = edit->text(no);
            if( str.isEmpty() || str == " " )
                break;
            ++no;
        }
    QString res;
    // Use all lines until one which begins with non-whitespace
    // Remove tabs or 8 whitespace at beginning of each line
    while (no < edit->lines())
        {
            QString str = edit->text(no);
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
                    str.remove(0, QMIN(j, 8));
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
