/*
 * Copyright (c) 2003-2005 André Wöbbeking <Woebbeking@web.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "updateview_visitors.h"

#include "updateview.h"
#include "updateview_items.h"


using Cervisia::Entry;


ApplyFilterVisitor::ApplyFilterVisitor(UpdateView::Filter filter)
    : m_filter(filter)
{
}


void ApplyFilterVisitor::preVisit(UpdateDirItem* item)
{
    // as QListViewItem::setVisible() is recursive we have to make
    // this UpdateDirItem visible first and later we can make it invisible
    item->setVisible(true);

    // assume that this item is invisible and correct it later
    // (see markAllParentsAsVisible())
    m_invisibleDirItems.insert(item);
}


void ApplyFilterVisitor::postVisit(UpdateDirItem* item)
{
    // a UpdateDirItem is visible if
    // - it has visible children
    // - it is not opened
    // - empty directories are not hidden
    // - it has no parent (top level item)
    const bool visible(!m_invisibleDirItems.count(item)
                       || !item->wasScanned()
                       || !(m_filter & UpdateView::NoEmptyDirectories)
                       || !item->parent());

    // only set invisible as QListViewItem::setVisible() is recursive
    // and so maybe overrides the state applied by the filter
    if (visible)
    {
        markAllParentsAsVisible(item);
    }
    else
    {
        item->setVisible(false);
    }
}


void ApplyFilterVisitor::visit(UpdateFileItem* item)
{
    const bool visible(item->applyFilter(m_filter));
    if (visible)
    {
        markAllParentsAsVisible(item);
    }
}


void ApplyFilterVisitor::markAllParentsAsVisible(UpdateItem* item)
{
    while ((item = static_cast<UpdateDirItem*>(item->parent())))
    {
        TItemSet::iterator it = m_invisibleDirItems.find(item);
        if (it != m_invisibleDirItems.end())
        {
            m_invisibleDirItems.erase(it);
        }
        else
        {
            // if this item isn't in the map anymore all parents
            // are already removed too
            break;
        }
    }
}
