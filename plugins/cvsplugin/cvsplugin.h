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

#ifndef CERVISIA_CVSPLUGIN_H
#define CERVISIA_CVSPLUGIN_H

#include <pluginbase.h>

class CvsService_stub;


namespace Cervisia
{

class CvsPlugin : public PluginBase
{
public:
    CvsPlugin(QObject* parent, const char* name, const QStringList&);
    ~CvsPlugin();

    virtual bool canHandle(const KURL& workingCopy);
    virtual QString type() const;
    virtual DCOPRef service() const;

private:
    void startService();

    CvsService_stub* m_cvsService;
};


}


#endif
