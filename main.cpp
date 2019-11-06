/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2003-2008 Christian Loose <christian.loose@kdemail.net>
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
#include <QApplication>
#include <QCommandLineParser>
#include <kconfig.h>
#include <ktoolinvocation.h>
#include <KLocalizedString>

#include "misc.h"
#include "cervisiashell.h"
#include "cvsserviceinterface.h"
#include "annotatedialog.h"
#include "annotatecontroller.h"
#include "logdialog.h"
#include "resolvedialog.h"
#include "cervisia_version.h"
#include "repositoryinterface.h"

static OrgKdeCervisia5CvsserviceCvsserviceInterface* StartDBusService(const QString& directory)
{
    // start the cvs D-Bus service
    QString error;
    QString appId;
    if ( KToolInvocation::startServiceByDesktopName("org.kde.cvsservice5", QStringList(), &error, &appId) )
    {
        std::cerr << "Starting cvsservice failed with message: "
                  << error.toLocal8Bit().constData() << std::endl;
        exit(1);
    }

    OrgKdeCervisia5RepositoryInterface repository(appId, "/CvsRepository", QDBusConnection::sessionBus());

    repository.setWorkingCopy(directory);

    // create a reference to the service
    return new OrgKdeCervisia5CvsserviceCvsserviceInterface(appId, "/CvsService", QDBusConnection::sessionBus());
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
    OrgKdeCervisia5CvsserviceCvsserviceInterface* cvsService = StartDBusService(directory);

    if( dlg->parseCvsLog(cvsService, fi.fileName()) )
        dlg->show();
    else
        delete dlg;

    int result = qApp->exec();

    // stop the cvs D-Bus service
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
    OrgKdeCervisia5CvsserviceCvsserviceInterface* cvsService = StartDBusService(directory);

    AnnotateController ctl(dlg, cvsService);
    ctl.showDialog(fi.fileName());

    int result = qApp->exec();

    // stop the cvs D-Bus service
    cvsService->quit();
    delete cvsService;

    delete config;

    return result;
}


extern "C" Q_DECL_EXPORT int kdemain(int argc, char **argv)
{
    KLocalizedString::setApplicationDomain("cervisia");

    QApplication app(argc, argv);

    KAboutData about("cervisia", i18n("Cervisia"), CERVISIA_VERSION_STRING,
                     i18n("A CVS frontend"), KAboutLicense::GPL,
                     i18n("Copyright (c) 1999-2002 Bernd Gehrmann\n"
                          "Copyright (c) 2002-2008 the Cervisia authors"), QString(),
                     QLatin1String("http://cervisia.kde.org"));

    about.addAuthor(i18n("Bernd Gehrmann"), i18n("Original author and former "
                    "maintainer"), "bernd@mail.berlios.de");
    about.addAuthor(i18n("Christian Loose"), i18n("Maintainer"),
                    "christian.loose@kdemail.net");
    about.addAuthor(i18n("Andr\303\251 W\303\266bbeking"), i18n("Developer"),
                    "woebbeking@kde.org");
    about.addAuthor(i18n("Carlos Woelz"), i18n("Documentation"),
                    "carloswoelz@imap-mail.com");

    about.addCredit(i18n("Richard Moore"), i18n("Conversion to KPart"),
                    "rich@kde.org");
    about.addCredit(i18n("Laurent Montel"), i18n("Conversion to D-Bus"),
                    "montel@kde.org");
    about.addCredit(i18n("Martin Koller"), i18n("Port to KDE Frameworks 5"),
                    "kollix@aon.at");

    about.setOrganizationDomain(QByteArray("kde.org"));

    KAboutData::setApplicationData(about);

    QCommandLineParser parser;
    about.setupCommandLine(&parser);

    parser.addPositionalArgument(QLatin1String("directory"), i18n("The sandbox to be loaded"), QLatin1String("[directory]"));
    parser.addOption(QCommandLineOption(QLatin1String("resolve"), i18n("Show resolve dialog for the given file."), QLatin1String("file")));
    parser.addOption(QCommandLineOption(QLatin1String("log"), i18n("Show log dialog for the given file."), QLatin1String("file")));
    parser.addOption(QCommandLineOption(QLatin1String("annotate"), i18n("Show annotation dialog for the given file."), QLatin1String("file")));
    parser.addVersionOption();
    parser.addHelpOption();

    parser.process(app);
    about.processCommandLine(&parser);

    QString resolvefile = parser.value(QLatin1String("resolve"));
    if (!resolvefile.isEmpty())
        return ShowResolveDialog(resolvefile);

    // is command line option 'show log dialog' specified?
    QString logFile = parser.value(QLatin1String("log"));
    if( !logFile.isEmpty() )
        return ShowLogDialog(logFile);

    // is command line option 'show annotation dialog' specified?
    QString annotateFile = parser.value(QLatin1String("annotate"));
    if( !annotateFile.isEmpty() )
        return ShowAnnotateDialog(annotateFile);

    if ( app.isSessionRestored() ) {
        kRestoreMainWindows<CervisiaShell>();
    } else {
        CervisiaShell* shell = new CervisiaShell();

        if( parser.positionalArguments().count() )
        {
            QDir dir(parser.positionalArguments()[0]);
            QUrl directory = QUrl::fromLocalFile(dir.absolutePath());
            shell->openURL(directory);
        }
        else
            shell->openURL();

        shell->setWindowIcon(qApp->windowIcon());
        shell->show();
    }

    int res = app.exec();
    cleanupTempFiles();
    return res;
}


// Local Variables:
// c-basic-offset: 4
// End:
