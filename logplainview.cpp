/* 
 *  Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "logplainview.h"

#include <qstringlist.h>
#include <qstylesheet.h>
#include <kglobal.h>
#include <klocale.h>

#include <kdebug.h>

LogPlainView::LogPlainView(QWidget* parent, const char* name)
    : KTextBrowser(parent, name)
{
    setNotifyClick(false);
}


LogPlainView::~LogPlainView()
{
}


void LogPlainView::addRevision(const QString& rev, const QString& author, 
                               const QDateTime& date, const QString& comment, 
                               const QString& tagcomment)
{
    // convert timestamp to string   
    const QString dateStr(KGlobal::locale()->formatDateTime(date));
    
    setTextFormat(QStyleSheet::RichText);

    // assemble revision information lines
    QString logEntry;
    
    logEntry += "<b>" + i18n("revision %1").arg(QStyleSheet::escape(rev)) + 
                "</b>";
    logEntry += " [<a href=\"revA#" + QStyleSheet::escape(rev) + "\">" + 
                i18n("Select for revision A") +
                "</a>]";
    logEntry += " [<a href=\"revB#" + QStyleSheet::escape(rev) + "\">" + 
                i18n("Select for revision B") +
                "</a>]<br>";
    logEntry += "<i>" + 
                i18n("date: %1; author: %2").arg(QStyleSheet::escape(dateStr))
                                            .arg(QStyleSheet::escape(author)) +
                "</i>";

    append(logEntry);
           
    setTextFormat(QStyleSheet::PlainText);
                     
    // split comment in separate lines
    QStringList lines = QStringList::split("\n", comment);
    
    append("\n");
    QStringList::Iterator it  = lines.begin();
    QStringList::Iterator end = lines.end();
    for( ; it != end; ++it )
    {
        append(*it);
    }
    append("\n");

    setTextFormat(QStyleSheet::RichText);
    
    // split tag comment in separate lines
    lines = QStringList::split("\n", tagcomment);
    
    it  = lines.begin();
    end = lines.end();
    for( ; it != end; ++it )
    {
        append("<i>" + QStyleSheet::escape(*it) + "</i>");
    }
    
    // add an empty line when we had tags or branches
    if( lines.begin() != lines.end() )
    {
        setTextFormat(QStyleSheet::PlainText);
        append("\n");
    }

    // add horizontal line    
    setTextFormat(QStyleSheet::RichText);
    append("<hr>");   
}


void LogPlainView::setSource(const QString& name)
{
    if( name.isEmpty() )
        return;

    if( name.contains("revA") )
    {
        QString rev(name);
        emit revisionClicked(rev.remove("revA#"), false);
    }
    else if( name.contains("revB") )
    {
        QString rev(name);
        emit revisionClicked(rev.remove("revB#"), true);
    }
}

#include "logplainview.moc"
