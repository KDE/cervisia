/* 
 *  Copyright (C) 1999-2001 Bernd Gehrmann
 *                          bernd@physik.hu-berlin.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include <kaboutdata.h>
#include <kapp.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "misc.h"
#include "cervisiashell.h"
#include "version.h"

int main(int argc, char **argv)
{
    static KCmdLineOptions options[] = {
        { "+[directory]", I18N_NOOP("The sandbox to be loaded"), 0 },
        { 0, 0, 0 }
    };
    KAboutData about("cervisia", I18N_NOOP("Cervisia"), 
		     CERVISIA_VERSION, I18N_NOOP("A CVS frontend"),
                     KAboutData::License_QPL, I18N_NOOP("(c) 1999-2002 Bernd Gehrmann"));
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);
    
    KApplication *app = new KApplication();

    if ( app->isRestored() )
    {
        RESTORE(CervisiaShell);
    }
    else
    {
	CervisiaShell *t = new CervisiaShell();

        QString dirname = QString(KCmdLineArgs::parsedArgs()->count()?
                                  KCmdLineArgs::parsedArgs()->arg(0) : "");
        t->resize(t->sizeHint().width(),
        KApplication::desktop()->height()*8/10);

        t->restorePseudo(dirname);
        t->setIcon(app->icon());
        app->setMainWidget(t);
        t->show();
    }
    
    int res = app->exec();
    cleanupTempFiles();
    return res;
}


// Local Variables:
// c-basic-offset: 4
// End:
