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


#include "logdlg.h"

#include <qcombobox.h>
#include <qdir.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtabwidget.h>
#include <qtextedit.h>
#include <qwhatsthis.h>
#include <kdebug.h>
#include <kfinddialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kprocess.h>
#include <krfcdate.h>
#include <krun.h>
#include <ktempfile.h>
#include <kurl.h>

#include "cvsservice_stub.h"
#include "repository_stub.h"
#include "annotatedlg.h"
#include "annotatectl.h"
#include "cvsprogressdlg.h"
#include "diffdlg.h"
#include "loginfo.h"
#include "loglist.h"
#include "logplainview.h"
#include "logtree.h"
#include "misc.h"
#include "progressdlg.h"

#include <kdeversion.h>
#if KDE_VERSION < KDE_MAKE_VERSION(3,1,90)
#include "configutils.h"
#endif


LogDialog::LogDialog(KConfig& cfg, QWidget *parent, const char *name)
    : KDialogBase(parent, name, false, QString::null,
                  Ok | Close | Help | User1 | User2 | User3, Close, true,
                  KGuiItem(i18n("&Annotate")),
                  KGuiItem(i18n("&Diff")),
                  KGuiItem(i18n("&Find"), "find"))
    , cvsService(0)
    , partConfig(cfg)
{
    QFrame* mainWidget = makeMainWidget();

    QBoxLayout *layout = new QVBoxLayout(mainWidget, 0, spacingHint());

    tree = new LogTreeView(mainWidget);
    connect( tree, SIGNAL(revisionClicked(QString,bool)),
             this, SLOT(revisionSelected(QString,bool)) );

    list = new LogListView(partConfig, mainWidget);
    connect( list, SIGNAL(revisionClicked(QString,bool)),
             this, SLOT(revisionSelected(QString,bool)) );

    plain = new LogPlainView(mainWidget);
    connect( plain, SIGNAL(revisionClicked(QString,bool)),
             this, SLOT(revisionSelected(QString,bool)) );

    tabWidget = new QTabWidget(mainWidget);
    tabWidget->addTab(tree, i18n("&Tree"));
    tabWidget->addTab(list, i18n("&List"));
    tabWidget->addTab(plain, i18n("CVS &Output"));
    layout->addWidget(tabWidget, 3);

    connect(tabWidget, SIGNAL(currentChanged(QWidget*)),
            this, SLOT(tabChanged(QWidget*)));

    QWhatsThis::add(tree, i18n("Choose revision A by clicking with the left"
                               "mouse button,\nrevision B by clicking with "
                               "the middle mouse button."));

    items.setAutoDelete(true);
    tags.setAutoDelete(true);

    for (int i = 0; i < 2; ++i)
    {
        QFrame *frame = new QFrame(mainWidget);
        frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
        layout->addWidget(frame);

        QGridLayout *grid = new QGridLayout(layout);
        grid->setRowStretch(0, 0);
        grid->setRowStretch(1, 0);
        grid->setRowStretch(2, 1);
        grid->setColStretch(0, 0);
        grid->setColStretch(1, 1);
        grid->setColStretch(2, 0);
        grid->setColStretch(3, 1);
        grid->setColStretch(4, 2);

        QString versionident = (i==0)? i18n("Revision A:") : i18n("Revision B:");
        QLabel *versionlabel = new QLabel(versionident, mainWidget);
        grid->addWidget(versionlabel, 0, 0);

        revbox[i] = new QLabel(mainWidget);
        revbox[i]->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        grid->addWidget(revbox[i], 0, 1, Qt::AlignVCenter);

        QLabel *selectlabel = new QLabel(i18n("Select by tag:"), mainWidget);
        grid->addWidget(selectlabel, 0, 2);

        tagcombo[i] = new QComboBox(mainWidget);
        QFontMetrics fm(tagcombo[i]->fontMetrics());
        tagcombo[i]->setMinimumWidth(fm.width("X")*20);
        grid->addWidget(tagcombo[i], 0, 3);

        QLabel *authorlabel = new QLabel(i18n("Author:"), mainWidget);
        grid->addWidget(authorlabel, 1, 0);

        authorbox[i] = new QLabel(mainWidget);
        authorbox[i]->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        grid->addWidget(authorbox[i], 1, 1);

        QLabel *datelabel = new QLabel(i18n("Date:"), mainWidget);
        grid->addWidget(datelabel, 1, 2);

        datebox[i] = new QLabel(mainWidget);
        datebox[i]->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        grid->addWidget(datebox[i], 1, 3);

        QLabel *commentlabel = new QLabel(i18n("Comment/Tags:"), mainWidget);
        grid->addWidget(commentlabel, 2, 0);

        commentbox[i] = new QTextEdit(mainWidget);
        commentbox[i]->setReadOnly(true);
        commentbox[i]->setTextFormat(Qt::PlainText);
        fm = commentbox[i]->fontMetrics();
        commentbox[i]->setFixedHeight(2*fm.lineSpacing()+10);
        grid->addMultiCellWidget(commentbox[i], 2, 2, 1, 3);

        tagsbox[i] = new QTextEdit(mainWidget);
        tagsbox[i]->setReadOnly(true);
        tagsbox[i]->setFixedHeight(2*fm.lineSpacing()+10);
        grid->addWidget(tagsbox[i], 2, 4);
    }

    QWhatsThis::add(revbox[0], i18n("This revision is used when you click "
                                    "Annotate.\nIt is also used as the first "
                                    "item of a Diff operation."));
    QWhatsThis::add(revbox[1], i18n("This revision is used as the second "
                                    "item of a Diff operation."));

    connect( tagcombo[0], SIGNAL(activated(int)),
             this, SLOT(tagASelected(int)) );
    connect( tagcombo[1], SIGNAL(activated(int)),
             this, SLOT(tagBSelected(int)) );

    connect( this, SIGNAL(user1Clicked()),
             this, SLOT(annotateClicked()) );
    connect( this, SIGNAL(user2Clicked()),
             this, SLOT(diffClicked()) );
    connect( this, SIGNAL(user3Clicked()),
             this, SLOT(findClicked()) );

    setButtonOKText(i18n("to view something", "&View"));

    setHelp("browsinglogs");

    setWFlags(Qt::WDestructiveClose | getWFlags());

#if KDE_IS_VERSION(3,1,90)
    QSize size = configDialogSize(partConfig, "LogDialog");
#else
    QSize size = Cervisia::configDialogSize(this, partConfig, "LogDialog");
#endif
    resize(size);

    KConfigGroupSaver cs(&partConfig, "LogDialog");
    tabWidget->setCurrentPage(partConfig.readNumEntry("ShowTab", 0));
}


