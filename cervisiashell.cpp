/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2002-2003 Christian Loose <christian.loose@hamburg.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "cervisiashell.h"

#include <kapplication.h>
#include <kconfig.h>
#include <kdeversion.h>
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
  , part(0)
{
    setXMLFile( "cervisiashellui.rc" );

    KLibFactory* factory = KLibLoader::self()->factory("libcervisiapart");
    if( factory )
    {
        part = static_cast<KParts::ReadOnlyPart*>(factory->create(this,
                                      "cervisiaview", "KParts::ReadOnlyPart"));
        if( part )
            setCentralWidget(part->widget());
    }
    else
    {
        KMessageBox::error(this, "Could not find our Part!");
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
    part->actionCollection()->setHighlightingEnabled(true);
    connect( part->actionCollection(), SIGNAL( actionStatusText(const QString &) ),
             statusBar(), SLOT( message(const QString &) ) );
    connect( part->actionCollection(), SIGNAL( clearStatusText() ),
             statusBar(), SLOT( clear() ) );

    createGUI( part );

    // enable auto-save of toolbar/menubar/statusbar and window size settings
    // and apply the previously saved settings
    setAutoSaveSettings("MainWindow", true);
    
    readSettings();
}

CervisiaShell::~CervisiaShell()
{
    delete part;
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
    hint = i18n("Opens the Bug report dialog");
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
        part->openURL(KURL::fromPathOrURL(m_lastOpenDir));
}


void CervisiaShell::openURL(const KURL& url)
{
    part->openURL(url);
}


void CervisiaShell::slotConfigureKeys()
{
    KKeyDialog dlg;
    dlg.insert(actionCollection());
    if( part )
        dlg.insert(part->actionCollection());
        
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


void CervisiaShell::readSettings()
{   
    KConfig* config = KGlobal::config();
    
    config->setGroup("Session");
    m_lastOpenDir = config->readPathEntry("Current Directory");
}


void CervisiaShell::writeSettings()
{
    KConfig* config = KGlobal::config();
    
    config->setGroup("Session");

    // Save current working directory (if part was created)
    if( part )
    {
#if KDE_IS_VERSION(3,1,3)
        config->writePathEntry("Current Directory", part->url().path());
#else
        config->writeEntry("Current Directory", part->url().path());
#endif
    
        // write to disk
        config->sync();
    }
}


#include "cervisiashell.moc"


// Local Variables:
// c-basic-offset: 4
// End:
