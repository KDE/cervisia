/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qtextstream.h>
#include <qcursor.h>
#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <kinstance.h>
#include <klocale.h>
#include <kprocess.h>
#include <kstdaction.h>
#include <kxmlguifactory.h>
#include <krun.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kglobal.h>

#include "logdlg.h"
#include "loglist.h"
#include "diffdlg.h"
#include "resolvedlg.h"
#include "annotatedlg.h"
#include "annotatectl.h"
#include "commitdlg.h"
#include "updatedlg.h"
#include "checkoutdlg.h"
#include "tagdlg.h"
#include "mergedlg.h"
#include "historydlg.h"
#include "updateview.h"
#include "protocolview.h"
#include "cvsprogressdlg.h"
#include "repositorydlg.h"
#include "settingsdlg.h"
#include "changelogdlg.h"
#include "misc.h"
#include "cvsservice_stub.h"

#include "cervisiapart.h"
#include "version.h"
#include "cervisiapart.moc"

#define COMMIT_SPLIT_CHAR '\r'

K_EXPORT_COMPONENT_FACTORY( libcervisiapart, CervisiaFactory );

CervisiaPart::CervisiaPart( QWidget *parentWidget, const char *widgetName,
                            QObject *parent, const char *name, const QStringList& /*args*/ )
    : KParts::ReadOnlyPart( parent, name )
    , hasRunningJob( false )
    , opt_hideFiles( false )
    , opt_hideUpToDate( false )
    , opt_hideRemoved( false )
    , opt_hideNotInCVS( false )
    , opt_createDirs( false )
    , opt_pruneDirs( false )
    , opt_updateRecursive( true )
    , opt_commitRecursive( true )
    , opt_doCVSEdit( false )
    , recent( 0 )
{
    KGlobal::locale()->insertCatalogue("cervisia");

    hasRunningJob = false;
    setInstance( CervisiaFactory::instance() );
    new CervisiaBrowserExtension( this );

    // start the cvs DCOP service
    QString error;
    QCString appId;
    if( KApplication::startServiceByName("CvsService", QStringList(), &error, &appId) )
    {
        KMessageBox::sorry(0, "Starting cvsservice failed with message: " +
            error, "Cervisia");
        return;
    }
    
    // create a reference to the service
    cvsService = new CvsService_stub(appId, "CvsService");

    // Create UI
    KConfig *conf = config();
    conf->setGroup("LookAndFeel");
    bool splitHorz = conf->readBoolEntry("SplitHorizontally",true);

    splitter = new QSplitter(splitHorz? QSplitter::Vertical : QSplitter::Horizontal,
                             parentWidget, widgetName);

    update = new UpdateView(splitter);
    update->setFocusPolicy( QWidget::StrongFocus );
    update->setFocus();
    connect( update, SIGNAL(contextMenu()),
             this, SLOT(popupRequested()) );
    connect( update, SIGNAL(fileOpened(QString)),
             this, SLOT(openFile(QString)) );

    protocol = new ProtocolView(appId, splitter);
    protocol->setFocusPolicy( QWidget::StrongFocus );

    setWidget(splitter);
    setupActions();
    connect( update, SIGNAL( selectionChanged() ), this, SLOT( updateActions() ) );
    updateActions();
    setXMLFile( "cervisiaui.rc" );
    
}

CervisiaPart::~CervisiaPart()
{
    // stop the cvs DCOP service and delete reference
    cvsService->quit();
    delete cvsService;
}

KConfig *CervisiaPart::config()
{
    return CervisiaFactory::instance()->config();
}

bool CervisiaPart::openURL( const KURL &u )
{
    openSandbox( u.path() );
    return true;
}

void CervisiaPart::setupActions()
{
    KAction *action;
    QString hint;

    actionCollection()->setHighlightingEnabled(true);

    //
    // File Menu
    //
    action = new KAction( i18n("O&pen Sandbox..."), "fileopen", 0,
                          this, SLOT( slotOpenSandbox() ),
                          actionCollection(), "file_open" );
    hint = i18n("Opens a CVS working directory in the main window");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    recent = new KRecentFilesAction( i18n("Recent Sandboxes"), 0,
                                     this, SLOT( openURL( const KURL & ) ),
                                     actionCollection(), "file_open_recent" );

    action = new KAction( i18n("&Insert ChangeLog Entry..."), 0,
                          this, SLOT( slotChangeLog() ),
                          actionCollection(), "insert_changelog_entry" );
    hint = i18n("Inserts a new intro into the file ChangeLog in the toplevel directory");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Update"), "down", CTRL+Key_U,
                          this, SLOT( slotUpdate() ),
                          actionCollection(), "file_update" );
    hint = i18n("Updates (cvs update) the selected files and directories");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Status"), Key_F5,
                          this, SLOT( slotStatus() ),
                          actionCollection(), "file_status" );
    hint = i18n("Updates the status (cvs -n update) of the selected files and directories");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Edit"), 0,
                          this, SLOT( slotOpen() ),
                          actionCollection(), "file_edit" );
    hint = i18n("Opens the marked file for editing");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("Reso&lve..."), 0,
                          this, SLOT( slotResolve() ),
                          actionCollection(), "file_resolve" );
    hint = i18n("Opens the resolve dialog with the selected file");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Commit..."), "up", Key_NumberSign,
                          this, SLOT( slotCommit() ),
                          actionCollection(), "file_commit" );
    hint = i18n("Commits the selected files");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Add to Repository..."), Key_Plus,
                          this, SLOT( slotAdd() ),
                          actionCollection(), "file_add" );
    hint = i18n("Adds (cvs add) the selected files to the repository");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("Add &Binary..."), 0,
                          this, SLOT( slotAddBinary() ),
                          actionCollection(), "file_add_binary" );
    hint = i18n("Adds (cvs -kb add) the selected files as binaries to the repository");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Remove From Repository"), Key_Minus,
                          this, SLOT( slotRemove() ),
                          actionCollection(), "file_remove" );
    hint = i18n("Removes (cvs remove) the selected files from the repository");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("Rever&t"), 0,
                          this, SLOT( slotRevert() ),
                          actionCollection(), "file_revert_local_changes" );
    hint = i18n("Reverts (cvs update -C) the selected files (only cvs 1.11)");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    //
    // View Menu
    //
    action = new KAction( i18n("Stop"), "stop", Key_Escape,
                          protocol, SLOT(cancelJob()),
                          actionCollection(), "stop_job" );
    action->setEnabled( false );
    hint = i18n("Stops any running sub-processes");
    action->setToolTip( hint );
    action->setWhatsThis( hint );


    action = new KAction( i18n("Browse &Log..."), CTRL+Key_L,
                          this, SLOT(slotBrowseLog()),
                          actionCollection(), "view_log" );
    hint = i18n("Shows the revision tree of the selected file");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