LogDialog::~LogDialog()
{
#if KDE_IS_VERSION(3,1,90)
    saveDialogSize(partConfig, "LogDialog");
#else
    Cervisia::saveDialogSize(this, partConfig, "LogDialog");
#endif

    KConfigGroupSaver cs(&partConfig, "LogDialog");
    partConfig.writeEntry("ShowTab", tabWidget->currentPageIndex());
}


bool LogDialog::parseCvsLog(CvsService_stub* service, const QString& fileName)
{
    QString rev;

    Cervisia::LogInfo logInfo;

    enum { Begin, Tags, Admin, Revision,
       Author, Branches, Comment, Finished } state;

    // remember DCOP reference and file name for diff or annotate
    cvsService = service;
    filename = fileName;

    setCaption(i18n("CVS Log: %1").arg(filename));

    DCOPRef job = cvsService->log(filename);
    if( !cvsService->ok() )
        return false;

    ProgressDialog dlg(this, "Logging", job, "log", i18n("CVS Log"));
    if( !dlg.execute() )
        return false;

    // process cvs log output
    state = Begin;
    QString line;
    while( dlg.getLine(line) )
    {
        switch( state )
        {
            case Begin:
                if( line == "symbolic names:" )
                    state = Tags;
                break;
            case Tags:
                if( line[0] == '\t' )
                {
                    const QStringList strlist(splitLine(line, ':'));
                    rev = strlist[1].simplifyWhiteSpace();
                    const QString tag(strlist[0].simplifyWhiteSpace());
                    QString branchpoint;
                    int pos1, pos2;
                    if( (pos2 = rev.findRev('.')) > 0 &&
                        (pos1 = rev.findRev('.', pos2-1)) > 0 &&
                        rev.mid(pos1+1, pos2-pos1-1) == "0" )
                    {
                        // For a branch tag 2.10.0.6, we want:
                        // branchpoint = "2.10"
                        // rev = "2.10.6"
                        branchpoint = rev.left(pos1);
                        rev.remove(pos1+1, pos2-pos1);
                    }
                    if( rev != "1.1.1" )
                    {
                        TagInfo *taginfo = new TagInfo;
                        taginfo->rev = rev;
                        taginfo->tag = tag;
                        taginfo->branchpoint = branchpoint;
                        tags.append(taginfo);
                    }
                }
                else
                {
                    state = Admin;
                }
                break;
            case Admin:
                if( line == "----------------------------" )
                {
                    state = Revision;
                }
                break;
            case Revision:
                logInfo.m_revision = rev = line.section(' ', 1, 1);
                state = Author;
                break;
            case Author:
                {
                    QStringList strlist = splitLine(line);
                    // convert date in ISO format (YYYY-MM-DDTHH:MM:SS)
                    strlist[1].replace('/', '-');
                    strlist[2].truncate(8); // Time foramt is HH:MM:SS
                    logInfo.m_dateTime.setTime_t(KRFCDate::parseDateISO8601(strlist[1] + 'T' + strlist[2]));
                    logInfo.m_author = strlist[4];
                    logInfo.m_author.truncate(logInfo.m_author.length() - 1); // remove trailing ';'
                    state = Branches;
                }
                break;
            case Branches:
                if( !line.startsWith("branches:") )
                {
                    logInfo.m_comment = line;
                    state = Comment;
                }
                break;
            case Comment:
                if( line == "----------------------------" )
                {
                    state = Revision;
                }
                else if( line == "=============================================================================" )
                {
                    state = Finished;
                }
                if( state == Comment ) // still in message
                    logInfo.m_comment += '\n' + line;
                else
                {
                    // Create tagcomment
                    QString branchrev;
                    int pos1, pos2;
                    // 1.60.x.y => revision belongs to branch 1.60.0.x
                    if( (pos2 = rev.findRev('.')) > 0 &&
                        (pos1 = rev.findRev('.', pos2-1)) > 0 )
                        branchrev = rev.left(pos2);

                    // Build Cervisia::TagInfo for logInfo
                    QPtrListIterator<TagInfo> it(tags);
                    for( ; it.current(); ++it )
                    {
                        if( rev == it.current()->rev )
                        {
                            // This never matches branch tags...
                            logInfo.m_tags.push_back(Cervisia::TagInfo(it.current()->tag,
                                                                       Cervisia::TagInfo::Tag));
                        }
                        if( rev == it.current()->branchpoint )
                        {
                            logInfo.m_tags.push_back(Cervisia::TagInfo(it.current()->tag,
                                                                       Cervisia::TagInfo::Branch));
                        }
                        if( branchrev == it.current()->rev )
                        {
                            // ... and this never matches ordinary tags :-)
                            logInfo.m_tags.push_back(Cervisia::TagInfo(it.current()->tag,
                                                                       Cervisia::TagInfo::OnBranch));
                        }
                    }

                    plain->addRevision(logInfo);
                    tree->addRevision(logInfo);
                    list->addRevision(logInfo);

                    items.append(new Cervisia::LogInfo(logInfo));

                    // reset for next entry
                    logInfo = Cervisia::LogInfo();
                }
                break;
            case Finished:
                ;
        }
    }

    tagcombo[0]->insertItem(QString::null);
    tagcombo[1]->insertItem(QString::null);
    QPtrListIterator<TagInfo> it(tags);
    for( ; it.current(); ++it )
    {
        QString str = it.current()->tag;
        if( !it.current()->branchpoint.isEmpty() )
            str += i18n(" (Branchpoint)");
        tagcombo[0]->insertItem(str);
        tagcombo[1]->insertItem(str);
    }

    plain->scrollToTop();

    tree->collectConnections();
    tree->recomputeCellSizes();

    return true;    // successful
}


