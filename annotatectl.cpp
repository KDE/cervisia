/* 
 *  Copyright (c) 2002 Christian Loose <christian.loose@hamburg.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "annotatectl.h"

#include <qdatetime.h>
#include <qmap.h>

#include <dcopref.h>
#include <klocale.h>
#include <krfcdate.h>

#include "annotatedlg.h"
#include "progressdlg.h"


struct AnnotateController::Private
{
    typedef QMap<QString, QString>  RevisionCommentMap;
    RevisionCommentMap  comments;                           // maps comment to a revision

    DCOPRef*            cvsService;
    AnnotateDialog*     dialog;
    ProgressDialog*     progress;

    bool execute(const QString& fileName, const QString& revision);
    void parseCvsLogOutput();
    void parseCvsAnnotateOutput();
};


AnnotateController::AnnotateController(AnnotateDialog* dialog, DCOPRef* cvsService)
    : d(new Private)
{
    // initialize private data
    d->cvsService = cvsService;
    d->dialog     = dialog;
    d->progress   = 0;
}


AnnotateController::~AnnotateController()
{
    delete d;
}


void AnnotateController::showDialog(const QString& fileName, const QString& revision)
{
    if( !d->execute(fileName, revision) )
    {
        delete d->dialog;
        return;
    }
   
    d->parseCvsLogOutput();
    d->parseCvsAnnotateOutput();

    // hide progress dialog
    delete d->progress; d->progress = 0;
    
    d->dialog->setCaption(i18n("CVS Annotate: %1").arg(fileName));
    d->dialog->show();
}


bool AnnotateController::Private::execute(const QString& fileName, const QString& revision)
{
    DCOPReply job = cvsService->call("annotate(QString, QString)", fileName, revision);

    if( !job.isValid() )
        return false;

    progress = new ProgressDialog(dialog, "Annotate", job, "annotate", i18n("CVS Annotate"));

    return progress->execute();
}


void AnnotateController::Private::parseCvsLogOutput()
{
    QString line, comment, rev;

    enum { Begin, Tags, Admin, Revision, 
           Author, Branches, Comment, Finished } state;

    state = Begin;
    while( progress->getLine(line) )
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
                    comments[rev] = comment;
                break;
            case Finished:
                    ;
        }

        if (state == Finished)
            break;
    }

    // skip 2 lines
    for( int i = 0; i < 2; ++i )
        progress->getLine(line);
}


void AnnotateController::Private::parseCvsAnnotateOutput()
{
    QDate date;
    QString rev, author, content, comment, line;
    QString oldRevision = "";
    bool odd = false;

    while( progress->getLine(line) )
    {
        QDateTime dt;
        dt.setTime_t(KRFCDate::parseDate(line.mid(23, 9)), Qt::UTC);
        date    = dt.date();
        rev     = line.left(13).stripWhiteSpace();
        author  = line.mid(14, 8).stripWhiteSpace();
        content = line.mid(35, line.length()-35);

        comment = comments[rev];
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

        dialog->addLine(rev, author, date, content, comment, odd);
    }
}
