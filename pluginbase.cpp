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

#include "pluginbase.h"
using Cervisia::PluginBase;


PluginBase::PluginBase(QObject* parent, const char* name)
    : Plugin(parent, name)
{
}


PluginBase::~PluginBase()
{
}

#include "pluginbase.moc"