#if 0
    action = new KAction( i18n("Browse Multi-File Log..."), 0,
                          this, SLOT(slotBrowseMultiLog()),
                          actionCollection() );
#endif
    action = new KAction( i18n("&Annotate..."), CTRL+Key_A,
                          this, SLOT(slotAnnotate()),
                          actionCollection(), "view_annotate" );
    hint = i18n("Shows a blame-annotated view of the selected file");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Difference to Repository..."), CTRL+Key_D,
                          this, SLOT(slotDiff()),
                          actionCollection(), "view_diff" );
    hint = i18n("Shows the differences of the selected file to the BASE version");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("Last &Change..."), 0,
                          this, SLOT(slotLastChange()),
                          actionCollection(), "view_last_change" );
    hint = i18n("Shows the differences between the last two revisions of the selected file");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&History..."), 0,
                          this, SLOT(slotHistory()),
                          actionCollection(), "view_history" );
    hint = i18n("Shows the CVS history as reported by the server");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Unfold File Tree"), 0,
                          this , SLOT(slotUnfoldTree()),
                          actionCollection(), "view_unfold_tree" );

    hint = i18n("Opens all branches of the file tree");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Fold File Tree"), 0,
                          this, SLOT(slotFoldTree()),
                          actionCollection(), "view_fold_tree" );
    hint = i18n("Closes all branches of the file tree");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    //
    // Advanced Menu
    //
    action = new KAction( i18n("&Tag/Branch..."), 0,
                          this, SLOT(slotCreateTag()),
                          actionCollection(), "create_tag" );
    hint = i18n("Creates a tag or branch for the selected files");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Delete Tag..."), 0,
                          this, SLOT(slotDeleteTag()),
                          actionCollection(), "delete_tag" );
    hint = i18n("Deletes a tag from the selected files");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Update to Tag/Date..."), 0,
                          this, SLOT(slotUpdateToTag()),
                          actionCollection(), "update_to_tag" );
    hint = i18n("Updates the selected files to a given tag, branch or date");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("Update to &HEAD"), 0,
                          this, SLOT(slotUpdateToHead()),
                          actionCollection(), "update_to_head" );
    hint = i18n("Updates the selected files to the HEAD revision");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Merge..."), 0,
                          this, SLOT(slotMerge()),
                          actionCollection(), "merge" );
    hint = i18n("Merges a branch or a set of modifications into the selected files");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Add Watch..."), 0,
                          this, SLOT(slotAddWatch()),
                          actionCollection(), "add_watch" );
    hint = i18n("Adds a watch for the selected files");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Remove Watch..."), 0,
                          this, SLOT(slotRemoveWatch()),
                          actionCollection(), "remove_watch" );
    hint = i18n("Removes a watch from the selected files");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("Show &Watchers"), 0,
                          this, SLOT(slotShowWatchers()),
                          actionCollection(), "show_watchers" );
    hint = i18n("Shows the watchers of the selected files");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("Ed&it Files"), 0,
                          this, SLOT(slotEdit()),
                          actionCollection(), "edit_files" );
    hint = i18n("Edits (cvs edit) the selected files");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("U&nedit Files"), 0,
                          this, SLOT(slotUnedit()),
                          actionCollection(), "unedit_files" );
    hint = i18n("Unedits (cvs unedit) the selected files");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("Show &Editors"), 0,
                          this, SLOT(slotShowEditors()),
                          actionCollection(), "show_editors" );
    hint = i18n("Shows the editors of the selected files");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Lock Files"), 0,
                          this, SLOT(slotLock()),
                          actionCollection(), "lock_files" );
    hint = i18n("Locks the selected files, so that others can't modify them");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("Unl&ock Files"), 0,
                          this, SLOT(slotUnlock()),
                          actionCollection(), "unlock_files" );
    hint = i18n("Unlocks the selected files");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("Create &Patch Against Repository"), 0,
                          this, SLOT(slotMakePatch()),
                          actionCollection(), "make_patch" );
    hint = i18n("Creates a patch from the modifications in your sandbox");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    //
    // Repository Menu
    //
    action = new KAction( i18n("&Checkout..."), 0,
                          this, SLOT(slotCheckout()),
                          actionCollection(), "repository_checkout" );
    hint = i18n("Allows you to checkout a module from a repository");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Import..."), 0,
                          this, SLOT(slotImport()),
                          actionCollection(), "repository_import" );
    hint = i18n("Allows you to import a module into a repository");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Repositories..."), 0,
                          this, SLOT(slotRepositories()),
                          actionCollection(), "show_repositories" );
    hint = i18n("Configures a list of repositories you regularly use");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    //
    // Settings menu
    //
    action = new KToggleAction( i18n("Hide All &Files"), 0,
                                this, SLOT(slotHideFiles()),
                                actionCollection(), "settings_hide_files" );
    hint = i18n("Determines whether only directories are shown");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KToggleAction( i18n("Hide Up-&To-Date Files"), 0,
                                this, SLOT(slotHideUpToDate()),
                                actionCollection(), "settings_hide_uptodate" );
    hint = i18n("Determines whether up-to-date files are hidden");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KToggleAction( i18n("Hide Removed Files"), 0,
                                this, SLOT(slotHideRemoved()),
                                actionCollection(), "settings_hide_removed" );
    hint = i18n("Determines whether removed files are hidden");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KToggleAction( i18n("Hide Non-CVS Files"), 0,
                                this, SLOT(slotHideNotInCVS()),
                                actionCollection(), "settings_hide_notincvs" );
    hint = i18n("Determines whether files not in CVS are hidden");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KToggleAction( i18n("Create &Directories on Update"), 0,
                                this, SLOT(slotCreateDirs()),
                                actionCollection(), "settings_create_dirs" );
    hint = i18n("Determines whether updates create directories");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KToggleAction( i18n("&Prune Empty Directories on Update"), 0,
                                this, SLOT(slotPruneDirs()),
                                actionCollection(), "settings_prune_dirs" );
    hint = i18n("Determines whether updates remove empty directories");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KToggleAction( i18n("&Update Recursively"), 0,
                                this, SLOT(slotUpdateRecursive()),
                                actionCollection(), "settings_update_recursively" );
    hint = i18n("Determines whether updates are recursive");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KToggleAction( i18n("C&ommit && Remove Recursively"), 0,
                                this, SLOT(slotCommitRecursive()),
                                actionCollection(), "settings_commit_recursively" );
    hint = i18n("Determines whether commits and removes are recursive");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KToggleAction( i18n("Do cvs &edit Automatically When Necessary"), 0,
                                this, SLOT(slotDoCVSEdit()),
                                actionCollection(), "settings_do_cvs_edit" );
    hint = i18n("Determines whether automatic cvs editing is active");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("Configure Cervisia..."), "configure", 0,
                          this, SLOT(slotConfigure()),
                          actionCollection(), "configure_cervisia" );
    hint = i18n("Allows you to configure the Cervisia KPart");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    //
    // Help Menu
    //
    action = KStdAction::help( this, SLOT(slotHelp()),
                               actionCollection() );

    action = new KAction( i18n("CVS &Manual"), 0,
                          this, SLOT(slotCVSInfo()),
                          actionCollection(), "help_cvs_manual" );
    hint = i18n("Opens the help browser with the CVS documentation");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    //action = KStdAction::aboutApp( this, SLOT(aboutCervisia()),
    //			   actionCollection(), "help_about_cervisia" );
}

