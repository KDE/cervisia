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


#include "logdlg.h"

#include <qcombobox.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qtextedit.h>
#include <qwhatsthis.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kfinddialog.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klistviewsearchline.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <krfcdate.h>
#include <krun.h>
#include <kurl.h>

#include "cvsservice_stub.h"
#include "annotatedlg.h"
#include "annotatectl.h"
#include "diffdlg.h"
#include "loginfo.h"
#include "loglist.h"
#include "logplainview.h"
#include "logtree.h"
#include "misc.h"
#include "progressdlg.h"
#include "patchoptiondlg.h"


LogDialog::LogDialog(KConfig& cfg, QWidget *parent, const char *name)
    : KDialogBase(parent, name, false, QString::null,
                  Ok | Apply | Close | Help | User1 | User2 | User3, Close, true,
                  KGuiItem(i18n("&Annotate")),
                  KGuiItem(i18n("&Diff"), "vcs_diff"),
                  KGuiItem(i18n("&Find..."), "find"))
    , cvsService(0)
    , partConfig(cfg)
{
    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    setMainWidget(splitter);

    tree = new LogTreeView(this);
    connect( tree, SIGNAL(revisionClicked(QString,bool)),
             this, SLOT(revisionSelected(QString,bool)) );

    QWidget* listWidget = new QWidget(this);
    QVBoxLayout* listLayout = new QVBoxLayout(listWidget);
    QHBoxLayout* searchLayout = new QHBoxLayout(listLayout);
    searchLayout->setMargin(KDialog::spacingHint());
    searchLayout->setSpacing(KDialog::spacingHint());

    list = new LogListView(partConfig, listWidget);
    listLayout->addWidget(list, 1);

    KListViewSearchLine* searchLine = new KListViewSearchLine(listWidget, list);
    QLabel* searchLabel = new QLabel(searchLine, i18n("S&earch:"), listWidget);
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchLine, 1);

    connect( list, SIGNAL(revisionClicked(QString,bool)),
             this, SLOT(revisionSelected(QString,bool)) );

    plain = new LogPlainView(this);
    connect( plain, SIGNAL(revisionClicked(QString,bool)),
             this, SLOT(revisionSelected(QString,bool)) );

    tabWidget = new QTabWidget(splitter);
    tabWidget->addTab(tree, i18n("&Tree"));
    tabWidget->addTab(listWidget, i18n("&List"));
    tabWidget->addTab(plain, i18n("CVS &Output"));

    connect(tabWidget, SIGNAL(currentChanged(QWidget*)),
            this, SLOT(tabChanged(QWidget*)));

    QWhatsThis::add(tree, i18n("Choose revision A by clicking with the left "
                               "mouse button,\nrevision B by clicking with "
                               "the middle mouse button."));

    items.setAutoDelete(true);
    tags.setAutoDelete(true);

    QWidget *mainWidget = new QWidget(splitter);
    QBoxLayout *layout = new QVBoxLayout(mainWidget, 0, spacingHint());

    for (int i = 0; i < 2; ++i)
    {
        if ( i == 1 )
        {
            QFrame *frame = new QFrame(mainWidget);
            frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
            layout->addWidget(frame);
        }

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
        commentbox[i]->setMinimumHeight(2*fm.lineSpacing()+10);
        grid->addMultiCellWidget(commentbox[i], 2, 2, 1, 3);

        tagsbox[i] = new QTextEdit(mainWidget);
        tagsbox[i]->setReadOnly(true);
        tagsbox[i]->setMinimumHeight(2*fm.lineSpacing()+10);
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

    setButtonGuiItem(Ok, KGuiItem(i18n("to view something", "&View"),"fileopen"));
    setButtonGuiItem(Apply, KGuiItem(i18n("Create Patch...")));
    setHelp("browsinglogs");

    setWFlags(Qt::WDestructiveClose | getWFlags());

    QSize size = configDialogSize(partConfig, "LogDialog");
    resize(size);

    KConfigGroupSaver cs(&partConfig, "LogDialog");
    tabWidget->setCurrentPage(partConfig.readNumEntry("ShowTab", 0));

    updateButtons();
}


