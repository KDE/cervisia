/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2003-2004 Christian Loose <christian.loose@hamburg.de>
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
#include "cvsservice_stub.h"
#include "annotatedlg.h"
#include "annotatectl.h"
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
    KConfig* config = new KConfig("cervisiapartrc");

    ResolveDialog* dlg = new ResolveDialog(*config);
    kapp->setMainWidget(dlg);
    if( dlg->parseFile(fileName) )
        dlg->show();
    else
        delete dlg;

    int result = kapp->exec();

    delete config;

    return result;
}


static int ShowLogDialog(const QString& fileName)
{
    KConfig* config = new KConfig("cervisiapartrc");
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

    delete config;

    return result;
}


static int ShowAnnotateDialog(const QString& fileName)
{
    KConfig* config = new KConfig("cervisiapartrc");
    AnnotateDialog* dlg = new AnnotateDialog(*config);
    kapp->setMainWidget(dlg);

    // get directory for file
    const QFileInfo fi(fileName);
    QString directory = fi.dirPath(true);

    // start the cvs DCOP service
    CvsService_stub* cvsService = StartDCOPService(directory);

    AnnotateController ctl(dlg, cvsService);
    ctl.showDialog(fi.fileName());

    int result = kapp->exec();

    // stop the cvs DCOP service
    cvsService->quit();
    delete cvsService;

    delete config;

    return result;
}


int main(int argc, char **argv)
{
    static KCmdLineOptions options[] = {
        { "+[directory]", I18N_NOOP("The sandbox to be loaded"), 0 },
        { "resolve <file>", I18N_NOOP("Show resolve dialog for the given file"), 0 },
        { "log <file>", I18N_NOOP("Show log dialog for the given file"), 0 },
        { "annotate <file>", I18N_NOOP("Show annotation dialog for the given file"), 0 },
        KCmdLineLastOption
    };
    KAboutData about("cervisia", I18N_NOOP("Cervisia"), CERVISIA_VERSION,
                     I18N_NOOP("A CVS frontend"), KAboutData::License_QPL,
                     I18N_NOOP("Copyright (c) 1999-2002 Bernd Gehrmann\n"
                               "Copyright (c) 2002-2004 the Cervisia authors"), 0,
                     "http://www.kde.org/apps/cervisia");

    about.addAuthor("Bernd Gehrmann", I18N_NOOP("Original author and former "
                    "maintainer"), "bernd@mail.berlios.de", 0);
    about.addAuthor("Christian Loose", I18N_NOOP("Maintainer"),
                    "christian.loose@kdemail.net", 0);
    about.addAuthor("Andr\303\251 W\303\266bbeking", I18N_NOOP("Developer"),
                    "woebbeking@web.de", 0);
    about.addAuthor("Carlos Woelz", I18N_NOOP("Documentation"),
                    "carloswoelz@imap-mail.com", 0);

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

    // is command line option 'show annotation dialog' specified?
    QString annotateFile = KCmdLineArgs::parsedArgs()->getOption("annotate");
    if( !annotateFile.isEmpty() )
        return ShowAnnotateDialog(annotateFile);

    if ( app.isRestored() ) {
        RESTORE(CervisiaShell);
    } else {
        CervisiaShell* shell = new CervisiaShell();

        const KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
        if( args->count() )
        {
            KURL directory = args->url(0);
            shell->openURL(directory);
        }
        else
            shell->openURL();

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