void LogDialog::slotOk()
{
    // make sure that the user selected a revision
    if( selectionA.isEmpty() && selectionB.isEmpty() )
    {
        KMessageBox::information(this,
            i18n("Please select revision A or B first."), "Cervisia");
        return;
    }

    // retrieve the selected revision
    QString revision;
    if( !selectionA.isEmpty() )
        revision = selectionA;
    else
        revision = selectionB;

    // create a temporary file
    const QString suffix("-" + revision + "-" + filename);
    KTempFile file(QString::null, suffix);

    // create a command line to retrieve the file revision from
    // cvs and save it into the temporary file
    // FIXME CL hardcoded call to cvs command-line client!!!
    QString cmdline = "cvs update -p -r ";
    cmdline += KProcess::quote(revision);
    cmdline += " ";
    cmdline += KProcess::quote(filename);
    cmdline += " > ";
    cmdline += file.name();

    // FIXME: We shouldn't need to use CvsProgressDialog here
    Repository_stub cvsRepository(cvsService->app(), "CvsRepository");
    QString sandbox    = cvsRepository.workingCopy();
    QString repository = cvsRepository.location();

    CvsProgressDialog dlg("View", this);
    if( dlg.execCommand(sandbox, repository, cmdline, "view", &partConfig) )
    {
        // make file read-only
        chmod(QFile::encodeName(file.name()), 0400);

        // open file in preferred editor
        QDir dir(sandbox);
        KURL url;
        url.setPath(file.name());
        (void) new KRun(url, 0, true, false);
    }

    file.close();
}


