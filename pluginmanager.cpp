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

#include "pluginmanager.h"
using Cervisia::PluginManager;

#include <kdebug.h>
#include <kstaticdeleter.h>
#include <kxmlguifactory.h>

#include "pluginbase.h"


PluginManager* PluginManager::m_self = 0;
static KStaticDeleter<PluginManager> staticDeleter;

PluginManager* PluginManager::self()
{
    if( !m_self )
        staticDeleter.setObject(m_self, new PluginManager());

    return m_self;
}


PluginManager::PluginManager()
    : m_part(0)
    , m_currentPlugin(0)
{
    kdDebug() << "PluginManager::PluginManager()" << endl;
}


PluginManager::~PluginManager()
{
    kdDebug() << "PluginManager::~PluginManager()" << endl;
}


void PluginManager::setPart(KParts::Part* part)
{
    m_part = part;

    // get the list of all KPart plugins
    m_pluginList = KParts::Plugin::pluginObjects(m_part);

    // remove all plugins from main menu
    if( m_part->factory() )
    {
        KParts::Plugin* plugin = m_pluginList.first();
        for( ; plugin; plugin = m_pluginList.next() )
        {
            m_part->factory()->removeClient(plugin);
        }
    }
}


bool PluginManager::activatePluginForUrl(const KURL& url)
{
    kdDebug() << "PluginManager::activatePluginForUrl(): url = " << url.prettyURL() << endl;

    bool activated = false;

    Cervisia::PluginBase* plugin = static_cast<Cervisia::PluginBase*>(m_pluginList.first());
    for( ; plugin; plugin = static_cast<Cervisia::PluginBase*>(m_pluginList.next()) )
    {
        if( plugin->canHandle(url) )
        {
            activated = true;

            // is the plugin already active? --> no need to change the menu
            if( m_currentPlugin && m_currentPlugin->type() == plugin->type() )
                break;

            if( m_currentPlugin )
                m_part->factory()->removeClient(m_currentPlugin);

            m_part->factory()->addClient(plugin);
            m_currentPlugin = plugin;
            break;
        }
    }

    return activated;
}


Cervisia::PluginBase* PluginManager::currentPlugin() const
{
    return m_currentPlugin;
}