void CervisiaPart::popupRequested()
{
    QPopupMenu *pop = static_cast<QPopupMenu *>( factory()->container("context_popup", this) );
    if (!pop)
    {
        qWarning( "CervisiaPart: Missing XML definition for context_popup\n" );
        return;
    }
    pop->exec(QCursor::pos());
}

void CervisiaPart::updateActions()
{
    bool hassandbox = !sandbox.isNull();

    actionCollection()->action( "insert_changelog_entry" )->setEnabled( hassandbox );
    actionCollection()->action( "view_unfold_tree" )->setEnabled( hassandbox );
    actionCollection()->action( "view_fold_tree" )->setEnabled( hassandbox );

    bool single = update->hasSingleSelection();

    actionCollection()->action( "file_edit" )->setEnabled( single );
    actionCollection()->action( "file_resolve" )->setEnabled( single );
    actionCollection()->action( "view_log" )->setEnabled( single );
    actionCollection()->action( "view_annotate" )->setEnabled( single );
    actionCollection()->action( "view_diff" )->setEnabled( single );
    actionCollection()->action( "view_last_change" )->setEnabled( single );

    //    bool nojob = !( actionCollection()->action( "stop_job" )->isEnabled() );
    bool selected = (update->currentItem() != 0);
    bool nojob = !hasRunningJob && selected;
    actionCollection()->action( "file_update" )->setEnabled( nojob );
    actionCollection()->action( "file_status" )->setEnabled( nojob );
    actionCollection()->action( "file_commit" )->setEnabled( nojob );
    actionCollection()->action( "file_add" )->setEnabled( nojob );
    actionCollection()->action( "file_add_binary" )->setEnabled( nojob );
    actionCollection()->action( "file_remove" )->setEnabled( nojob );
    actionCollection()->action( "file_revert_local_changes" )->setEnabled( nojob );

    actionCollection()->action( "create_tag" )->setEnabled( nojob );
    actionCollection()->action( "delete_tag" )->setEnabled( nojob );
    actionCollection()->action( "update_to_tag" )->setEnabled( nojob );
    actionCollection()->action( "update_to_head" )->setEnabled( nojob );
    actionCollection()->action( "merge" )->setEnabled( nojob );
    actionCollection()->action( "add_watch" )->setEnabled( nojob );
    actionCollection()->action( "remove_watch" )->setEnabled( nojob );
    actionCollection()->action( "show_watchers" )->setEnabled( nojob );
    actionCollection()->action( "edit_files" )->setEnabled( nojob );
    actionCollection()->action( "unedit_files" )->setEnabled( nojob );
    actionCollection()->action( "show_editors" )->setEnabled( nojob );
    actionCollection()->action( "lock_files" )->setEnabled( nojob );
    actionCollection()->action( "unlock_files" )->setEnabled( nojob );

    actionCollection()->action( "repository_checkout" )->setEnabled( !hasRunningJob );
    actionCollection()->action( "repository_import" )->setEnabled( !hasRunningJob );

    actionCollection()->action( "view_history" )->setEnabled(selected);
    actionCollection()->action( "make_patch" )->setEnabled(selected);
}


void CervisiaPart::aboutCervisia()
{
    QString aboutstr(i18n("Cervisia %1\n"
                          "(Using KDE %2)\n"
                          "\n"
                          "Copyright (c) 1999-2002\n"
                          "Bernd Gehrmann <bernd@mail.berlios.de>\n"
                          "\n"
                          "This program may be distributed under the terms of the Q Public\n"
                          "License as defined by Trolltech AS of Norway and appearing in the\n"
                          "file LICENSE.QPL included in the packaging of this file.\n\n"
                          "This program is distributed in the hope that it will be useful,\n"
                          "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                          "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\n"
                          "See the ChangeLog file for a list of contributors."));
    QMessageBox::about(0, i18n("About Cervisia"),
                       aboutstr.arg(CERVISIA_VERSION).arg(KDE_VERSION_STRING));
}


KAboutData* CervisiaPart::createAboutData()
{
    return new KAboutData( "cervisiapart", I18N_NOOP("Cervisia"),
                           CERVISIA_VERSION,
                           I18N_NOOP("A CVS frontend"),
                           KAboutData::License_QPL,
                           I18N_NOOP("Copyright (c) 1999-2002 Bernd Gehrmann"));
}


void CervisiaPart::slotOpenSandbox()
{
    QString dirname = KFileDialog::getExistingDirectory(QDir::homeDirPath(), widget(),
                                                        i18n("Open Sandbox"));
    if (dirname.isEmpty())
        return;

    openSandbox(dirname);
}


void CervisiaPart::slotChangeLog()
{
    // Modal dialog
    ChangeLogDialog *l = new ChangeLogDialog();
    if (l->readFile(sandbox + "/ChangeLog"))
    {
        if (l->exec())
            changelogstr = l->message();
    }

    delete l;
}


void CervisiaPart::slotOpen()
{
    QStringList filenames = update->fileSelection();
    if (filenames.isEmpty())
        return;
    openFiles(filenames);
}


void CervisiaPart::openFile(QString filename)
{
    QStringList files;
    files << filename;
    openFiles(files);
}


void CervisiaPart::openFiles(const QStringList &filenames)
{
    // First check the cvs edit stuff
    if (opt_doCVSEdit)
        {
            CvsProgressDialog l("Edit", widget() );
            l.setCaption(i18n("CVS Edit"));
            QString cmdline = cvsClient(repository) + " edit ";

            bool doit = false;
            for ( QStringList::ConstIterator it = filenames.begin();
                  it != filenames.end(); ++it )
                {
                    if (!QFileInfo(*it).isWritable())
                        {
                            doit = true;
                            cmdline += " ";
                            cmdline += KShellProcess::quote(*it);
                        }
                }

            if (doit)
                if (!l.execCommand(sandbox, repository, cmdline, "edit"))
                    return;
        }

    // Now open the files by either by running the configured external
    // editor, or (if it is not explicitly set) by using KRun
    KConfig *conf = config();
    conf->setGroup("Communication");
    QString editor = conf->readEntry("Editor");

    if (!editor.isEmpty()) {
        KShellProcess proc("/bin/sh");
        proc << editor;
        for ( QStringList::ConstIterator it = filenames.begin();
              it != filenames.end(); ++it )
            proc << KShellProcess::quote(*it);
        proc.start(KProcess::DontCare);
    } else {
        QDir dir(sandbox);
        for ( QStringList::ConstIterator it = filenames.begin();
              it != filenames.end(); ++it )
        {
            KURL u;
            u.setPath(dir.absFilePath(*it));
            (void) new KRun(u, 0, true, false);
        }
    }
}


void CervisiaPart::slotResolve()
{
    QString filename;
    update->getSingleSelection(&filename);
    if (filename.isEmpty())
        return;

    // Non-modal dialog
    ResolveDialog *l = new ResolveDialog();
    if (l->parseFile(filename))
        l->show();
    else
        delete l;
}


void CervisiaPart::slotUpdate()
{
    updateSandbox();
}


void CervisiaPart::slotStatus()
{
    QStringList list = update->multipleSelection();
    if (list.isEmpty())
        return;

    update->prepareJob(opt_updateRecursive, UpdateView::UpdateNoAct);
    
    QString files = joinLine(list);
        
    DCOPRef cvsJob = cvsService->status(files, opt_updateRecursive);

    // get command line from cvs job
    QString cmdline;
    DCOPReply reply = cvsJob.call("cvsCommand()");
    if( reply.isValid() )
        reply.get<QString>(cmdline);
            
    if( protocol->startJob() )
    {
        showJobStart(cmdline);
        connect( protocol, SIGNAL(receivedLine(QString)), update, SLOT(processUpdateLine(QString)) );
        connect( protocol, SIGNAL(jobFinished(bool, int)), update, SLOT(finishJob(bool, int)) );
        connect( protocol, SIGNAL(jobFinished(bool, int)), this, SLOT(slotJobFinished()) );
    }
}


void CervisiaPart::slotUpdateToTag()
{
    UpdateDialog *l = new UpdateDialog(sandbox, repository, widget() );

    if (l->exec())
    {
        QString tagopt;
        if (l->byTag())
        {
            tagopt = "-r ";
            tagopt += l->tag();
        }
        else
        {
            tagopt = "-D ";
            tagopt += KShellProcess::quote(l->date());
        }
        tagopt += " ";
        updateSandbox(tagopt);
    }
    delete l;
}


void CervisiaPart::slotUpdateToHead()
{
    updateSandbox("-A");
}


void CervisiaPart::slotRevert()
{
    updateSandbox("-C");
}


void CervisiaPart::slotMerge()
{
    MergeDialog *l = new MergeDialog(sandbox, repository, widget() );

    if (l->exec())
    {
        QString tagopt;
        if (l->byBranch())
        {
            tagopt = "-j ";
            tagopt += l->branch();
        }
        else
        {
            tagopt = "-j ";
            tagopt += l->tag1();
            tagopt += " -j ";
            tagopt += l->tag2();
        }
        tagopt += " ";
        updateSandbox(tagopt);
    }
    delete l;
}


void CervisiaPart::slotCommit()
{
    commitOrAddOrRemove(CommitDialog::Commit);
}


void CervisiaPart::slotAdd()
{
    commitOrAddOrRemove(CommitDialog::Add);
}


void CervisiaPart::slotAddBinary()
{
    commitOrAddOrRemove(CommitDialog::AddBinary);
}


void CervisiaPart::slotRemove()
{
    commitOrAddOrRemove(CommitDialog::Remove);
}


void CervisiaPart::updateSandbox(const QString &extraopt)
{
    QStringList list = update->multipleSelection();
    if (list.isEmpty())
        return;

    update->prepareJob(opt_updateRecursive, UpdateView::Update);
   
    QString files = joinLine(list);
        
    DCOPRef cvsJob = cvsService->update(files, opt_updateRecursive,
                        opt_createDirs, opt_pruneDirs, extraopt);

    // get command line from cvs job
    QString cmdline;
    DCOPReply reply = cvsJob.call("cvsCommand()");
    if( reply.isValid() )
        reply.get<QString>(cmdline);
            
    if( protocol->startJob() )
    {
        showJobStart(cmdline);
        connect( protocol, SIGNAL(receivedLine(QString)), update, SLOT(processUpdateLine(QString)) );
        connect( protocol, SIGNAL(jobFinished(bool, int)), update, SLOT(finishJob(bool, int)) );
        connect( protocol, SIGNAL(jobFinished(bool, int)), this, SLOT(slotJobFinished()) );
    }
}


void CervisiaPart::commitOrAddOrRemove(CommitDialog::ActionType action)
{
    QStringList list = update->multipleSelection();
    if (list.isEmpty())
        return;

    // modal dialog
    CommitDialog *l = new CommitDialog(action, widget());
    if (action == CommitDialog::Commit)
    {
        l->setLogMessage(changelogstr);
        l->setLogHistory(sandbox, repository, recentCommits);
    }
    l->setFileList(list);

    if (l->exec())
    {
        QString cmdline;
        switch (action)
        {
            case CommitDialog::Commit:
            {
                QString msg = l->logMessage();
                if( !recentCommits.contains( msg ) )
                {
                    recentCommits.prepend( msg );
                    while (recentCommits.count() > 50)
                        recentCommits.remove( recentCommits.last() );

                    KConfig* conf = config();
                    conf->setGroup( "CommitLogs" );
                    conf->writeEntry( sandbox, recentCommits, COMMIT_SPLIT_CHAR );
                }

                update->prepareJob(opt_commitRecursive, UpdateView::Commit);
                cmdline = cvsClient(repository) + " commit ";
                if (opt_commitRecursive)
                    cmdline += "-R ";
                else
                    cmdline += "-l ";
                cmdline += "-m ";
                cmdline += KShellProcess::quote(l->logMessage());
                cmdline += " ";
            }
            break;

            case CommitDialog::Add:
                update->prepareJob(false, UpdateView::Add);
                cmdline = cvsClient(repository) + " add ";
            break;

            case CommitDialog::AddBinary:
                update->prepareJob(false, UpdateView::Add);
                cmdline = cvsClient(repository) + " add -kb ";
            break;

            case CommitDialog::Remove:
                update->prepareJob(opt_commitRecursive, UpdateView::Remove);
                cmdline = cvsClient(repository) + " remove -f ";
                if (opt_commitRecursive)
                    cmdline += "-R ";
                else
                    cmdline += "-l ";
            break;
        }

        cmdline += joinLine(list);
        cmdline += " 2>&1";

        if (protocol->startJob(sandbox, repository, cmdline))
        {
            showJobStart(cmdline);
            connect( protocol, SIGNAL(jobFinished(bool, int)),
                     update, SLOT(finishJob(bool, int)) );
            connect( protocol, SIGNAL(jobFinished(bool, int)),
                     this, SLOT(slotJobFinished()) );
        }
    }

    delete l;
}

void CervisiaPart::slotBrowseLog()
{
    QString filename;
    update->getSingleSelection(&filename);
    if (filename.isEmpty())
        return;

    // Non-modal dialog
    LogDialog *l = new LogDialog();
    if (l->parseCvsLog(cvsService, filename))
        l->show();
    else
        delete l;
}


#if 0
void CervisiaPart::slotBrowseMultiLog()
{
    QStrList list = update->multipleSelection();
    if (!list.isEmpty())
    {
        // Non-modal dialog
        MultiLogDialog *l = new MultiLogDialog();
        if (l->parseCvsLog(".", list))
            l->show();
        else
            delete l;
    }
}
#endif


void CervisiaPart::slotAnnotate()
{
    QString filename;
    update->getSingleSelection(&filename);

    if (filename.isEmpty())
        return;

    // Non-modal dialog
    AnnotateDialog *l = new AnnotateDialog();
    AnnotateController ctl(l, cvsService);
    ctl.showDialog(filename);
}


void CervisiaPart::slotDiff()
{
    QString filename;
    update->getSingleSelection(&filename);

    if (filename.isEmpty())
        return;

    // Non-modal dialog
    DiffDialog *l = new DiffDialog();
    if (l->parseCvsDiff(sandbox, repository, filename, "", ""))
        l->show();
    else
        delete l;
}


void CervisiaPart::slotAddWatch()
{
    addOrRemoveWatch(WatchDialog::Add);
}


void CervisiaPart::slotRemoveWatch()
{
    addOrRemoveWatch(WatchDialog::Remove);
}


void CervisiaPart::addOrRemoveWatch(WatchDialog::ActionType action)
{
    QStringList list = update->multipleSelection();
    if (list.isEmpty())
        return;

    WatchDialog *l = new WatchDialog(action, widget());

    if (l->exec() && l->events() != WatchDialog::None)
    {
        QString cmdline = cvsClient(repository);
        cmdline += " watch ";
        if (action == WatchDialog::Add)
            cmdline += "add ";
        else
            cmdline += "remove ";

        WatchDialog::Events events = l->events();
        if (events != WatchDialog::All)
        {
            if (events & WatchDialog::Commits)
                cmdline += "-a commit ";
            if (events & WatchDialog::Edits)
                cmdline += "-a edit ";
            if (events & WatchDialog::Unedits)
                cmdline += "-a unedit ";
        }

        cmdline += joinLine(list);

        if (protocol->startJob(sandbox, repository, cmdline))
        {
            showJobStart(cmdline);
            connect( protocol, SIGNAL(jobFinished(bool, int)),
                     this,     SLOT(slotJobFinished()) );
        }
    }

    delete l;
}


void CervisiaPart::slotShowWatchers()
{
    QStringList list = update->multipleSelection();
    if (list.isEmpty())
        return;

    QString cmdline = cvsClient(repository);
    cmdline += " watchers ";
    cmdline += joinLine(list);

    if (protocol->startJob(sandbox, repository, cmdline))
    {
        showJobStart(cmdline);
        connect( protocol, SIGNAL(jobFinished(bool, int)),
                 this,     SLOT(slotJobFinished()) );
    }
}


void CervisiaPart::slotEdit()
{
    QStringList list = update->multipleSelection();
    if (list.isEmpty())
        return;

    QString cmdline = cvsClient(repository);
    cmdline += " edit ";
    cmdline += joinLine(list);

    if (protocol->startJob(sandbox, repository, cmdline))
    {
        showJobStart(cmdline);
        connect( protocol, SIGNAL(jobFinished(bool, int)),
                 this,     SLOT(slotJobFinished()) );
    }
}


void CervisiaPart::slotUnedit()
{
    QStringList list = update->multipleSelection();
    if (list.isEmpty())
        return;

    QString cmdline = "echo y | ";
    cmdline += cvsClient(repository);
    cmdline += " unedit ";
    cmdline += joinLine(list);

    if (protocol->startJob(sandbox, repository, cmdline))
    {
        showJobStart(cmdline);
        connect( protocol, SIGNAL(jobFinished(bool, int)),
                 this,     SLOT(slotJobFinished()) );
    }
}


void CervisiaPart::slotLock()
{
    QStringList list = update->multipleSelection();
    if (list.isEmpty())
        return;

    QString cmdline = cvsClient(repository);
    cmdline += " admin -l ";
    cmdline += joinLine(list);

    if (protocol->startJob(sandbox, repository, cmdline))
    {
        showJobStart(cmdline);
        connect( protocol, SIGNAL(jobFinished(bool, int)),
                 this,     SLOT(slotJobFinished()) );
    }
}


void CervisiaPart::slotUnlock()
{
    QStringList list = update->multipleSelection();
    if (list.isEmpty())
        return;

    QString cmdline = cvsClient(repository);
    cmdline += " admin -u ";
    cmdline += joinLine(list);

    if (protocol->startJob(sandbox, repository, cmdline))
    {
        showJobStart(cmdline);
        connect( protocol, SIGNAL(jobFinished(bool, int)),
                 this,     SLOT(slotJobFinished()) );
    }
}


