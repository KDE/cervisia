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


#ifndef _LISTVIEW_H_
#define _LISTVIEW_H_

#include <qlistview.h>
#include <qarray.h>

class ListView;
class QTimer;


class ListViewItem : public QListViewItem
{
public:
    ListViewItem(ListView *parent);
    ListViewItem(ListViewItem *parent);

    void setVisible(bool b);
    bool visible() const;

    ListViewItem *myFirstChild() const;
    ListViewItem *myNextSibling() const;
    
private:
    ListViewItem *hiddenChild;
    ListViewItem *hiddenSibling;
    ListViewItem *formerParent;
};


class ListView : public QListView
{
    Q_OBJECT

public:
    ListView( QWidget *parent=0, const char *name=0 );
    ~ListView();

    void setColumnConfig(int sortColumn, bool sortAscending,
                         QArray<int> indexToColumn, QArray<int> columnSizes);
    void getColumnConfig(int *sortColumn, bool *sortAscending,
                         QArray<int> *indexToColumn, QArray<int> *columnSizes) const;

    void setPreferredColumn(int i);
    int preferredColumn() const;

    int sortColumn() const
      { return m_sortColumn; }
    bool sortAscending() const
      { return m_sortAscending; }
    
protected:
    virtual void setColumnWidth(int column, int w);
    virtual void resizeEvent(QResizeEvent *e);
    
private slots:
    void headerSizeChange();
    void headerClicked(int column);

private:
    int m_sortColumn;
    bool m_sortAscending;
    int m_preferredColumn;
    QTimer *timer;
    friend class ListViewItem;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
