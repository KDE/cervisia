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


#include "changelogdlg.h"

#include <qfile.h>
#include <qfileinfo.h>
#include <qlayout.h>
#include <qmultilinedit.h>
#include <qpushbutton.h>
#include <qtextstream.h>
#include <kapplication.h>
#include <kbuttonbox.h>
#include <kconfig.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "misc.h"
#include "cervisiapart.h"


static QString dateStringISO8601()
{
    return QDate::currentDate().toString(Qt::ISODate);
}


ChangeLogDialog::Options *ChangeLogDialog::options = 0;


ChangeLogDialog::ChangeLogDialog(QWidget *parent, const char *name)
    : QDialog(parent, name, true,
              WStyle_Customize|WStyle_NormalBorder|WStyle_Title|WStyle_MinMax)
{
    setCaption(i18n("Edit ChangeLog"));
    
    QBoxLayout *layout = new QVBoxLayout(this, 10);

    edit = new QMultiLineEdit(this);
    QFontMetrics fm(fontMetrics());
    edit->setMinimumSize(fm.width("0123456789")*8,
                         fm.lineSpacing()*20);
    edit->setFont(KGlobalSettings::fixedFont());
    edit->setFocus();
    layout->addWidget(edit, 10);

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(frame, 0);

    KButtonBox *buttonbox = new KButtonBox(this);
    buttonbox->addStretch();
    QPushButton *ok = buttonbox->addButton(i18n("&OK"));
    QPushButton *cancel = buttonbox->addButton(i18n("Cancel"));
    connect( ok, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancel, SIGNAL(clicked()), this, SLOT(reject()) );
    buttonbox->layout();
    layout->addWidget(buttonbox, 0);

    layout->activate();
    resize(sizeHint());

    if (options)
        resize(options->size);
}


void ChangeLogDialog::done(int res)
{
    if (res == Accepted)
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
            for(int i = 0 ; i < edit->numLines(); ++i)
                stream << edit->textLine(i) << '\n';
            f.close();
        }
    if (!options)
        options = new Options;
    options->size = size();
    
    QDialog::done(res);
}


void ChangeLogDialog::loadOptions(KConfig *config)
{
    if (!config->readEntry("Customized"))
        return;

    options = new Options;
    options->size = config->readSizeEntry("Size");
}


void ChangeLogDialog::saveOptions(KConfig *config)
{
    if (!options)
        return;

    config->writeEntry("Customized", true);
    config->writeEntry("Size", options->size);
}


bool ChangeLogDialog::readFile(const QString &filename)
{
    fname = filename;
    QFileInfo fi(filename);
    bool exists = QFileInfo(filename).exists();
    if (!exists)
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
            while (!stream.eof())
                {
                    QString str = stream.readLine();

                    // preserve empty lines
                    if (str.isEmpty() && !stream.eof())
                        str = "\n";

                    edit->append(str);
                }
            f.close();
        }

    KConfig *config = CervisiaPart::config();
    config->setGroup("General");
    edit->insertLine("", 0);
    edit->insertLine("\t* ", 0);
    edit->insertLine("", 0);
    edit->insertLine(dateStringISO8601() + "  " + config->readEntry("Username", userName()), 0);
    edit->setCursorPosition(2, 10);
    
    return true;
}


QString ChangeLogDialog::message()
{
    int no = 0;
    // Find first line which begins with non-whitespace
    while (no < edit->numLines())
        {
            QString str = edit->textLine(no);
            if (!str.isEmpty() && !str[0].isSpace())
                break;
            ++no;
        }
    ++no;
    // Skip empty lines
    while (no < edit->numLines())
        {
            if (edit->textLine(no).isEmpty())
                break;
            ++no;
        }
    QString res;
    // Use all lines until one which begins with non-whitespace
    // Remove tabs or 8 whitespace at beginning of each line
    while (no < edit->numLines())
        {
            QString str = edit->textLine(no);
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

#include "changelogdlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
