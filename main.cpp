/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2003      Christian Loose <christian.loose@hamburg.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <iostream>

#include <qfileinfo.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <klocale.h>
#include <kurl.h>

#include "misc.h"
#include "cervisiashell.h"
#include "cervisiapart.h"
#include "cvsservice_stub.h"
#include "logdlg.h"
#include "resolvedlg.h"
#include "version.h"


static CvsService_stub* StartDCOPService(const QString& directory)
{
    // start the cvs DCOP service
    QString error;
    QCString appId;
    if( KApplication::startServiceByDesktopName("cvsservice", QStringList(),
                                                &error, &appId) )
    {
        std::cerr << "Starting cvsservice failed with message: " 
                  << error.latin1() << std::endl;
        exit(1);
    }

    DCOPRef repository(appId, "CvsRepository");

    repository.call("setWorkingCopy(QString)", directory);

    // create a reference to the service
    return new CvsService_stub(appId, "CvsService");
}


static int ShowResolveDialog(const QString& fileName)
{
    KConfig* config = CervisiaFactory::instance()->config();
    ResolveDialog* dlg = new ResolveDialog(*config);
    kapp->setMainWidget(dlg);
    if( dlg->parseFile(fileName) )
        dlg->show();
    else
        delete dlg;

    int result = kapp->exec();

    delete CervisiaFactory::instance();

    return result;
}


static int ShowLogDialog(const QString& fileName)
{
    KConfig* config = CervisiaFactory::instance()->config();
    LogDialog* dlg = new LogDialog(*config);
    kapp->setMainWidget(dlg);

    // get directory for file
    const QFileInfo fi(fileName);
    QString directory = fi.dirPath(true);

    // start the cvs DCOP service
    CvsService_stub* cvsService = StartDCOPService(directory);

    if( dlg->parseCvsLog(cvsService, fi.fileName()) )
        dlg->show();
    else
        delete dlg;

    int result = kapp->exec();

    // stop the cvs DCOP service
    cvsService->quit();
    delete cvsService;

    delete CervisiaFactory::instance();

    return result;
}


int main(int argc, char **argv)
{
    static KCmdLineOptions options[] = {
        { "+[directory]", I18N_NOOP("The sandbox to be loaded"), 0 },
        { "resolve <file>", I18N_NOOP("Show resolve dialog for the given file"), 0 },
        { "log <file>", I18N_NOOP("Show log dialog for the given file"), 0 },
        { 0, 0, 0 }
    };
    KAboutData about("cervisia", I18N_NOOP("Cervisia"), CERVISIA_VERSION, 
                     I18N_NOOP("A CVS frontend"), KAboutData::License_QPL, 
                     I18N_NOOP("Copyright (c) 1999-2002 Bernd Gehrmann"), 0,
                     "http://www.kde.org/apps/cervisia");

    about.addAuthor("Bernd Gehrmann", I18N_NOOP("Original author and former "
                    "maintainer"), "bernd@mail.berlios.de", 0);
    about.addAuthor("Christian Loose", I18N_NOOP("Maintainer"),
                    "christian.loose@hamburg.de", 0);
    about.addAuthor("Andr\303\251 W\303\266bbeking", I18N_NOOP("Developer"),
                    "woebbeking@web.de", 0);

    about.addCredit("Richard Moore", I18N_NOOP("Conversion to KPart"),
                    "rich@kde.org", 0);

    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);
    
    KApplication app;

    QString resolvefile = KCmdLineArgs::parsedArgs()->getOption("resolve");
    if (!resolvefile.isEmpty())
        return ShowResolveDialog(resolvefile);

    // is command line option 'show log dialog' specified?
    QString logFile = KCmdLineArgs::parsedArgs()->getOption("log");
    if( !logFile.isEmpty() )
        return ShowLogDialog(logFile);

    if ( app.isRestored() ) {
        RESTORE(CervisiaShell);
    } else {
        CervisiaShell* shell = new CervisiaShell();
        
        const KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
        QString dirName = args->count() ? QString(args->arg(0)) : QString::null;       

        if( !dirName.isEmpty() )
            shell->openURL(KURL::fromPathOrURL(dirName));
            
        shell->setIcon(app.icon());
        app.setMainWidget(shell);
        shell->show();
    }
    
    int res = app.exec();
    cleanupTempFiles();
    return res;
}


// Local Variables:
// c-basic-offset: 4
// End:
