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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include <qregexp.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kpassdlg.h>

#include <iostream>


static KCmdLineOptions options[] =
{
    { "+[prompt]", I18N_NOOP("prompt"), 0 },
    KCmdLineLastOption
};


extern "C" KDE_EXPORT int kdemain(int argc, char** argv)
{
    KAboutData about("cvsaskpass", I18N_NOOP("cvsaskpass"), "0.1",
                     I18N_NOOP("ssh-askpass for the CVS DCOP Service"),
                     KAboutData::License_LGPL,
                     I18N_NOOP("Copyright (c) 2003 Christian Loose"));

    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);

    // no need to register with the dcop server
    KApplication::disableAutoDcopRegistration();
    KApplication app;

    // no need for session management
    app.disableSessionManagement();

    if( !KCmdLineArgs::parsedArgs()->count() )
        return 1;

    // parse repository name from the passed argument
    QString prompt = KCmdLineArgs::parsedArgs()->arg(0);
    QRegExp rx("(.*@.*)'s password:");
    int pos = rx.search(prompt);

    KPasswordDialog dlg(KPasswordDialog::Password, false, 0);
    dlg.setPrompt(i18n("Please type in your password below."));

    if( pos > -1 )
        dlg.addLine(i18n("Repository:"), rx.cap(1));

    int res = dlg.exec();
    if( res == KPasswordDialog::Accepted )
    {
        std::cout << dlg.password() << std::endl;
        return 0;
    }

    return 1;
}
