/*
 * Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
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

#include <QRegExp>
#include <QApplication>
#include <QCommandLineParser>

#include <kaboutdata.h>
#include <KPasswordDialog>

#include <iostream>



extern "C" KDE_EXPORT int kdemain(int argc, char** argv)
{
    KAboutData about("cvsaskpass", 0, i18n("cvsaskpass"), "0.1",
                     i18n("ssh-askpass for the CVS D-Bus Service"),
                     KAboutData::License_LGPL,
                     i18n("Copyright (c) 2003 Christian Loose"));

    KAboutData::setApplicationData(about);

    // no need to register with the dcop server
    //KApplication::disableAutoDcopRegistration();
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addPositionalArgument(QCommandlineOption(QLatin1String("prompt"), i18n("prompt"), QLatin1String("[prompt]")));

    // no need for session management
    app.disableSessionManagement();

    parser.process(app);

    if( !parser.positionalArguments()->count() )
        return 1;

    // parse repository name from the passed argument
    QString prompt = parser.positionalArguments()[0];
    QRegExp rx("(.*@.*)'s password:");

    KPasswordDialog dlg;
    dlg.setPrompt(i18n("Please type in your password below."));
    //dlg.setWindowTitle(i18n("Enter Password"));

    if( prompt.contains( rx ) )
        dlg.addCommentLine(i18n("Repository:"), rx.cap(1));

    int res = dlg.exec();
    if( res == KPasswordDialog::Accepted )
    {
        std::cout << dlg.password().toAscii().constData() << std::endl;
        return 0;
    }

    return 1;
}