LogDialog::~LogDialog()
{
    saveDialogSize(partConfig, "LogDialog");

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
                        LogDialogTagInfo *taginfo = new LogDialogTagInfo;
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
                    QStringList strList = QStringList::split(";", line);

                    // convert date into ISO format (YYYY-MM-DDTHH:MM:SS)
                    int len = strList[0].length();
                    QString dateTimeStr = strList[0].right(len-6); // remove 'date: '
                    dateTimeStr.replace('/', '-');

                    QString date = dateTimeStr.section(' ', 0, 0);
                    QString time = dateTimeStr.section(' ', 1, 1);
                    logInfo.m_dateTime.setTime_t(KRFCDate::parseDateISO8601(date + 'T' + time));

                    logInfo.m_author = strList[1].section(':', 1, 1).stripWhiteSpace();

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
                    QPtrListIterator<LogDialogTagInfo> it(tags);
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
    QPtrListIterator<LogDialogTagInfo> it(tags);
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
    const QString suffix("-" + revision + "-" + QFileInfo(filename).fileName());
    const QString tempFileName(::tempFileName(suffix));

    // retrieve the file with the selected revision from cvs
    // and save the content into the temporary file
    DCOPRef job = cvsService->downloadRevision(filename, revision, tempFileName);
    if( !cvsService->ok() )
        return;

    ProgressDialog dlg(this, "View", job, "view", i18n("View File"));
    if( dlg.execute() )
    {
        // make file read-only
        chmod(QFile::encodeName(tempFileName), 0400);

        // open file in preferred editor
        KURL url;
        url.setPath(tempFileName);
        (void) new KRun(url, 0, true, false);
    }
}


void LogDialog::slotApply()
{
    if( selectionA.isEmpty() )
    {
        KMessageBox::information(this,
            i18n("Please select revision A or revisions A and B first."),
            "Cervisia");
        return;
    }
    
    Cervisia::PatchOptionDialog optionDlg;
    if( optionDlg.exec() == KDialogBase::Rejected )
        return;
    
    QString format      = optionDlg.formatOption();
    QString diffOptions = optionDlg.diffOptions();
    
    DCOPRef job = cvsService->diff(filename, selectionA, selectionB, diffOptions,
                                   format);
    if( !cvsService->ok() )
        return;

    ProgressDialog dlg(this, "Diff", job, "", i18n("CVS Diff"));
    if( !dlg.execute() )
        return;

    QString fileName = KFileDialog::getSaveFileName();
    if( fileName.isEmpty() )
        return;

    if( !Cervisia::CheckOverwrite(fileName) )
        return;

    QFile f(fileName);
    if( !f.open(IO_WriteOnly) )
    {
        KMessageBox::sorry(this,
                           i18n("Could not open file for writing."),
                           "Cervisia");
        return;
    }

    QTextStream t(&f);
    QString line;
    while( dlg.getLine(line) )
        t << line << '\n';

    f.close();    
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

                updateButtons();
                return;
            }
    kdDebug(8050) << "Internal error: Revision not found " << rev << "." << endl;
}


void LogDialog::tagSelected(LogDialogTagInfo* tag, bool rmb)
{
    if (tag->branchpoint.isEmpty())
        revisionSelected(tag->rev, rmb);
    else
        revisionSelected(tag->branchpoint, rmb);
}


void LogDialog::updateButtons()
{
    // no versions selected?
    if( selectionA.isEmpty() && selectionB.isEmpty() )
    {
        enableButton(User1, true);  // annotate
        enableButton(User2, false); // diff
        enableButtonOK(false);      // view
        enableButtonApply(false);   // create patch
    }
    // both versions selected?
    else if( !selectionA.isEmpty() && !selectionB.isEmpty() )
    {
        enableButton(User1, false); // annotate
        enableButton(User2, true);  // diff
        enableButtonOK(false);      // view
        enableButtonApply(true);    // create patch
    }
    // only single version selected?
    else
    {
        enableButton(User1, true);  // annotate
        enableButton(User2, true);  // diff
        enableButtonOK(true);       // view
        enableButtonApply(true);    // create patch
    }
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
