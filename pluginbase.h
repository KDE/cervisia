/*
 * Copyright (c) 2005 Christian Loose <christian.loose@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef CERVISIA_PLUGINBASE_H
#define CERVISIA_PLUGINBASE_H

#include <dcopref.h>
#include <kparts/plugin.h>

#include "entry.h"


namespace Cervisia
{


class PluginBase : public KParts::Plugin
{
    Q_OBJECT

public:
    PluginBase(QObject* parent, const char* name);
    ~PluginBase();

    virtual bool canHandle(const KURL& workingCopy) = 0;
    virtual QString type() const = 0;
    virtual DCOPRef service() const = 0;

    virtual void syncWithEntries(const QString& path) = 0;

signals:
    void updateItem(const Cervisia::Entry& entry);
};


}


#endif