void LogDialog::findClicked()
{
    KFindDialog dlg(this);
    if( dlg.exec() == KDialogBase::Accepted )
        plain->searchText(dlg.options(), dlg.pattern());
}


void LogDialog::diffClicked()
{
    if (selectionA.isEmpty())
    {
        KMessageBox::information(this,
            i18n("Please select revision A or revisions A and B first."),
            "Cervisia");
        return;
    }

    // Non-modal dialog
    DiffDialog *l = new DiffDialog(partConfig);
    if (l->parseCvsDiff(cvsService, filename, selectionA, selectionB))
        l->show();
    else
        delete l;
}


void LogDialog::annotateClicked()
{
    AnnotateDialog *l = new AnnotateDialog(partConfig);
    AnnotateController ctl(l, cvsService);
    ctl.showDialog(filename, selectionA);
}


void LogDialog::revisionSelected(QString rev, bool rmb)
{
    QPtrListIterator<Cervisia::LogInfo> it(items);
    for (; it.current(); ++it)
        if (it.current()->m_revision == rev)
            {
                if (rmb)
                    selectionB = rev;
                else
                    selectionA = rev;

                revbox[rmb?1:0]->setText(rev);
                authorbox[rmb?1:0]->setText(it.current()->m_author);
                datebox[rmb?1:0]->setText(it.current()->dateTimeToString());
                commentbox[rmb?1:0]->setText(it.current()->m_comment);
                tagsbox[rmb?1:0]->setText(it.current()->tagsToString());

                tree->setSelectedPair(selectionA, selectionB);
                list->setSelectedPair(selectionA, selectionB);
                return;
            }
    kdDebug(8050) << "Internal error: Revision not found " << rev << "." << endl;
}


void LogDialog::tagSelected(TagInfo* tag, bool rmb)
{
    if (tag->branchpoint.isEmpty())
        revisionSelected(tag->rev, rmb);
    else
        revisionSelected(tag->branchpoint, rmb);
}


void LogDialog::tagASelected(int n)
{
    if (n)
        tagSelected(tags.at(n-1), false);
}


void LogDialog::tagBSelected(int n)
{
    if (n)
        tagSelected(tags.at(n-1), true);
}


void LogDialog::tabChanged(QWidget* w)
{
    bool isPlainView = (w == plain);
    showButton(User3, isPlainView);
}

#include "logdlg.moc"

// Local Variables:
// c-basic-offset: 4
// End:
