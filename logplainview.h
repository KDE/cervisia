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


#ifndef LOGPLAINVIEW_H
#define LOGPLAINVIEW_H

#include <ktextbrowser.h>

class QDateTime;
class KConfig;
class KFind;


class LogPlainView : public KTextBrowser
{   
    Q_OBJECT

public:
    explicit LogPlainView(QWidget* parent = 0, const char* name = 0);
    ~LogPlainView();

    void addRevision(const QString& rev, const QString& author, 
                     const QDateTime& date, const QString& comment, 
                     const QString& tagcomment);            

    void searchText(int options, const QString& pattern);

signals:
    void revisionClicked(QString rev, bool rmb);

public slots:
    void scrollToTop();
    void findNext();
    void searchHighlight(const QString& text, int index, int length);

protected:                     
    virtual void setSource(const QString& name);
    
private:
    KFind* m_find;
    int    m_findPos;
};

#endif