void CervisiaPart::slotShowEditors()
{
    QStringList list = update->multipleSelection();
    if (list.isEmpty())
        return;

    QString cmdline = cvsClient(repository);
    cmdline += " editors ";
    cmdline += joinLine(list);

    if (protocol->startJob(sandbox, repository, cmdline))
    {
        showJobStart(cmdline);
        connect( protocol, SIGNAL(jobFinished(bool, int)),
                 this,     SLOT(slotJobFinished()) );
    }
}


void CervisiaPart::slotMakePatch()
{
    CvsProgressDialog l("Diff", widget());
    l.setCaption(i18n("CVS Diff"));

    QString cmdline = cvsClient(repository);
    cmdline += " diff -uR 2>/dev/null";
    if (!l.execCommand(sandbox, repository, cmdline, ""))
        return;

    QString filename = KFileDialog::getSaveFileName();
    if (filename.isEmpty())
        return;

    QFile f(filename);
    if (!f.open(IO_WriteOnly))
    {
        KMessageBox::sorry(widget(),
                           i18n("Could not open file for writing."),
                           "Cervisia");
        return;
    }
    QTextStream t(&f);
    QString line;
    while (l.getOneLine(&line))
        t << line << '\n';

    f.close();
}


void CervisiaPart::slotImport()
{
    importOrCheckout(CheckoutDialog::Import);
}


void CervisiaPart::slotCheckout()
{
    importOrCheckout(CheckoutDialog::Checkout);
}


void CervisiaPart::importOrCheckout(CheckoutDialog::ActionType action)
{
    CheckoutDialog *l = new CheckoutDialog(action, widget());

    if (l->exec())
        {
            QString cmdline = "cd ";
            cmdline += l->workingDirectory();
            cmdline += " && ";
            cmdline += cvsClient(repository);
            cmdline += " -d ";
            cmdline += l->repository();
            if (action == CheckoutDialog::Checkout)
                {
                    cmdline += " checkout ";
                    if (!l->branch().isEmpty())
			{
                            cmdline += " -r ";
                            cmdline += l->branch();
			}
                    if (opt_pruneDirs)
                        cmdline += " -P ";
                    cmdline += l->module();
                }
            else
                {
                    cmdline += " import";
                    if (l->importBinary())
                        cmdline += " -kb";
                    QString ignore = l->ignoreFiles().stripWhiteSpace();
                    if (!ignore.isEmpty())
                        {
                            cmdline += " -I ";
                            cmdline += KShellProcess::quote(ignore);
                        }
                    QString comment = l->comment().stripWhiteSpace();
                    cmdline += " -m ";
                    cmdline += (QString("\"") + comment + "\" ");
                    cmdline += l->module();
                    cmdline += " ";
                    cmdline += l->vendorTag();
                    cmdline += " ";
                    cmdline += l->releaseTag();
                }

            if (protocol->startJob(sandbox, repository, cmdline))
                {
                    showJobStart(cmdline);
                    connect( protocol, SIGNAL(jobFinished(bool, int)),
                             this,     SLOT(slotJobFinished()) );
                }
        }

    delete l;
}


void CervisiaPart::slotRepositories()
{
    RepositoryDialog *l = new RepositoryDialog(widget());
    l->show();
}


void CervisiaPart::slotCreateTag()
{
    createOrDeleteTag(TagDialog::Create);
}


void CervisiaPart::slotDeleteTag()
{
    createOrDeleteTag(TagDialog::Delete);
}


void CervisiaPart::createOrDeleteTag(TagDialog::ActionType action)
{
    QStringList list = update->multipleSelection();
    if (list.isEmpty())
        return;

    TagDialog *l = new TagDialog(action, sandbox, repository, widget());

    if (l->exec())
    {
        QString cmdline = cvsClient(repository);
        cmdline += " tag ";
        if (action == TagDialog::Delete)
            cmdline += "-d ";
        if (l->branchTag())
            cmdline += "-b ";
        if (l->forceTag())
            cmdline += "-F ";
        cmdline += KProcess::quote(l->tag());
        cmdline += " ";
        cmdline += joinLine(list);

        if (protocol->startJob(sandbox, repository, cmdline))
        {
            showJobStart(cmdline);
            connect( protocol, SIGNAL(jobFinished(bool, int)),
                     this,     SLOT(slotJobFinished()) );
        }
    }

    delete l;
}



void CervisiaPart::slotLastChange()
{
    QString filename, revA, revB;
    update->getSingleSelection(&filename, &revA);
    if (filename.isEmpty())
        return;

    int pos, lastnumber;
    bool ok;
    if ( (pos = revA.findRev('.')) == -1
         || (lastnumber=revA.right(revA.length()-pos-1).toUInt(&ok), !ok) )
    {
        KMessageBox::sorry(widget(),
                           i18n("The revision looks invalid."),
                           "Cervisia");
        return;
    }
    if (lastnumber == 0)
    {
        KMessageBox::sorry(widget(),
                           i18n("This is the first revision of the branch."),
                           "Cervisia");
        return;
    }
    revB = revA.left(pos+1);
    revB += QString::number(lastnumber-1);

    // Non-modal dialog
    DiffDialog *l = new DiffDialog();
    if (l->parseCvsDiff(sandbox, repository, filename, revB, revA))
        l->show();
    else
        delete l;
}


void CervisiaPart::slotHistory()
{
    // Non-modal dialog
    HistoryDialog *l = new HistoryDialog();
    if (l->parseHistory(sandbox, repository))
        l->show();
    else
        delete l;
}


void CervisiaPart::slotHideFiles()
{
    opt_hideFiles = !opt_hideFiles;
    setFilter();
}


void CervisiaPart::slotHideUpToDate()
{
    opt_hideUpToDate = !opt_hideUpToDate;
    setFilter();
}


void CervisiaPart::slotHideRemoved()
{
    opt_hideRemoved = !opt_hideRemoved;
    setFilter();
}


void CervisiaPart::slotHideNotInCVS()
{
    opt_hideNotInCVS = !opt_hideNotInCVS;
    setFilter();
}

void CervisiaPart::slotFoldTree()
{
	update->foldTree();
	setFilter();
}

void CervisiaPart::slotUnfoldTree()
{
	update->unfoldTree();
	setFilter();
}

void CervisiaPart::slotCreateDirs()
{
    opt_createDirs = !opt_createDirs;
}


void CervisiaPart::slotPruneDirs()
{
    opt_pruneDirs = !opt_pruneDirs;
}


void CervisiaPart::slotUpdateRecursive()
{
    opt_updateRecursive = !opt_updateRecursive;
}


void CervisiaPart::slotCommitRecursive()
{
    opt_commitRecursive = !opt_commitRecursive;
}


