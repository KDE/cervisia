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

}

void CervisiaShell::setupActions()
{
    setHelpMenuEnabled(false);
    (void) new KHelpMenu(this, instance()->aboutData(), false, actionCollection());

    KAction* action = KStdAction::showToolbar( 0, 0, actionCollection() );
    connect( action, SIGNAL(toggled(bool)),
             this,   SLOT(slotToggleToolbar( bool )) );

    action = KStdAction::configureToolbars( this, SLOT(slotConfigureToolBars()),
                                            actionCollection() );
    action = KStdAction::keyBindings( this, SLOT(slotConfigureKeys()),
                                      actionCollection() );

    (void) KStdAction::quit( this, SLOT( slotExit() ), actionCollection() );
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

    part->saveDialogProperties( config );
    config->setGroup( "Session" );
    part->saveProperties( config );

    config->sync();
    return true;
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
    part->saveProperties( config );
    config->writeEntry( "Current Directory", part->sandBox() );
}

void CervisiaShell::restorePseudo( const QString &dirname )
{
    KConfig *config = part->config();

    config->setGroup("Main window");
    if (config->readBoolEntry("Customized"))
        resize(config->readSizeEntry("Size"));

    part->readDialogProperties( config );

    config->setGroup("Session");
    if (!dirname.isEmpty())
        config->writeEntry("Current Directory", dirname);
    readProperties(config);
}

#include "cervisiashell.moc"


// Local Variables:
// c-basic-offset: 4
// End:
