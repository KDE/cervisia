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
#include <qfileinfo.h>
#include <qframe.h>
#include <qlayout.h>
#include <kapp.h>
#include <kbuttonbox.h>
#include <kconfig.h>
#include <klocale.h>
#include <kprocess.h>

#include "annotateview.h"
#include "cvsprogressdlg.h"
#include "misc.h"

#include "annotatedlg.h"
#include "annotatedlg.moc"


AnnotateDialog::Options *AnnotateDialog::options = 0;


AnnotateDialog::AnnotateDialog(QWidget *parent, const char *name)
    : QDialog(parent, name, false,
              WStyle_Customize|WStyle_NormalBorder|WStyle_Title|WStyle_MinMax)
{
    QBoxLayout *layout = new QVBoxLayout(this, 10);

    annotate = new AnnotateView(this);
    layout->addWidget(annotate, 10);

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    frame->setMinimumHeight(frame->sizeHint().height());
    layout->addWidget(frame, 0);

    KButtonBox *buttonbox = new KButtonBox(this);
    buttonbox->addStretch();
    connect( buttonbox->addButton(i18n("&Close")), SIGNAL(clicked()),
	     SLOT(reject()) );
    buttonbox->layout();
    layout->addWidget(buttonbox, 0);

    QFontMetrics fm(fontMetrics());
    setMinimumSize(fm.width("0123456789")*12,
		   fm.lineSpacing()*30);
    layout->activate();

    if (options)
        resize(options->size);
}


void AnnotateDialog::done(int res)
{
    if (!options)
        options = new Options;
    options->size = size();

    QDialog::done(res);
    delete this;
}


void AnnotateDialog::loadOptions(KConfig *config)
{
    if (!config->readEntry("Customized"))
        return;

    options = new Options;
    options->size = config->readSizeEntry("Size");
}


void AnnotateDialog::saveOptions(KConfig *config)
{
    if (!options)
        return;

    config->writeEntry("Customized", true);
    config->writeEntry("Size", options->size);
}


bool AnnotateDialog::parseCvsAnnotate(const QString &sandbox, const QString &repository,
                                      const QString &filename, const QString &annorev)
{
    QStringList strlist;
    QString rev, date, author, comment, content;
    QMap<QString, QString> logmap;
    enum { Begin, Tags, Admin, Revision,
	   Author, Branches, Comment, Finished } state;

    setCaption("CVS Annotate: " + filename);

    QString cmdline = cvsClient(repository);
    cmdline += " log ";
    cmdline += KShellProcess::quote(filename);
    cmdline += " && ";
    cmdline += cvsClient(repository);
    cmdline += " annotate ";
    
    if (!annorev.isEmpty())
	{
	    cmdline += " -r ";
	    cmdline += annorev;
	}
    cmdline += " ";
    cmdline += KShellProcess::quote(filename);
    // Hack because the string ´Annotations for blabla´ is
    // printed to stderr even with option -Q. Arg!
    cmdline += " 2>&1";

    CvsProgressDialog l("Annotate", this);
    l.setCaption("CVS Annotate");
    if (!l.execCommand(sandbox, repository, cmdline, "annotate"))
        return false;

    //
    // First the log output...
    //

    state = Begin;
    QCString line;
    while (l.getOneLine(&line))
        {
            switch (state)
                {
                case Begin:
                    if (line == "symbolic names:")
                        state = Tags;
                    break;
                case Tags:
                    if (line[0] != '\t')
                        state = Admin;
                    break;
                case Admin:
                    if (line == "----------------------------")
                        state = Revision;
                    break;
                case Revision:
                    strlist = splitLine(line);
                    rev = strlist[1];
                    state = Author;
                    break;
                case Author:
                    state = Branches;
                    break;
                case Branches:
                    if (line.left(9) != "branches:")
                        {
                            state = Comment;
                            comment = line;
                        }
                    break;
                case Comment:
                    if (line == "----------------------------")
                        state = Revision;
                    else if (line == "=============================================================================")
                        state = Finished;
                    if (state == Comment)
                        comment += QString("\n") + QString(line);
                    else
                        logmap[rev] = comment;
                    break;
                case Finished:
                    ;
                }
            if (state == Finished)
                break;
        }

    //
    // ... then the annotate output.
    //
    
    for (int i = 0; i < 2; ++i)
        l.getOneLine(&line);

    bool odd = false;
    QString oldRevision = "";
    while ( l.getOneLine(&line) )
        {
	    rev = line.left(13).stripWhiteSpace();
            comment = logmap[rev];
	    author = line.mid(14, 8).stripWhiteSpace();
	    date = line.mid(23, 9);
	    content = line.mid(35, line.length()-35);
            if (comment.isNull())
                comment = "";
            if (rev == oldRevision)
                {
                    author = QString::null;
                    rev = QString::null;
                }
            else
                {
                    oldRevision = rev;
                    odd = !odd;
                }
	    annotate->addLine(rev, author, date, content, comment, odd);
	}

    return true; // successful
}


// Local Variables:
// c-basic-offset: 4
// End:
