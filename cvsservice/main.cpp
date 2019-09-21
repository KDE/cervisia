/*
 * Copyright (c) 2002 Christian Loose <christian.loose@hamburg.de>
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include <QApplication>

#include <kaboutdata.h>
#include <KLocalizedString>

#include "cvsservice.h"
#include "../cervisia_version.h"


extern "C" Q_DECL_EXPORT int kdemain(int argc, char** argv)
{
    KLocalizedString::setApplicationDomain("cvsservice");

    QApplication app(argc, argv);

    KAboutData about("cvsservice5", i18n("CVS D-Bus service"), CERVISIA_VERSION_STRING,
                     i18n("D-Bus service for CVS"), KAboutLicense::LGPL,
                     i18n("Copyright (c) 2002-2003 Christian Loose"));

    about.setOrganizationDomain("kde.org");

    about.addAuthor(i18n("Christian Loose"), i18n("Developer"),
                    "christian.loose@hamburg.de");

    KAboutData::setApplicationData(about);

    // Don't quit if password dialog for login is closed
    app.setQuitOnLastWindowClosed(false);

    CvsService service;

    return app.exec();
}
