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


#include <qheader.h>
#include <qtimer.h>

#include "misc.h"

#include "listview.h"
#include "listview.moc"


ListViewItem::ListViewItem(ListView *parent)
    : QListViewItem(parent)
{
    hiddenChild = 0;
    hiddenSibling = 0;
    formerParent = 0;
}


ListViewItem::ListViewItem(ListViewItem *parent)
    : QListViewItem(parent)
{
    hiddenChild = 0;
    hiddenSibling = 0;
    formerParent = 0;
}


ListViewItem *ListViewItem::myFirstChild() const
{
    if (!visible())
        return 0;
    
    if (hiddenChild)
        return hiddenChild;

    // Cast guaranteed by ListViewItem's constructor
    // (provided no other QListViewItems are constructed)
    return static_cast<ListViewItem*>(firstChild());
}


ListViewItem *ListViewItem::myNextSibling() const
{
    if (visible())
        return static_cast<ListViewItem*>(nextSibling());

    if (hiddenSibling)
        return hiddenSibling;

    // We're the last hidden item of our parent
    return static_cast<ListViewItem*>(formerParent->firstChild());
}


void ListViewItem::setVisible(bool b)
{
    if (visible() == b)
        return;

    if (b)
        {
            ListViewItem *item = formerParent->hiddenChild;
            if (item == this)
                formerParent->hiddenChild = item->hiddenSibling;
            else
                {
                    // Loop should be finite...
                    while (item->hiddenSibling != this)
                        item = item->hiddenSibling;
                    item->hiddenSibling = this->hiddenSibling;
                }
            formerParent->insertItem(this);
            formerParent = 0;
        }
    else
        {
            // Cast guaranteed by constructor
            ListViewItem *p = static_cast<ListViewItem*>(parent());
            ListViewItem *item = p->hiddenChild;
            this->hiddenSibling = item;
            p->hiddenChild = this;
            formerParent = p;
            p->takeItem(this);
        }
}


bool ListViewItem::visible() const
{
    return formerParent == 0;
}


ListView::ListView(QWidget *parent, const char *name)
    : QListView(parent, name)
{
    m_preferredColumn = -1;
    m_sortColumn = 0;
    m_sortAscending = true;

    timer = new QTimer(this);
    connect( timer, SIGNAL(timeout()), this, SLOT(headerSizeChange()) );
    
    connect( header(), SIGNAL(clicked(int)), this, SLOT(headerClicked(int)) );
    connect( header(), SIGNAL(sizeChange(int,int,int)), this, SLOT(headerSizeChange()) );
}


ListView::~ListView()
{}


void ListView::setColumnConfig(int sortColumn, bool sortAscending,
                               QArray<int> indexToColumn, QArray<int> columnSizes)
{
    m_sortColumn = sortColumn;
    m_sortAscending = sortAscending;
    setSorting(sortColumn, sortAscending);
    int n = QMIN(QMIN((int)indexToColumn.count(),
                      (int)columnSizes.count()),
                 header()->count());
    for (int i=0; i<n; ++i)
        {
            header()->moveSection(indexToColumn.at(i), i);
            header()->resizeSection(i, columnSizes.at(i));
        }
}


void ListView::getColumnConfig(int *sortColumn, bool *sortAscending,
                               QArray<int> *indexToColumn, QArray<int> *columnSizes) const
{
    *sortColumn = m_sortColumn;
    *sortAscending = m_sortAscending;

    int n = header()->count();
    indexToColumn->resize(n);
    columnSizes->resize(n);
    for (int i=0; i<n; ++i)
        {
            indexToColumn->at(i) = header()->mapToSection(i);
            columnSizes->at(i) = header()->sectionSize(i);
        }
}


void ListView::setPreferredColumn(int i)
{
    m_preferredColumn = i;
    header()->setResizeEnabled(false);
    setHScrollBarMode(AlwaysOff);
}


int ListView::preferredColumn() const
{
    return m_preferredColumn;
}


void ListView::setColumnWidth(int column, int w)
{
    QListView::setColumnWidth(column, w);
    timer->start(20, true);
}


void ListView::resizeEvent(QResizeEvent *e)
{
    QListView::resizeEvent(e);
    timer->start(20, true);
}


void ListView::headerSizeChange()
{
    if (m_preferredColumn != -1)
        {
            int w = 0;
            for (int i = 0; i < header()->count(); ++i)
                if (i != preferredColumn())
                    w += header()->sectionSize(i);
            //            int newsize = visibleWidth() - w;
            int newsize = viewportSize(0, contentsHeight()).width() - w;
            if (newsize > 20)
                {
                    header()->resizeSection(preferredColumn(), newsize);
                    viewport()->update();
                    //                    header()->repaint(false);
                    //                    viewport()->repaint(false);
                }
        }
}


void ListView::headerClicked(int column)
{
    m_sortAscending = (column == m_sortColumn)? !m_sortAscending : true;
    m_sortColumn = column;
}


// Local Variables:
// c-basic-offset: 4
// End:
