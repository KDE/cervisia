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


#include "logdlg.h"

#include <stdio.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtabwidget.h>
#include <qtextedit.h>
#include <qwhatsthis.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kprocess.h>
#include <krfcdate.h>

#include "cvsservice_stub.h"
#include "annotatedlg.h"
#include "annotatectl.h"
#include "diffdlg.h"
#include "loglist.h"
#include "logtree.h"
#include "logplainview.h"
#include "misc.h"
#include "progressdlg.h"

#include <kdeversion.h>
#if KDE_VERSION < KDE_MAKE_VERSION(3,1,90)
#include "configutils.h"
#endif


LogDialog::LogDialog(KConfig& cfg, QWidget *parent, const char *name)
    : KDialogBase(parent, name, false, QString::null,
                  Close | Help | User1 | User2, Close, true)
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
    tabWidget->addTab(plain, i18n("&CVS Output"));
    layout->addWidget(tabWidget, 3);

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

    setButtonText(User1, i18n("&Annotate"));
    setButtonText(User2, i18n("&Diff"));

    connect( this, SIGNAL(user1Clicked()),
             this, SLOT(annotateClicked()) );
    connect( this, SIGNAL(user2Clicked()),
             this, SLOT(diffClicked()) );

    setHelp("browsinglogs");

    setWFlags(Qt::WDestructiveClose | getWFlags());

#if KDE_IS_VERSION(3,1,90)
    QSize size = configDialogSize(partConfig, "LogDialog");
#else
    QSize size = Cervisia::configDialogSize(this, partConfig, "LogDialog");
#endif
    resize(size);

    KConfigGroupSaver cs(&partConfig, "LogDialog");
    if (partConfig.readBoolEntry("ShowListTab"))
        tabWidget->setCurrentPage(1);
}


LogDialog::~LogDialog()
{
#if KDE_IS_VERSION(3,1,90)
    saveDialogSize(partConfig, "LogDialog");
#else
    Cervisia::saveDialogSize(this, partConfig, "LogDialog");
#endif

    KConfigGroupSaver cs(&partConfig, "LogDialog");
    bool showListTab = (tabWidget->currentPageIndex() == 1);
    partConfig.writeEntry("ShowListTab", showListTab);
}


bool LogDialog::parseCvsLog(CvsService_stub* service, const QString& fileName)
{
    QString tag, rev, author, comment;
    QDateTime date;
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
                    QStringList strlist = splitLine(line, ':');
                    QString rev = strlist[1].simplifyWhiteSpace();
                    QString tag = strlist[0].simplifyWhiteSpace();
                    QString branchpoint = "";
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
                rev = line.section(' ', 1, 1);
                state = Author;
                break;
            case Author:
                {
                    QStringList strlist = splitLine(line);
                    // convert date in ISO format (YYYY-MM-DDTHH:MM:SS)
                    strlist[1].replace('/', '-');
                    strlist[2].truncate(8); // Time foramt is HH:MM:SS
                    date.setTime_t(KRFCDate::parseDateISO8601(strlist[1] + 'T' + strlist[2]));
                    author = strlist[4];
                    author = author.left(author.length()-1);
                    comment = "";
                    state = Branches;
                }
                break;
            case Branches:
                if( !line.startsWith("branches:") )
                {
                    comment = line;
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
                    comment += QString("\n") + line;
                else
                {
                    // Create tagcomment
                    QString tagcomment, taglist, branchrev;
                    int pos1, pos2;
                    // 1.60.x.y => revision belongs to branch 1.60.0.x
                    if( (pos2 = rev.findRev('.')) > 0 &&
                        (pos1 = rev.findRev('.', pos2-1)) > 0 )
                        branchrev = rev.left(pos2);

                    // Build tagcomment and taglist:
                    // tagcomment contains Tags, Branchpoints and 'On Branch's
                    // taglist contains tags (without prefix) and Branchpoints
                    QPtrListIterator<TagInfo> it(tags);
                    for( ; it.current(); ++it )
                    {
                        if( rev == it.current()->rev )
                        {
                            // This never matches branch tags...
                            tagcomment += i18n("\nTag: ");
                            tagcomment += it.current()->tag;
                            taglist += "\n";
                            taglist += it.current()->tag;
                        }
                        if( rev == it.current()->branchpoint )
                        {
                            tagcomment += i18n("\nBranchpoint: ");
                            tagcomment += it.current()->tag;
                            taglist += i18n("\nBranchpoint: ");
                            taglist += it.current()->tag;
                        }
                        if( branchrev == it.current()->rev )
                        {
                            // ... and this never matches ordinary tags :-)
                            tagcomment += i18n("\nOn branch: ");
                            tagcomment += it.current()->tag;
                        }
                    }

                    // remove leading '\n'
                    if( !tagcomment.isEmpty() )
                        tagcomment.remove(0, 1);
                    if( !taglist.isEmpty() )
                        taglist.remove(0, 1);
                    plain->addRevision(rev, author, date, comment, tagcomment);
                    tree->addRevision(rev, author, date, comment, taglist, tagcomment);
                    list->addRevision(rev, author, date, comment, tagcomment);
                    RevisionInfo *item = new RevisionInfo;
                    item->rev = rev;
                    item->author = author;
                    item->date = date;
                    item->comment = comment;
                    item->tagcomment = tagcomment;
                    items.append(item);
                }
                break;
            case Finished:
                ;
        }
    }

    tagcombo[0]->insertItem("");
    tagcombo[1]->insertItem("");
    QPtrListIterator<TagInfo> it(tags);
    for( ; it.current(); ++it )
    {
        QString str = it.current()->tag;
        if( !it.current()->branchpoint.isEmpty() )
            str += i18n(" (Branchpoint)");
        tagcombo[0]->insertItem(str);
        tagcombo[1]->insertItem(str);
    }

    tree->collectConnections();
    tree->recomputeCellSizes();

    return true;    // successful
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
    QPtrListIterator<RevisionInfo> it(items);
    for (; it.current(); ++it)
        if (it.current()->rev == rev)
            {
                if (rmb)
                    selectionB = rev;
                else
                    selectionA = rev;
                
                revbox[rmb?1:0]->setText(rev);
                authorbox[rmb?1:0]->setText(it.current()->author);
                datebox[rmb?1:0]->setText(KGlobal::locale()->formatDateTime(it.current()->date));
                commentbox[rmb?1:0]->setText(it.current()->comment);
                tagsbox[rmb?1:0]->setText(it.current()->tagcomment);
                
                tree->setSelectedPair(selectionA, selectionB);
                list->setSelectedPair(selectionA, selectionB);
                return;
            }
    kdDebug() << "Internal error: Revision not found " << rev << "." << endl;
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

#include "logdlg.moc"

// Local Variables:
// c-basic-offset: 4
// End:
