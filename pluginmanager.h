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

#ifndef CERVISIA_PLUGINMANAGER_H
#define CERVISIA_PLUGINMANAGER_H

#include <kparts/part.h>
#include <kparts/plugin.h>


namespace Cervisia
{

class PluginBase;


class PluginManager
{
public:
    PluginManager(KParts::Part* part);
    ~PluginManager();

    bool activatePluginForUrl(const KURL& url);

    PluginBase* currentPlugin() const;

private:
    KParts::Part*            m_part;
    PluginBase*              m_currentPlugin;
    QPtrList<KParts::Plugin> m_pluginList;
};


}


#endif
