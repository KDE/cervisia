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

#include "cvsplugin.h"
using Cervisia::CvsPlugin;

#include <qfileinfo.h>

#include <dcopref.h>
#include <kapplication.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurl.h>

#include <cvsservice_stub.h>

#include <kdebug.h>


typedef KGenericFactory<CvsPlugin> CvsPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libcvsplugin, CvsPluginFactory( "cvsplugin" ) )


CvsPlugin::CvsPlugin(QObject* parent, const char* name, const QStringList&)
    : PluginBase(parent, name)
    , m_cvsService(0)
{
    kdDebug() << "CvsPlugin::CvsPlugin()" << endl;

    startService();
}


CvsPlugin::~CvsPlugin()
{
    kdDebug() << "CvsPlugin::~CvsPlugin()" << endl;

    // stop the cvs DCOP service and delete reference
    if( m_cvsService )
        m_cvsService->quit();

    delete m_cvsService;
}


bool CvsPlugin::canHandle(const KURL& workingCopy)
{
    const QFileInfo fi(workingCopy.path());

    QString path = fi.absFilePath() + "/CVS";

    // is this really a cvs-controlled directory?
    QFileInfo cvsDirInfo(path);

    return ( cvsDirInfo.exists() &&
             cvsDirInfo.isDir() &&
             QFile::exists(path + "/Entries") &&
             QFile::exists(path + "/Repository") &&
             QFile::exists(path + "/Root") );
}


QString CvsPlugin::type() const
{
    return "CVS";
}


DCOPRef CvsPlugin::service() const
{
    return DCOPRef(m_cvsService->app(), m_cvsService->obj());
}


void CvsPlugin::startService()
{
    // start the cvs DCOP service
    QString error;
    QCString appId;

    if( KApplication::startServiceByDesktopName("cvsservice", QStringList(), &error, &appId) )
    {
        KMessageBox::sorry(0, i18n("Starting cvsservice failed with message: ") +
                error, "Cervisia");
    }
    else
        // create a reference to the service
        m_cvsService = new CvsService_stub(appId, "CvsService");
}
