/*
 *  Copyright (C) 1999-2001 Bernd Gehrmann
 *                          bernd@physik.hu-berlin.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include <stdio.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qwidgetstack.h>
#include <qwhatsthis.h>
#include <kbuttonbox.h>
#include <kdebug.h>
#include <kapp.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kprocess.h>
#include <diffdlg.h>
#include <annotatedlg.h>
#include "cvsprogressdlg.h"
#include "misc.h"

#include "logdlg.h"
#include "logdlg.moc"


LogDialog::Options *LogDialog::options = 0;


LogDialog::LogDialog(QWidget *parent, const char *name)
    : QDialog(parent, name, false, WStyle_MinMax)
{
    QBoxLayout *layout = new QVBoxLayout(this, 10, 0);

    tree = new LogTreeView(this);
    connect( tree, SIGNAL(revisionClicked(QString,bool)),
	     this, SLOT(revisionSelected(QString,bool)) );

    list = new LogListView(this);
    connect( list, SIGNAL(revisionClicked(QString,bool)),
	     this, SLOT(revisionSelected(QString,bool)) );

    QWidgetStack *stack = new QWidgetStack(this);
#if QT_VERSION < 300
    stack->setPalettePropagation(NoChildren);
#endif
    tabbar = new QTabBar(this);
    QTab *tab1 = new QTab(i18n("&Tree"));
    stack->addWidget(tree, tabbar->addTab(tab1));
    QTab *tab2 = new QTab(i18n("&List"));
    stack->addWidget(list, tabbar->addTab(tab2));

    tabbar->adjustSize();
    tabbar->setMinimumWidth(tabbar->width());
    tabbar->setFixedHeight(tabbar->height());
    layout->addWidget(tabbar, 0);
    layout->addWidget(stack, 3);
    
    connect( tabbar, SIGNAL(selected(int)),
	     stack, SLOT(raiseWidget(int)) );

    QWhatsThis::add(tree, i18n("Choose revision A by clicking with the left"
			       "mouse button,\nrevision B by clicking with "
			       "the middle mouse button."));
    
    items.setAutoDelete(true);
    tags.setAutoDelete(true);

    for (int i = 0; i < 2; ++i)
	{
	    QFrame *frame = new QFrame(this);
	    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
            //	    frame->setMinimumHeight(frame->sizeHint().height());
	    layout->addSpacing(8);
	    layout->addWidget(frame);
	    layout->addSpacing(8);

	    QGridLayout *grid = new QGridLayout(3, 5, 4);
	    layout->addLayout(grid, 1);
            grid->setRowStretch(0, 0);
            grid->setRowStretch(1, 0);
            grid->setRowStretch(2, 1);
	    grid->setColStretch(0, 0);
	    grid->setColStretch(1, 1);
	    grid->setColStretch(2, 0);
	    grid->setColStretch(3, 1);
	    grid->setColStretch(4, 2);
	    
	    QString versionident = (i==0)? i18n("Revision A:") : i18n("Revision B:");
	    QLabel *versionlabel = new QLabel(versionident, this);
	    grid->addWidget(versionlabel, 0, 0);

	    revbox[i] = new QLabel("1.0.1.0.1.0", this);
            revbox[i]->setMinimumSize(revbox[i]->sizeHint());
	    revbox[i]->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	    revbox[i]->setText("");
	    grid->addWidget(revbox[i], 0, 1);
            
            QLabel *selectlabel = new QLabel(i18n("Select by tag:"), this);
            grid->addWidget(selectlabel, 0, 2);
            
            tagcombo[i] = new QComboBox(this);
            QFontMetrics fm(tagcombo[i]->fontMetrics());
            tagcombo[i]->setMinimumWidth(fm.width("X")*20);
            grid->addWidget(tagcombo[i], 0, 3);
            
	    QLabel *authorlabel = new QLabel(i18n("Author:"), this);
	    grid->addWidget(authorlabel, 1, 0);

	    authorbox[i] = new QLabel("Foo", this);
	    authorbox[i]->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	    authorbox[i]->setText("");
	    grid->addWidget(authorbox[i], 1, 1);

	    QLabel *datelabel = new QLabel(i18n("Date:"), this);
	    grid->addWidget(datelabel, 1, 2);

	    datebox[i] = new QLabel("1999/99/99 00:00:00", this);
	    datebox[i]->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	    datebox[i]->setText("");
	    grid->addWidget(datebox[i], 1, 3);

	    QLabel *commentlabel = new QLabel(i18n("Comment/Tags:"), this);
	    grid->addWidget(commentlabel, 2, 0);

            commentbox[i] = new QTextEdit(this);
	    commentbox[i]->setReadOnly(true);
	    commentbox[i]->setMinimumSize(commentbox[i]->sizeHint().width(),
                commentbox[i]->sizeHint().height()-50);
	    grid->addMultiCellWidget(commentbox[i], 2, 2, 1, 3);

            tagsbox[i] = new QTextEdit(this);
	    tagsbox[i]->setReadOnly(true);
	    tagsbox[i]->setMinimumSize(tagsbox[i]->sizeHint().width(),
                tagsbox[i]->sizeHint().height()-50);
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

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addSpacing(8);
    layout->addWidget(frame, 0);
    layout->addSpacing(8);

    KButtonBox *buttonbox = new KButtonBox(this);
    connect( buttonbox->addButton(i18n("&Diff")), SIGNAL(clicked()),
	     SLOT(diffClicked()) );
    connect( buttonbox->addButton(i18n("&Annotate")), SIGNAL(clicked()),
	     SLOT(annotateClicked()) );
    buttonbox->addStretch();
    connect( buttonbox->addButton(i18n("&Close")), SIGNAL(clicked()),
	     SLOT(reject()) );
    buttonbox->layout();
    layout->addWidget(buttonbox, 0);

    if (options)
        {
            resize(options->size);
            if (options->showlisttab)
                tabbar->setCurrentTab(1);
        }
}


void LogDialog::done(int res)
{
    if (!options)
        options = new Options;
    options->size = size();
    options->showlisttab = tabbar->currentTab() == 1;
    
    QDialog::done(res);
    delete this;
}


void LogDialog::loadOptions(KConfig *config)
{
    if (!config->readEntry("Customized"))
        return;

    options = new Options;
    options->size = config->readSizeEntry("Size");
    options->showlisttab = config->readBoolEntry("ShowListTab");
}


void LogDialog::saveOptions(KConfig *config)
{
    if (!options)
        return;

    config->writeEntry("Customized", true);
    config->writeEntry("Size", options->size);
    config->writeEntry("ShowListTab", options->showlisttab);
}


bool LogDialog::parseCvsLog(const QString &sbox, const QString &repo, const QString &fname)
{
    QStringList strlist;
    QString tag, rev, date, author, comment;
    enum { Begin, Tags, Admin, Revision,
	   Author, Branches, Comment, Finished } state;

    sandbox = sbox;
    repository = repo;
    filename = fname;
    
    setCaption(i18n("CVS Log: ") + filename);

    QString cmdline = cvsClient(repository) + " -f log ";
    cmdline += KShellProcess::quote(filename);
    
    CvsProgressDialog l("Logging", this);
    l.setCaption(i18n("CVS Log"));
    if (!l.execCommand(sandbox, repository, cmdline, "log"))
        return false;

    state = Begin;
    QCString line;
    while ( l.getOneLine(&line) )
        {
            switch (state)
                {
                case Begin:
                    if (line == "symbolic names:")
                        state = Tags;
                    break;
                case Tags:
                    if (line[0] == '\t')
                        {
                            strlist = splitLine(line, ':');
                            QString rev = strlist[1].simplifyWhiteSpace();
                            QString tag = strlist[0].simplifyWhiteSpace();
                            QString branchpoint = "";
                            int pos1, pos2;
                            if ( (pos2 = rev.findRev('.')) > 0 &&
                                 (pos1 = rev.findRev('.', pos2-1)) > 0 &&
                                 rev.mid(pos1+1, pos2-pos1-1) == "0" )
                                {
                                    // For a branch tag 2.10.0.6, we want:
                                    // branchpoint = "2.10"
                                    // rev = "2.10.6"
                                    branchpoint = rev.left(pos1);
                                    rev.remove(pos1+1, pos2-pos1);
                                }
                            if (rev != "1.1.1")
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
                    if (line == "----------------------------")
                        {
                            state = Revision;
                        }
                    break;
                case Revision:
                    strlist = splitLine(line);
                    rev = strlist[1];
                    state = Author;
                    break;
                case Author:
                    strlist = splitLine(line);
                    date = strlist[2];
                    date = strlist[1] + " " + date.left(date.length()-1);
                    author = strlist[4];
                    author = author.left(author.length()-1);
                    comment = "";
                    state = Branches;
                    break;
                case Branches:
                    if (qstrncmp(line, "branches:", 9) != 0)
                        {
                            comment = line;
                            state = Comment;
                        }
                    break;
                case Comment:
                    if (line == "----------------------------")
                        {
                            state = Revision;
                        }
                    else if (line == "=============================================================================")
                        {
                            state = Finished;
                        }
                    if (state == Comment) // still in message
                        comment += QString("\n") + QString(line);
                    else 
                        {
                            // Create tagcomment
                            QString tagcomment, taglist, branchrev;
                            int pos1, pos2;
                            // 1.60.x.y => revision belongs to branch 1.60.0.x
                            if ( (pos2 = rev.findRev('.')) > 0 &&
                                 (pos1 = rev.findRev('.', pos2-1)) > 0)
                                branchrev = rev.left(pos2);
                            
                            // Build tagcomment and taglist:
                            // tagcomment contains Tags, Branchpoints and 'On Branch's
                            // taglist contains tags (without prefix) and Branchpoints
                            QPtrListIterator<TagInfo> it(tags);
                            for (; it.current(); ++it)
                                {
                                    if (rev == it.current()->rev)
                                        {
                                            // This never matches branch tags...
                                            tagcomment += i18n("\nTag: ");
                                            tagcomment += it.current()->tag;
                                            taglist += "\n";
                                            taglist += it.current()->tag;
                                        }
                                    if (rev == it.current()->branchpoint)
                                        {
                                            tagcomment += i18n("\nBranchpoint: ");
                                            tagcomment += it.current()->tag;
                                            taglist += i18n("\nBranchpoint: ");
                                            taglist += it.current()->tag;
                                        }
                                    if (branchrev == it.current()->rev)
                                        {
                                            // ... and this never matches ordinary tags :-)
                                            tagcomment += i18n("\nOn branch: ");
                                            tagcomment += it.current()->tag;
                                        }
                                }
                            
                            // remove leading '\n'
                            if (!tagcomment.isEmpty())
                                tagcomment.remove(0, 1);
                            if (!taglist.isEmpty())
                                taglist.remove(0, 1);
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
    for (; it.current(); ++it)
        {
	    QString str = it.current()->tag;
            if (!it.current()->branchpoint.isEmpty())
                str += i18n(" (Branchpoint)");
            tagcombo[0]->insertItem(str);
            tagcombo[1]->insertItem(str);
        }

    tree->collectConnections();
    tree->recomputeCellSizes();
    layout()->activate();

    return true; // successful
}


void LogDialog::diffClicked()
{
    if (selectionA.isEmpty() || selectionB.isEmpty())
	{
	    KMessageBox::information(this,
				     i18n("Please select revisions A and B first."),
				     "Cervisia");
	    return;
	}

    // Non-modal dialog
    DiffDialog *l = new DiffDialog();
    if (l->parseCvsDiff(sandbox, repository, filename, selectionA, selectionB))
        l->show();
    else
        delete l;
}


void LogDialog::annotateClicked()
{
    AnnotateDialog *l = new AnnotateDialog();
    if (l->parseCvsAnnotate(sandbox, repository, filename, selectionA))
        l->show();
    else
        delete l;
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
                datebox[rmb?1:0]->setText(it.current()->date);
                commentbox[rmb?1:0]->setText(it.current()->comment);
                tagsbox[rmb?1:0]->setText(it.current()->tagcomment);
                
                tree->setSelectedPair(selectionA, selectionB);
                list->setSelectedPair(selectionA, selectionB);
                return;
            }
    kdDebug() << "Internal error: Revision not found " << rev << "." << endl;
}


void LogDialog::tagSelected(QString tag, bool rmb)
{
    QPtrListIterator<TagInfo> it(tags);
    for (; it.current(); ++it)
        if (tag == it.current()->tag)
            {
                if (it.current()->branchpoint.isEmpty())
                    revisionSelected(it.current()->rev, rmb);
                else
                    revisionSelected(it.current()->branchpoint, rmb);
                return;
            }
}


void LogDialog::tagASelected(int n)
{
    if (n)
        tagSelected(tags.at(n-1)->tag, false);
}


void LogDialog::tagBSelected(int n)
{
    if (n)
        tagSelected(tags.at(n-1)->tag, true);
}

// Local Variables:
// c-basic-offset: 4
// End:
