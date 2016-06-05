/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2002-2004 Christian Loose <christian.loose@kdemail.net>
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


#include "cervisiashell.h"

#include <kactioncollection.h>
#include <kconfig.h>
#include <KSharedConfig>
#include <kedittoolbar.h>
#include <khelpmenu.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kmessagebox.h>
#include <kshortcutsdialog.h>
#include <kstandardaction.h>
#include <KLocalizedString>
#include <KAboutData>
#include <KConfigGroup>

#include <QApplication>
#include <QAction>


CervisiaShell::CervisiaShell( const char *name )
  : m_part(0)
{
    setObjectName( name );
    setXMLFile( "cervisiashellui.rc" );

    KPluginLoader loader("cervisiapart5");
    if( KPluginFactory *factory = loader.factory() )
    {
        m_part = factory->create< KParts::ReadOnlyPart >(this);
        if( m_part )
        {
            m_part->setObjectName( "cervisiaview" );
            setCentralWidget(m_part->widget());
        }
    }
    else
    {
        KMessageBox::detailedError(this, i18n("The Cervisia library could not be loaded."),
                                   loader.errorString() +
                                   QLatin1String("\n") + loader.pluginName() +
                                   QLatin1String("\n") + loader.fileName());
        qApp->quit();
        return;
    }

    setupActions();

    //
    // Magic needed for status texts
    //
    createGUI( m_part );

    // enable auto-save of toolbar/menubar/statusbar and window size settings
    // and apply the previously saved settings
    setAutoSaveSettings("MainWindow", true);

    // if the session is restoring, we already read the settings
    if( !qApp->isSessionRestored() )
        readSettings();
}

CervisiaShell::~CervisiaShell()
{
    delete m_part;
}

void CervisiaShell::setupActions()
{
    setStandardToolBarMenuEnabled( true );

    QAction *action = KStandardAction::configureToolbars( this, SLOT(slotConfigureToolBars()),
                                            actionCollection() );
    QString hint = i18n("Allows you to configure the toolbar");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = KStandardAction::keyBindings( this, SLOT(slotConfigureKeys()), actionCollection() );
    hint = i18n("Allows you to customize the keybindings");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = KStandardAction::quit( this, SLOT(close()), actionCollection() );
    hint = i18n("Exits Cervisia");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    setHelpMenuEnabled(true);
}


void CervisiaShell::openURL()
{
    if ( m_part && !m_lastOpenDir.isEmpty() )
        m_part->openUrl( QUrl::fromLocalFile( m_lastOpenDir ) );
}


void CervisiaShell::openURL(const QUrl& url)
{
    if ( m_part )
        m_part->openUrl(url);
}


void CervisiaShell::slotConfigureKeys()
{
    KShortcutsDialog dlg;
    dlg.addCollection(actionCollection());
    if ( m_part )
        dlg.addCollection(m_part->actionCollection());

    dlg.configure();
}

void CervisiaShell::slotConfigureToolBars()
{
    KConfigGroup cg(KSharedConfig::openConfig(), autoSaveGroup());
    saveMainWindowSettings(cg);
    KEditToolBar dlg( factory() );
    connect(&dlg,SIGNAL(newToolbarConfig()),this,SLOT(slotNewToolbarConfig()));
    dlg.exec();
}

void CervisiaShell::slotNewToolbarConfig()
{
    KConfigGroup cg(KSharedConfig::openConfig(), autoSaveGroup());
    applyMainWindowSettings(cg);
}

void CervisiaShell::closeEvent(QCloseEvent *event)
{
    writeSettings();
    KParts::MainWindow::closeEvent(event);
}


void CervisiaShell::readProperties(const KConfigGroup& config)
{
    m_lastOpenDir = config.readPathEntry("Current Directory", QString());

    // if the session is restoring, make sure we open the URL
    // since it's not handled by main()
    if( qApp->isSessionRestored() )
        openURL();
}


void CervisiaShell::saveProperties(KConfigGroup & config)
{
    // Save current working directory (if part was created)
    if ( m_part )
    {
        config.writePathEntry("Current Directory", m_part->url().path());

        // write to disk
        config.sync();
    }
}


void CervisiaShell::readSettings()
{
    KConfigGroup cg(KSharedConfig::openConfig(), "Session");

    readProperties(cg);
}


void CervisiaShell::writeSettings()
{
    KConfigGroup cg(KSharedConfig::openConfig(), "Session");
    saveProperties(cg);
}




// Local Variables:
// c-basic-offset: 4
// End:
