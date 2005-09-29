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

#include <kapplication.h>
#include <kconfig.h>
#include <kedittoolbar.h>
#include <khelpmenu.h>
#include <kkeydialog.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <kstdaction.h>
#include <kurl.h>


CervisiaShell::CervisiaShell( const char *name )
  : KParts::MainWindow( name )
  , m_part(0)
{
    setXMLFile( "cervisiashellui.rc" );

    KLibFactory* factory = KLibLoader::self()->factory("libcervisiapart");
    if( factory )
    {
        m_part = static_cast<KParts::ReadOnlyPart*>(factory->create(this,
                                      "cervisiaview", "KParts::ReadOnlyPart"));
        if( m_part )
            setCentralWidget(m_part->widget());
    }
    else
    {
        KMessageBox::detailedError(this, i18n("The Cervisia library could not be loaded."), 
                                   KLibLoader::self()->lastErrorMessage());
        kapp->quit();
        return;
    }
    
    setupActions();
    
    //
    // Magic needed for status texts
    //
    actionCollection()->setHighlightingEnabled(true);
    connect( actionCollection(), SIGNAL( actionStatusText(const QString &) ),
             statusBar(), SLOT( message(const QString &) ) );
    connect( actionCollection(), SIGNAL( clearStatusText() ),
             statusBar(), SLOT( clear() ) );
    m_part->actionCollection()->setHighlightingEnabled(true);
    connect( m_part->actionCollection(), SIGNAL( actionStatusText(const QString &) ),
             statusBar(), SLOT( message(const QString &) ) );
    connect( m_part->actionCollection(), SIGNAL( clearStatusText() ),
             statusBar(), SLOT( clear() ) );

    createGUI( m_part );

    // enable auto-save of toolbar/menubar/statusbar and window size settings
    // and apply the previously saved settings
    setAutoSaveSettings("MainWindow", true);
    
    // if the session is restoring, we already read the settings
    if( !kapp->isRestored() )
        readSettings();
}

CervisiaShell::~CervisiaShell()
{
    delete m_part;
}

void CervisiaShell::setupActions()
{
    setStandardToolBarMenuEnabled( true );

    KAction *action = KStdAction::configureToolbars( this, SLOT(slotConfigureToolBars()),
                                            actionCollection() );
    QString hint = i18n("Allows you to configure the toolbar");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = KStdAction::keyBindings( this, SLOT(slotConfigureKeys()),
                                      actionCollection() );
    hint = i18n("Allows you to customize the keybindings");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = KStdAction::quit( kapp, SLOT( quit() ), actionCollection() );
    hint = i18n("Exits Cervisia");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    setHelpMenuEnabled(false);
    (void) new KHelpMenu(this, instance()->aboutData(), false, actionCollection());

    action = actionCollection()->action("help_contents");
    hint = i18n("Invokes the KDE help system with the Cervisia documentation");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = actionCollection()->action("help_report_bug");
    hint = i18n("Opens the bug report dialog");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = actionCollection()->action("help_about_app");
    hint = i18n("Displays the version number and copyright information");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = actionCollection()->action("help_about_kde");
    hint = i18n("Displays the information about KDE and its version number");
    action->setToolTip( hint );
    action->setWhatsThis( hint );
}


void CervisiaShell::openURL()
{
    if( !m_lastOpenDir.isEmpty() )
        m_part->openURL(KURL::fromPathOrURL(m_lastOpenDir));
}


void CervisiaShell::openURL(const KURL& url)
{
    m_part->openURL(url);
}


void CervisiaShell::slotConfigureKeys()
{
    KKeyDialog dlg;
    dlg.insert(actionCollection());
    if( m_part )
        dlg.insert(m_part->actionCollection());
        
    dlg.configure();
}

void CervisiaShell::slotConfigureToolBars()
{
    saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
    KEditToolbar dlg( factory() );
    connect(&dlg,SIGNAL(newToolbarConfig()),this,SLOT(slotNewToolbarConfig()));
    dlg.exec();
}

void CervisiaShell::slotNewToolbarConfig()
{
    applyMainWindowSettings( KGlobal::config(), autoSaveGroup() );
}

bool CervisiaShell::queryExit()
{
    writeSettings();
    return true;
}


void CervisiaShell::readProperties(KConfig* config)
{   
    m_lastOpenDir = config->readPathEntry("Current Directory");
    
    // if the session is restoring, make sure we open the URL 
    // since it's not handled by main()
    if( kapp->isRestored() )
        openURL();
}


void CervisiaShell::saveProperties(KConfig* config)
{
    // Save current working directory (if part was created)
    if( m_part )
    {
        config->writePathEntry("Current Directory", m_part->url().path());
    
        // write to disk
        config->sync();
    }
}


void CervisiaShell::readSettings()
{
    KConfig* config = KGlobal::config(); 
    config->setGroup("Session");
    
    readProperties(config);
}


void CervisiaShell::writeSettings()
{
    KConfig* config = KGlobal::config();  
    config->setGroup("Session");
    
    saveProperties(config);
}


#include "cervisiashell.moc"


// Local Variables:
// c-basic-offset: 4
// End:
