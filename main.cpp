/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2003-2004 Christian Loose <christian.loose@hamburg.de>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <iostream>

#include <QFileInfo>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <klocale.h>
#include <kurl.h>
#include <ktoolinvocation.h>
#include <kiconloader.h>

#include "misc.h"
#include "cervisiashell.h"
#include "cvsserviceinterface.h"
#include "annotatedialog.h"
#include "annotatectl.h"
#include "logdialog.h"
#include "resolvedialog.h"
#include "version.h"
#include "repositoryinterface.h"

static OrgKdeCervisiaCvsserviceCvsserviceInterface* StartDBusService(const QString& directory)
{
    // start the cvs D-Bus service
    QString error;
    QString appId;
    if( KToolInvocation::startServiceByDesktopName("cvsservice", QStringList(),
                                                &error,&appId)  )
    {
        std::cerr << "Starting cvsservice failed with message: "
                  << error.latin1() << std::endl;
        exit(1);
    }

    OrgKdeCervisiaRepositoryInterface repository(appId, "/CvsRepository",QDBusConnection::sessionBus());
    

    repository.setWorkingCopy(directory);

    // create a reference to the service
    return new OrgKdeCervisiaCvsserviceCvsserviceInterface(appId, "/CvsService",QDBusConnection::sessionBus());
}


static int ShowResolveDialog(const QString& fileName)
{
    KConfig* config = new KConfig("cervisiapartrc");

    ResolveDialog* dlg = new ResolveDialog(*config);
    if( dlg->parseFile(fileName) )
        dlg->show();
    else
        delete dlg;

    int result = qApp->exec();

    delete config;

    return result;
}


static int ShowLogDialog(const QString& fileName)
{
    KConfig* config = new KConfig("cervisiapartrc");
    LogDialog* dlg = new LogDialog(*config);

    // get directory for file
    const QFileInfo fi(fileName);
    QString directory = fi.absolutePath();

    // start the cvs DCOP service
    OrgKdeCervisiaCvsserviceCvsserviceInterface* cvsService = StartDBusService(directory);

    if( dlg->parseCvsLog(cvsService, fi.fileName()) )
        dlg->show();
    else
        delete dlg;

    int result = qApp->exec();

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

    // get directory for file
    const QFileInfo fi(fileName);
    QString directory = fi.absolutePath();

    // start the cvs D-Bus service
    OrgKdeCervisiaCvsserviceCvsserviceInterface* cvsService = StartDBusService(directory);

    AnnotateController ctl(dlg, cvsService);
    ctl.showDialog(fi.fileName());

    int result = qApp->exec();

    // stop the cvs D-Bus service
    cvsService->quit();
    delete cvsService;

    delete config;

    return result;
}


extern "C" KDE_EXPORT int kdemain(int argc, char **argv)
{
    static KCmdLineOptions options[] = {
        { "+[directory]", I18N_NOOP("The sandbox to be loaded"), 0 },
        { "resolve <file>", I18N_NOOP("Show resolve dialog for the given file"), 0 },
        { "log <file>", I18N_NOOP("Show log dialog for the given file"), 0 },
        { "annotate <file>", I18N_NOOP("Show annotation dialog for the given file"), 0 },
        KCmdLineLastOption
    };
    KAboutData about("cervisia", I18N_NOOP("Cervisia"), CERVISIA_VERSION,
                     I18N_NOOP("A CVS frontend"), KAboutData::License_GPL,
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
    about.addCredit("Laurent Montel", I18N_NOOP("Conversion to D-Bus"),
                    "montel@kde.org", 0);

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

    if ( app.isSessionRestored() ) {
        RESTORE(CervisiaShell);
    } else {
        CervisiaShell* shell = new CervisiaShell();

        const KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
        if( args->count() )
        {
            KUrl directory = args->url(0);
            shell->openURL(directory);
        }
        else
            shell->openURL();

        shell->setIcon(qApp->windowIcon().pixmap(IconSize(K3Icon::Desktop),IconSize(K3Icon::Desktop)));
        shell->show();
    }

    int res = app.exec();
    cleanupTempFiles();
    return res;
}


// Local Variables:
// c-basic-offset: 4
// End:
