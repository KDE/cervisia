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


void CvsPlugin::syncWithEntries(const QString& path)
{
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