void CervisiaPart::slotDoCVSEdit()
{
    opt_doCVSEdit = !opt_doCVSEdit;
}

void CervisiaPart::slotConfigure()
{
    KConfig *conf = config();
    SettingsDialog *l = new SettingsDialog( conf, widget() );
    l->exec();

    conf->setGroup("LookAndFeel");
    bool splitHorz = conf->readBoolEntry("SplitHorizontally",true);
    splitter->setOrientation( splitHorz ?
                              QSplitter::Vertical :
                              QSplitter::Horizontal);
}

void CervisiaPart::slotHelp()
{
    emit setStatusBarText( i18n("Invoking help on Cervisia") );
    KApplication::startServiceByDesktopName("khelpcenter", QString("help:/cervisia/index.html"));
}


void CervisiaPart::slotCVSInfo()
{
    emit setStatusBarText( i18n("Invoking help on CVS") );
    KApplication::startServiceByDesktopName("khelpcenter", QString("info:/cvs/Top"));
}


void CervisiaPart::showJobStart(const QString &cmdline)
{
    hasRunningJob = true;
    actionCollection()->action( "stop_job" )->setEnabled( true );

    emit setStatusBarText( cmdline );
    updateActions();
}


void CervisiaPart::slotJobFinished()
{
    actionCollection()->action( "stop_job" )->setEnabled( false );
    hasRunningJob = false;
    emit setStatusBarText( i18n("Done") );
    updateActions();
}


void CervisiaPart::openSandbox(const QString &dirname)
{
    // change the working copy directory for the cvs DCOP service
    bool opened = cvsService->setWorkingCopy(dirname);
    
    if( !cvsService->ok() || !opened )
    {
        KMessageBox::sorry(widget(),
                           i18n("This is not a CVS directory.\n"
                           "If you did not intend to use Cervisia, you can "
                           "switch view modes within Konqueror."),
                           "Cervisia");
        
        // remove path from recent sandbox menu
        QFileInfo fi(dirname);
        recent->removeURL( KURL::fromPathOrURL(fi.absFilePath()) );
        
        return;
    }
    
    changelogstr = "";
    sandbox      = "";
    repository   = "";
    
    // get path of sandbox for recent sandbox menu
    sandbox = cvsService->workingCopy();
    recent->addURL( KURL::fromPathOrURL(sandbox) );
    
    // get repository for the caption of the window
    repository = cvsService->repository();
    emit setWindowCaption(sandbox + "(" + repository + ")");
    
    QDir::setCurrent(sandbox);
    update->openDirectory(sandbox);
    setFilter();

    KConfig *conf = config();
    conf->setGroup("General");
    bool dostatus = conf->readBoolEntry(repository.contains(":")?
                                        "StatusForRemoteRepos" :
                                        "StatusForLocalRepos",
                                        false);
    if (dostatus)
    {
        update->setSelected(update->firstChild(), true);
        slotStatus();
    }

    //load the recentCommits for this app from the KConfig app
    conf->setGroup( "CommitLogs" );
    recentCommits = conf->readListEntry( sandbox, COMMIT_SPLIT_CHAR );
}


void CervisiaPart::setFilter()
{
    UpdateView::Filter filter = UpdateView::Filter(0);
    if (opt_hideFiles)
        filter = UpdateView::Filter(filter | UpdateView::OnlyDirectories);
    if (opt_hideUpToDate)
        filter = UpdateView::Filter(filter | UpdateView::NoUpToDate);
    if (opt_hideRemoved)
        filter = UpdateView::Filter(filter | UpdateView::NoRemoved);
    if (opt_hideNotInCVS)
        filter = UpdateView::Filter(filter | UpdateView::NoNotInCVS);
    update->setFilter(filter);

    QString str;
    if (opt_hideFiles)
        str = "F";
    else
        {
            if (opt_hideUpToDate)
                str += "N";
            if (opt_hideRemoved)
                str += "R";
        }

    // TODO: Find a new way to handle the status items as you can't do this with KParts yet
    //filterLabel->setText(str);
    emit filterStatusChanged(str);
}


#if 0
void CervisiaPart::parseStatus(QString pathname, QStrList list)
{
    char buf[512];
    QString command;
    QString dirpath;
    QString name, statusstr, version;
    enum { Begin, File, FileSep, WorkRev, FinalSep } state;

    QString line = joinLine(list);

    command = "cd ";
    command += KProcess::quote(pathname);
    command += " && " + cvsClient(repository) + " status -l ";
    command += line;
    //command += " 2>&1";

    FILE *f = popen(QFile::encodeName(command), "r");
    if (!f)
        return;

    state = Begin;
    while (fgets(buf, sizeof buf, f))
        {
            QCString line = buf;
            chomp(&line);
            //DEBUGOUT( "Line: " << line );
            switch (state)
                {
                case Begin:
                    if (line.left(22) == "cvs status: Examining ")
                        dirpath = line.right(line.length()-22);
                    state = File;
                    //DEBUGOUT( "state = file" );
                    break;
                case File:
                    if (line.length() > 32 &&
                        line.left(6) == "File: " &&
                        line.mid(24, 8) == "Status: ")
                    {
                        name = line.mid(6, 18).stripWhiteSpace();
                        if (dirpath != ".")
                            name.prepend("/").prepend(dirpath);
                        statusstr = line.right(line.length()-32)
                                        .stripWhiteSpace();
                        state = FileSep;
                        //DEBUGOUT( "state = FileSep" );
                    }
                    break;
                case FileSep:
                    if (!line.isEmpty()) ; // Error
                        state = WorkRev;
                    //DEBUGOUT( "state = WorkRev" );
                    break;
                case WorkRev:
                    if (line.left(21) == "   Working revision:\t")
                    {
                        int pos;
                        version = line.right(line.length()-21);
                        if ( (pos = version.find(" ")) != -1 )
                            version.truncate(pos);
                        state = FinalSep;
                        //DEBUGOUT( "state = FinalSep" );
                    }
                    break;
                case FinalSep:
                    if (line == "")
                    {
                        //DEBUGOUT( "Adding: " << name <<
                        //          "Status: " << statusstr << "Version: " << version );
                        UpdateView::Status status = UpdateView::Unknown;
                        if (statusstr == "Up-to-date")
                            status = UpdateView::UpToDate;
                        else if (statusstr == "Locally Modified")
                            status = UpdateView::LocallyModified;
                        else if (statusstr == "Locally Added")
                            status = UpdateView::LocallyAdded;
                        else if (statusstr == "Locally Removed")
                            status = UpdateView::LocallyRemoved;
                        else if (statusstr == "Needs Checkout")
                            status = UpdateView::NeedsUpdate;
                        else if (statusstr == "Needs Patch")
                            status = UpdateView::NeedsPatch;
                        else if (statusstr == "Needs Merge")
                            status = UpdateView::NeedsMerge;
                        else if (statusstr == "File had conflicts on merge")
                            status = UpdateView::Conflict;
                        //update->addEntry(status, name /*, version*/);
                        state = Begin;
                        //DEBUGOUT( "state = Begin" );
                    }
                }
        }
    pclose(f);
}
#endif


void CervisiaPart::readProperties(KConfig *config)
{
    recent->loadEntries( config );

    // Unfortunately, the KConfig systems sucks and we have to live
    // with all entries in one group for session management.

    opt_createDirs = config->readBoolEntry("Create Dirs", true);
    (static_cast<KToggleAction *> (actionCollection()->action( "settings_create_dirs" )))
    ->setChecked( opt_createDirs );

    opt_pruneDirs = config->readBoolEntry("Prune Dirs", true);
    (static_cast<KToggleAction *> (actionCollection()->action( "settings_prune_dirs" )))
    ->setChecked( opt_pruneDirs );

    opt_updateRecursive = config->readBoolEntry("Update Recursive", false);
    (static_cast<KToggleAction *> (actionCollection()->action( "settings_update_recursively" )))
    ->setChecked( opt_updateRecursive );

    opt_commitRecursive = config->readBoolEntry("Commit Recursive", false);
    (static_cast<KToggleAction *> (actionCollection()->action( "settings_commit_recursively" )))
    ->setChecked( opt_commitRecursive );

    opt_doCVSEdit = config->readBoolEntry("Do cvs edit", false);
    (static_cast<KToggleAction *> (actionCollection()->action( "settings_do_cvs_edit" )))
    ->setChecked( opt_doCVSEdit );

    opt_hideFiles = config->readBoolEntry("Hide Files", false);
    (static_cast<KToggleAction *> (actionCollection()->action( "settings_hide_files" )))
    ->setChecked( opt_hideFiles );

    opt_hideUpToDate = config->readBoolEntry("Hide UpToDate Files", false);
    (static_cast<KToggleAction *> (actionCollection()->action( "settings_hide_uptodate" )))
    ->setChecked( opt_hideUpToDate );

    opt_hideRemoved = config->readBoolEntry("Hide Removed Files", false);
    (static_cast<KToggleAction *> (actionCollection()->action( "settings_hide_removed" )))
    ->setChecked( opt_hideRemoved );

    opt_hideNotInCVS = config->readBoolEntry("Hide Non CVS Files", false);
    (static_cast<KToggleAction *> (actionCollection()->action( "settings_hide_notincvs" )))
    ->setChecked( opt_hideNotInCVS );

    setFilter();

    int splitterpos1 = config->readNumEntry("Splitter Pos 1", 0);
    int splitterpos2 = config->readNumEntry("Splitter Pos 2", 0);
    if (splitterpos1)
    {
        QValueList<int> sizes;
        sizes << splitterpos1;
        sizes << splitterpos2;
        splitter->setSizes(sizes);
    }
}


void CervisiaPart::saveProperties( KConfig *config )
{
    recent->saveEntries( config );

    config->writeEntry("Create Dirs", opt_createDirs);
    config->writeEntry("Prune Dirs", opt_pruneDirs);
    config->writeEntry("Update Recursive", opt_updateRecursive);
    config->writeEntry("Commit Recursive", opt_commitRecursive);
    config->writeEntry("Do cvs edit", opt_doCVSEdit);
    config->writeEntry("Hide Files", opt_hideFiles);
    config->writeEntry("Hide UpToDate Files", opt_hideUpToDate);
    config->writeEntry("Hide Removed Files", opt_hideRemoved);
    config->writeEntry("Hide Non CVS Files", opt_hideNotInCVS);
    QValueList<int> sizes = splitter->sizes();
    config->writeEntry("Splitter Pos 1", sizes[0]);
    config->writeEntry("Splitter Pos 2", sizes[1]);
}


void CervisiaPart::readDialogProperties( KConfig *config )
{
    config->setGroup("Diff dialog");
    DiffDialog::loadOptions(config);
    config->setGroup("Log dialog");
    LogDialog::loadOptions(config);
    config->setGroup("LogList view");
    LogListView::loadOptions(config);
    config->setGroup("Resolve dialog");
    ResolveDialog::loadOptions(config);
    config->setGroup("Resolve edit dialog");
    ResolveEditorDialog::loadOptions(config);
    config->setGroup("Commmit dialog");
    CommitDialog::loadOptions(config);
    config->setGroup("ChangeLog dialog");
    ChangeLogDialog::loadOptions(config);
    config->setGroup("Annotate dialog");
    AnnotateDialog::loadOptions(config);
    config->setGroup("Checkout dialog");
    CheckoutDialog::loadOptions(config);
    config->setGroup("History dialog");
    HistoryDialog::loadOptions(config);
    config->setGroup("Repository dialog");
    RepositoryDialog::loadOptions(config);
}


void CervisiaPart::saveDialogProperties( KConfig *config )
{
    config->setGroup("Diff dialog");
    DiffDialog::saveOptions(config);
    config->setGroup("Log dialog");
    LogDialog::saveOptions(config);
    config->setGroup("LogList view");
    LogListView::saveOptions(config);
    config->setGroup("Resolve dialog");
    ResolveDialog::saveOptions(config);
    config->setGroup("Resolve edit dialog");
    ResolveEditorDialog::saveOptions(config);
    config->setGroup("Commit dialog");
    CommitDialog::saveOptions(config);
    config->setGroup("ChangeLog dialog");
    ChangeLogDialog::saveOptions(config);
    config->setGroup("Annotate dialog");
    AnnotateDialog::saveOptions(config);
    config->setGroup("Checkout dialog");
    CheckoutDialog::saveOptions(config);
    config->setGroup("History dialog");
    HistoryDialog::saveOptions(config);
    config->setGroup("Repository dialog");
    RepositoryDialog::saveOptions(config);
}

CervisiaBrowserExtension::CervisiaBrowserExtension( CervisiaPart *p )
    : KParts::BrowserExtension( p, "CervisiaBrowserExtension" )
{
    KGlobal::locale()->insertCatalogue("cervisia");
}

CervisiaBrowserExtension::~CervisiaBrowserExtension()
{

}

// Local Variables:
// c-basic-offset: 4
// End:
