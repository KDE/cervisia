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


#ifndef CERVISIAPART_H
#define CERVISIAPART_H

#include <kdeversion.h>
#include <kparts/part.h>
#include <kparts/browserextension.h>
#include <kparts/genericfactory.h>
#if KDE_IS_VERSION(3,1,90)
#include <kparts/statusbarextension.h>
#endif

#include "addremovedlg.h"
#include "commitdlg.h"
#include "checkoutdlg.h"
#include "watchdlg.h"
#include "tagdlg.h"

class QLabel;
class QListViewItem;
class QSplitter;
class QTimer;
class UpdateView;
class ProtocolView;
class KAboutData;
class KListView;
class KRecentFilesAction;
class CvsService_stub;


#if KDE_IS_VERSION(3,1,90)
class CervisiaStatusBarExtension : public KParts::StatusBarExtension
{
public:
    CervisiaStatusBarExtension(KParts::ReadOnlyPart* parent)
        : KParts::StatusBarExtension(parent, "CervisiaStatusBarExtension")
    {
    }
};
#endif


/**
 * An embeddable Cervisia viewer.
 */
class CervisiaPart : public KParts::ReadOnlyPart
{
    Q_OBJECT

public:
    CervisiaPart( QWidget *parentWidget, const char *widgetName,
                  QObject *parent, const char *name=0, const QStringList& args = QStringList());
    virtual ~CervisiaPart();

    /**
     * Get the config object for the part's instance.
     */
    static KConfig *config();

    QString sandBox() const { return sandbox; }

    static KAboutData* createAboutData();

public slots:
    // unused because we overwrite the default behaviour of openURL()
    virtual bool openFile() { return true; }
    virtual bool openURL( const KURL & );

    void openFile(QString filename);
    void openFiles(const QStringList &filenames);
    void popupRequested(KListView*, QListViewItem*, const QPoint&);
    void updateActions();
    
    void aboutCervisia();

    void slotOpen();
    void slotResolve();
    void slotStatus();
    void slotUpdate();
    void slotChangeLog();
    void slotCommit();
    void slotAdd();
    void slotAddBinary();

    void slotRemove();
    void slotRevert();
    void slotBrowseLog();
    //    void slotBrowseMultiLog();
    void slotAnnotate();
    void slotDiffBase();
    void slotDiffHead();
    void slotLastChange();
    void slotHistory();
    void slotCheckout();
    void slotImport();
    void slotRepositories();
    void slotCreateTag();
    void slotDeleteTag();
    void slotUpdateToTag();
    void slotUpdateToHead();
    void slotMerge();
    void slotAddWatch();
    void slotRemoveWatch();
    void slotShowWatchers();
    void slotEdit();
    void slotUnedit();
    void slotShowEditors();
    void slotLock();
    void slotUnlock();
    void slotMakePatch();
    void slotCreateDirs();
    void slotPruneDirs();
    void slotHideFiles();
    void slotHideUpToDate();
    void slotHideRemoved();

    void slotHideNotInCVS();
    void slotHideEmptyDirectories();

    void slotFoldTree();
    void slotUnfoldTree();

    void slotUpdateRecursive();
    void slotCommitRecursive();
    void slotDoCVSEdit();
    void slotConfigure();
    void slotHelp();
    void slotCVSInfo();

protected slots:
    void slotJobFinished();

private slots:
    // called by menu action "Open Sandbox..."
    void slotOpenSandbox();

protected:
    virtual void guiActivateEvent(KParts::GUIActivateEvent* event);

private:    
    void setupActions();

    void readSettings();
    void writeSettings();
       
    bool openSandbox(const QString &dirname);
    void updateSandbox(const QString &extraopt = QString::null);
    void addOrRemove(AddRemoveDialog::ActionType action);
    void addOrRemoveWatch(WatchDialog::ActionType action);
    void importOrCheckout(CheckoutDialog::ActionType action);
    void createOrDeleteTag(TagDialog::ActionType action);
    void showJobStart(const QString &command);
    void showDiff(const QString& revision);
    void setFilter();

    UpdateView *update;
    ProtocolView *protocol;
    bool hasRunningJob;
    QSplitter *splitter;

    QString sandbox;
    QString repository;

    QString changelogstr;
    QStringList recentCommits;
    bool opt_hideFiles, opt_hideUpToDate, opt_hideRemoved, opt_hideNotInCVS, opt_hideEmptyDirectories;
    bool opt_createDirs, opt_pruneDirs;
    bool opt_updateRecursive, opt_commitRecursive, opt_doCVSEdit;

    //for the Open Recent directories
    KRecentFilesAction *recent;

    CvsService_stub* cvsService;
#if KDE_IS_VERSION(3,1,90)
    CervisiaStatusBarExtension* statusBar;
    QLabel*                     filterLabel;
#endif
};

typedef KParts::GenericFactory<CervisiaPart> CervisiaFactory;

/**
 * A mysterious class, needed to make Konqueror intrgration work.
 */
class CervisiaBrowserExtension : public KParts::BrowserExtension
{
    Q_OBJECT

public:
    CervisiaBrowserExtension( CervisiaPart * );
    ~CervisiaBrowserExtension();
};

#endif // CERVISIAPART_H


// Local Variables:
// c-basic-offset: 4
// End:
