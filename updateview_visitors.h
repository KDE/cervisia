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


class UpdateDirItem;
class UpdateFileItem;


class Visitor
{
public:

    virtual ~Visitor() {}

    virtual void visit(UpdateDirItem*) = 0;

    virtual void visit(UpdateFileItem*) = 0;
};


#endif // UPDATEVIEW_VISITORS_H
