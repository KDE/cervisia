/* 
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
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

#include <kparts/part.h>
#include <kparts/browserextension.h>
#include <kparts/genericfactory.h>

#include "commitdlg.h"
#include "checkoutdlg.h"
#include "watchdlg.h"
#include "tagdlg.h"

class QLabel;
class QSplitter;
class QTimer;
class UpdateView;
class ProtocolView;
class KAboutData;
class KRecentFilesAction;


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

    virtual void readProperties(KConfig *config);
    virtual void saveProperties(KConfig *config);

    void readDialogProperties( KConfig *config );
    void saveDialogProperties( KConfig *config );
    
    static KAboutData* createAboutData();

signals:
    void filterStatusChanged(QString status);

public slots:
    virtual bool openFile() { return true; }
    virtual bool openURL( const KURL & );
    void openSandbox(const QString &dirname);

    void openFile(QString filename);
    void openFiles(const QStringList &filenames);
    void popupRequested();
    void updateActions();
    
    void aboutCervisia();
    void slotOpenSandbox( const KURL &url );

    void slotOpenSandbox();
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
    void slotDiff();
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

    void slotFoldTree();
    void slotUnfoldTree();

    void slotUpdateRecursive();
    void slotCommitRecursive();
    void slotDoCVSEdit();
    void slotConfigure();
    void slotHelp();
    void slotCVSInfo();

protected slots:
    void slotJobFinished(bool /*success*/);

private:    
    void setupActions();

    void updateOrStatus(bool noact, const QString &extraopt);
    void commitOrAddOrRemove(CommitDialog::ActionType action);
    void addOrRemoveWatch(WatchDialog::ActionType action);
    void importOrCheckout(CheckoutDialog::ActionType action);
    void createOrDeleteTag(TagDialog::ActionType action);
    void showJobStart(const QString &command);
    void setFilter();

    UpdateView *update;
    ProtocolView *protocol;
    bool hasRunningJob;
    QSplitter *splitter;

    // TODO: Find a new way to handle the status items as you can't do this with KParts yet
    //    QLabel *filterLabel;
    QString sandbox;
    QString repository;

    QString changelogstr;
    QStringList recentCommits;
    bool opt_hideFiles, opt_hideUpToDate, opt_hideRemoved, opt_hideNotInCVS;
    bool opt_createDirs, opt_pruneDirs;
    bool opt_updateRecursive, opt_commitRecursive, opt_doCVSEdit;

    //for the Open Recent directories
    KRecentFilesAction *recent;
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
