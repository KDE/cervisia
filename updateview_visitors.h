/*
 * Copyright (c) 2003 André Wöbbeking <Woebbeking@web.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef UPDATEVIEW_VISITORS_H
#define UPDATEVIEW_VISITORS_H


#include "updateview.h"

#include <set>


class UpdateItem;
class UpdateDirItem;
class UpdateFileItem;


class Visitor
{
public:

    virtual ~Visitor() {}

    virtual void preVisit(UpdateDirItem*) = 0;
    virtual void postVisit(UpdateDirItem*) = 0;

    virtual void visit(UpdateFileItem*) = 0;
};


class ApplyFilterVisitor : public Visitor
{
public:

    explicit ApplyFilterVisitor(UpdateView::Filter filter);

    virtual void preVisit(UpdateDirItem*);
    virtual void postVisit(UpdateDirItem*);

    virtual void visit(UpdateFileItem*);

private:

    void markAllParentsAsVisible(UpdateItem*);

    UpdateView::Filter m_filter;

    typedef std::set<UpdateItem*> TItemSet;
    TItemSet m_invisibleDirItems;
};


#endif // UPDATEVIEW_VISITORS_H
