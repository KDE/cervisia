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


#include "annotatedlg.h"

#include <dcopref.h>
#include <kconfig.h>
#include <klocale.h>
#include <kprocess.h>
#include <krfcdate.h>

#include "annotateview.h"
#include "cvsprogressdlg.h"
#include "misc.h"
#include "progressdlg.h"


AnnotateDialog::Options *AnnotateDialog::options = 0;


AnnotateDialog::AnnotateDialog(QWidget *parent, const char *name)
    : KDialogBase(parent, name, false, QString::null,
                  Close | Help, Close, true)
{
    annotate = new AnnotateView(this);
    setMainWidget(annotate);

    setHelp("annotate");

    setWFlags(Qt::WDestructiveClose | getWFlags());

    if (options)
        resize(options->size);
}


AnnotateDialog::~AnnotateDialog()
{
    if (!options)
        options = new Options;
    options->size = size();
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
    QString rev, author, comment, content;
    QDate date;
    QMap<QString, QString> logmap;
    enum { Begin, Tags, Admin, Revision,
	   Author, Branches, Comment, Finished } state;

    setCaption(i18n("CVS Annotate: %1").arg(filename));

    QString cmdline = "( ";
    cmdline += cvsClient(repository);
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
    cmdline += " ) 2>&1";

    CvsProgressDialog l("Annotate", this);
    l.setCaption(i18n("CVS Annotate"));
    if (!l.execCommand(sandbox, repository, cmdline, "annotate"))
        return false;

    //
    // First the log output...
    //

    state = Begin;
    QString line;
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
            QDateTime d;
            d.setTime_t(KRFCDate::parseDate(line.mid(23, 9)), Qt::UTC);
            date = d.date();
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


bool AnnotateDialog::parseCvsAnnotate(DCOPRef& cvsService, const QString& fileName,
                                      const QString& revision)
{
    QMap<QString, QString> logmap;                      // maps comment to a revision
    enum { Begin, Tags, Admin, Revision,
        Author, Branches, Comment, Finished } state;

    setCaption(i18n("CVS Annotate: %1").arg(fileName));

    DCOPReply job = cvsService.call("annotate(QString, QString)", fileName,
                                    revision);
    if( !job.isValid() )
        return false;

    ProgressDialog dlg(this, "Annotate", job, "annotate", i18n("CVS Annotate"));
    if( !dlg.execute() )
        return false;

    // process cvs log output
    state = Begin;
    QString line, comment, rev;
    while( dlg.getLine(line) )
    {
        switch( state )
        {
            case Begin:
                if( line == "symbolic names:" )
                    state = Tags;
                break;
            case Tags:
                if( line[0] != '\t' )
                    state = Admin;
                break;
            case Admin:
                if( line == "----------------------------" )
                    state = Revision;
                break;
            case Revision:
                rev = line.section(' ', 1, 1);
                state = Author;
                break;
            case Author:
                state = Branches;
                break;
            case Branches:
                if( !line.startsWith("branches:") )
                {
                    state = Comment;
                    comment = line;
                }
                break;
            case Comment:
                if( line == "----------------------------" )
                    state = Revision;
                else if( line == "=============================================================================" )
                    state = Finished;
                if( state == Comment )
                    comment += QString("\n") + line;
                else
                    logmap[rev] = comment;
                break;
            case Finished:
                    ;
        }

        if (state == Finished)
            break;
    }

    // skip 2 lines
    for( int i = 0; i < 2; ++i )
        dlg.getLine(line);


    // process cvs annotate output
    QString author, content;
    QDate date;
    bool odd = false;
    QString oldRevision = "";
    while( dlg. getLine(line) )
    {
        rev = line.left(13).stripWhiteSpace();
        comment = logmap[rev];
        author = line.mid(14, 8).stripWhiteSpace();
        QDateTime d;
        d.setTime_t(KRFCDate::parseDate(line.mid(23, 9)), Qt::UTC);
        date = d.date();
        content = line.mid(35, line.length()-35);
        if( comment.isNull() )
            comment = "";

        if( rev == oldRevision )
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

    return true;
}


// Local Variables:
// c-basic-offset: 4
// End:
