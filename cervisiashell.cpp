#include "cervisiashell.h"

#include <kapplication.h>
#include <kconfig.h>
#include <kedittoolbar.h>
#include <kfiledialog.h>
#include <khelpmenu.h>
#include <kkeydialog.h>
#include <klocale.h>
#include <kstdaction.h>
#include <kstatusbar.h>

#include "cervisiapart.h"


CervisiaShell::CervisiaShell( const char *name )
  : KParts::MainWindow( name )
{
    setXMLFile( "cervisiashellui.rc" );

    part = new CervisiaPart( this, "cervisiaview", this, "cervisiapart" );
    setCentralWidget( part->widget() );

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
}

CervisiaShell::~CervisiaShell()
{
    delete part;
}

void CervisiaShell::setupActions()
{
    KAction *action = KStdAction::showToolbar( 0, 0, actionCollection() );
    QString hint = i18n("Shows or hides the toolbar");
    action->setToolTip( hint );
    action->setWhatsThis( hint );
    connect( action, SIGNAL(toggled(bool)),
             this,   SLOT(slotToggleToolbar( bool )) );

    action = KStdAction::configureToolbars( this, SLOT(slotConfigureToolBars()),
                                            actionCollection() );
    hint = i18n("Allows you to configure the toolbar");
    action->setToolTip( hint );
    action->setWhatsThis( hint );
    
    action = KStdAction::keyBindings( this, SLOT(slotConfigureKeys()),
                                      actionCollection() );
    hint = i18n("Allows you to customize the keybindings");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = KStdAction::quit( this, SLOT( slotExit() ), actionCollection() );
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

void CervisiaShell::slotOpenSandbox()
{
    QString dirname = KFileDialog::getExistingDirectory(QDir::homeDirPath(), this,
                                                        i18n("Open Sandbox"));
    if (dirname.isEmpty())
        return;

    part->openSandbox(dirname);
}

void CervisiaShell::slotToggleToolbar( bool visible )
{
    KToolBar *tb = toolBar( "mainToolBar" );
    if ( visible )
        tb->show();
    else
        tb->hide();
}

void CervisiaShell::slotConfigureKeys()
{
    KKeyDialog::configureKeys( actionCollection(), xmlFile() );
}

void CervisiaShell::slotConfigureToolBars()
{
    KEditToolbar dlg( actionCollection() );
    if ( dlg.exec() )
        createGUI( part );
}

void CervisiaShell::slotExit()
{
    (void) queryExit();
    kapp->quit();
}

bool CervisiaShell::queryExit()
{
    KConfig *config = part->config();

    config->setGroup("Main window");
    config->writeEntry("Customized", true);
    config->writeEntry("Size", size());

    part->saveDialogProperties(config);

    config->setGroup("Session");
    saveProperties(config);

    config->sync();
    return true;
}

void CervisiaShell::restorePseudo(const QString &dirname)
{
    KConfig *config = part->config();

    config->setGroup("Main window");
    if (config->readBoolEntry("Customized"))
        resize(config->readSizeEntry("Size"));

    part->readDialogProperties(config);

    config->setGroup("Session");
    // Explicitly override the loaded directory if
    // a command line argument is given
    if (!dirname.isEmpty())
        config->writeEntry("Current Directory", dirname);
    readProperties(config);
}

void CervisiaShell::readProperties(KConfig *config)
{
    part->readProperties( config );

    QString currentDir = config->readEntry("Current Directory");
    if (!currentDir.isEmpty())
        part->openSandbox(currentDir);
}

void CervisiaShell::saveProperties(KConfig *config)
{
    part->saveProperties(config);
    config->writeEntry("Current Directory", part->sandBox());
}

#include "cervisiashell.moc"


// Local Variables:
// c-basic-offset: 4
// End:
