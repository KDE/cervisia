/*
 * Copyright (c) 2005 Christian Loose <christian.loose@kdemail.net>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef CERVISIA_PLUGINBASE_H
#define CERVISIA_PLUGINBASE_H

#include <dcopref.h>
#include <kparts/plugin.h>


namespace Cervisia
{


class PluginBase : public KParts::Plugin
{
public:
    PluginBase(QObject* parent, const char* name);
    ~PluginBase();

    virtual bool canHandle(const KURL& workingCopy) = 0;
    virtual QString type() const = 0;
    virtual DCOPRef service() const = 0;
};


}


#endif
