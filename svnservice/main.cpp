/*
 * Copyright (c) 2005 Christian Loose <christian.loose@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include "svnservice.h"


int main(int argc, char** argv)
{
    KAboutData about("svnservice", I18N_NOOP("Subversion DCOP service"), "0.1",
            I18N_NOOP("DCOP service for Subversion"), KAboutData::License_LGPL,
            "Copyright (c) 2005 Christian Loose");
    about.addAuthor("Christian Loose", I18N_NOOP("Developer"),
            "christian.loose@kdemail.net");

    KCmdLineArgs::init(argc, argv, &about);

    KApplication app;

    // This app is started automatically, no need for session management
    app.disableSessionManagement();

    SvnService service;

    return app.exec();
}
