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
#include <kfind.h>
#include <kfinddialog.h>
#include <kglobal.h>
#include <klocale.h>


LogPlainView::LogPlainView(QWidget* parent, const char* name)
    : KTextBrowser(parent, name)
    , m_find(0)
    , m_findPos(0)
{
    setNotifyClick(false);
}


LogPlainView::~LogPlainView()
{
    delete m_find; m_find = 0;
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
    logEntry += " &nbsp;[<a href=\"revA#" + QStyleSheet::escape(rev) + "\">" + 
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


void LogPlainView::searchText(int options, const QString& pattern)
{
    m_find = new KFind(pattern, options, this);
    
    connect(m_find, SIGNAL(highlight(const QString&, int, int)),
            this, SLOT(searchHighlight(const QString&, int, int)));
    connect(m_find, SIGNAL(findNext()),
           this, SLOT(findNext()));
    
    m_findPos = 0;
    if( options & KFindDialog::FromCursor )
    {
        const QPoint pos(contentsX(), contentsY());
        m_findPos = paragraphAt(pos);
    }

    findNext();
}


void LogPlainView::scrollToTop()
{
    setContentsPos(0, 0);
}


void LogPlainView::findNext()
{   
    KFind::Result res = KFind::NoMatch;
    
    while( res == KFind::NoMatch && m_findPos < paragraphs() && m_findPos >= 0 )
    {
        if( m_find->needData() )
            m_find->setData(text(m_findPos));
   
        res = m_find->find();
        
        if( res == KFind::NoMatch )
        {
            if( m_find->options() & KFindDialog::FindBackwards )
                --m_findPos;
            else
                ++m_findPos;
        }
    }
       
    // reached the end?
    if( res == KFind::NoMatch )
    {
        if( m_find->shouldRestart() )
        {
            m_findPos = 0;
            findNext();
        }
        else
        {
            delete m_find;  
            m_find = 0;
        }
    }
}


void LogPlainView::searchHighlight(const QString& text, int index, int length)
{
    setSelection(m_findPos, index, m_findPos, index + length);
}


void LogPlainView::setSource(const QString& name)
{
    if( name.isEmpty() )
        return;

    const bool selectedRevisionB(name.startsWith("revB#"));
    if( selectedRevisionB || name.startsWith("revA#") )
    {
        emit revisionClicked(name.mid(5), selectedRevisionB);
    }
}

#include "logplainview.moc"
