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

#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpainter.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qkeycode.h>
#include <qfileinfo.h>
#include <kbuttonbox.h>
#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <ktempfile.h>
#include <kprocess.h>
#include "misc.h"
#include "cervisiapart.h"
#include "cvsprogressdlg.h"
#include "diffview.h"
#include "diffdlg.h"
#include "diffdlg.moc"


DiffDialog::Options *DiffDialog::options = 0;


DiffDialog::DiffDialog(QWidget *parent, const char *name, bool modal)
    : QDialog(parent, name, modal, WStyle_MinMax)
{
    items.setAutoDelete(true);
    markeditem = -1;

    QBoxLayout *layout = new QVBoxLayout(this, 10);

    QGridLayout *pairlayout = new QGridLayout(2, 3, 10);
    layout->addLayout(pairlayout, 10);
    pairlayout->setRowStretch(0, 0);
    pairlayout->setRowStretch(1, 1);
    pairlayout->setColStretch(1, 0);
    pairlayout->addColSpacing(1, 16);
    pairlayout->setColStretch(0, 10);
    pairlayout->setColStretch(2, 10);

    revlabel1 = new QLabel("Rev A", this);
    pairlayout->addWidget(revlabel1, 0, 0);
			      
    revlabel2 = new QLabel("Rev A", this);
    pairlayout->addWidget(revlabel2, 0, 2);

    diff1 = new DiffView(true, false, this);
    diff2 = new DiffView(true, true, this);
    DiffZoomWidget *zoom = new DiffZoomWidget(this);
    zoom->setDiffView(diff2);

    pairlayout->addWidget(diff1, 1, 0);
    pairlayout->addWidget(zoom,  1, 1);
    pairlayout->addWidget(diff2, 1, 2);

    diff1->setPartner(diff2);
    diff2->setPartner(diff1);
    
    syncbox = new QCheckBox(i18n("Synchronize scroll bars"), this);
    syncbox->setChecked(true);
    connect( syncbox, SIGNAL(toggled(bool)),
	     this, SLOT(toggleSynchronize(bool)) );
    syncbox->setMinimumSize(syncbox->sizeHint());

    itemscombo = new QComboBox(this);
    itemscombo->insertItem("");
    connect( itemscombo, SIGNAL(activated(int)),
             this, SLOT(comboActivated(int)) );
    
    nofnlabel = new QLabel(this);
    nofnlabel->setAlignment(AlignCenter);
    
    backbutton = new QPushButton("&<<", this);
    connect( backbutton, SIGNAL(clicked()), SLOT(backClicked()) );
    
    forwbutton = new QPushButton("&>>", this);
    connect( forwbutton, SIGNAL(clicked()), SLOT(forwClicked()) );

    QBoxLayout *buttonlayout = new QHBoxLayout();
    layout->addLayout(buttonlayout);
    buttonlayout->addWidget(syncbox, 0);
    buttonlayout->addStretch(4);
    buttonlayout->addWidget(itemscombo);
    buttonlayout->addStretch(1);
    buttonlayout->addWidget(nofnlabel);
    buttonlayout->addStretch(1);
    buttonlayout->addWidget(backbutton);
    buttonlayout->addWidget(forwbutton);
    
    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    frame->setMinimumHeight(frame->sizeHint().height());
    layout->addWidget(frame, 0);

    KButtonBox *buttonbox = new KButtonBox(this);
    buttonbox->addStretch();
    connect( buttonbox->addButton(i18n("&Close")), SIGNAL(clicked()),
	     SLOT(reject()) );
    buttonbox->layout();
    layout->addWidget(buttonbox, 0);

    QFontMetrics fm(fontMetrics());
    setMinimumSize(fm.width("0123456789")*12,
		   fm.lineSpacing()*30);

    if (options)
        {
            resize(options->size);
            syncbox->setChecked(options->sync);
        }
}


void DiffDialog::done(int res)
{
    if (!options)
        options = new Options;
    options->size = size();
    options->sync = syncbox->isChecked();
    
    QDialog::done(res);
    delete this;
}


void DiffDialog::loadOptions(KConfig *config)
{
    if (!config->readEntry("Customized"))
        return;

    options = new Options;
    options->size = config->readSizeEntry("Size");
    options->sync = config->readBoolEntry("Sync");
}


void DiffDialog::saveOptions(KConfig *config)
{
    if (!options)
        return;

    config->writeEntry("Customized", true);
    config->writeEntry("Size", options->size);
    config->writeEntry("Sync", options->sync);
}


void DiffDialog::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
	{
	case Key_Up:
            diff1->up();
            diff2->up();
            break;
	case Key_Down:
            diff1->down();
            diff2->down();
            break;
	case Key_Next:
            diff1->next();
            diff2->next();
            break;
	case Key_Prior:
            diff1->prior();
            diff2->prior();
            break;
        default:
            QDialog::keyPressEvent(e);
	}
}


void DiffDialog::toggleSynchronize(bool b)
{
    diff1->setPartner(b? diff2 : 0);
    diff2->setPartner(b? diff1 : 0);
}


void DiffDialog::comboActivated(int index)
{
    updateHighlight(index-1);
}


static void interpretRegion(QString line, int *linenoA, int *linenoB)
{
    //  No KRegExp in KDE1 :-(
    line.remove(0, 2);
    int pos1, pos2;
    if ( (pos1 = line.find('-')) == -1)
        return;
    pos1++;
    if ( (pos2 = line.find(',', pos1)) == -1)
        return;
    pos2++;
    *linenoA = line.mid(pos1, pos2-pos1-1).toInt()-1;
    if ( (pos1 = line.find('+'), pos2) == -1)
        return;
    pos1++;
    if ( (pos2 = line.find(',', pos1)) == -1)
        return;
    pos2++;
    *linenoB = line.mid(pos1, pos2-pos1-1).toInt()-1;
    if ( (pos1 = line.find('@'), pos2) == -1)
        return;
}


static QString regionAsString(int linenoA, int linecountA, int linenoB, int linecountB)
{
    int lineendA = linenoA+linecountA-1;
    int lineendB = linenoB+linecountB-1;
    QString res;
    if (linecountB == 0)
        res = QString("%1,%2d%3").arg(linenoA).arg(lineendA).arg(linenoB-1);
    else if (linecountA == 0)
        res = QString("%1a%2,%3").arg(linenoA-1).arg(linenoB).arg(lineendB);
    else if (linenoA == lineendA)
        if (linenoB == lineendB)
            res = QString("%1c%2").arg(linenoA).arg(linenoB);
        else 
            res = QString("%1c%2,%3").arg(linenoA).arg(linenoB).arg(lineendB);
    else if (linenoB == lineendB)
        res = QString("%1,%2c%3").arg(linenoA).arg(lineendA).arg(linenoB);
    else
        res = QString("%1,%2c%3,%4").arg(linenoA).arg(lineendA).arg(linenoB).arg(lineendB);

    return res;
    
}


class DiffItem
{
public:
    DiffView::DiffType type;
    int linenoA, linecountA;
    int linenoB, linecountB;
};


bool DiffDialog::parseCvsDiff(const QString &sandbox, const QString &repository,
                              const QString &filename, const QString &revA, const QString &revB)
{
    QStringList linesA, linesB;
    int linenoA, linenoB;
    enum { Normal, VersionA, VersionB } state;

    setCaption(i18n("CVS Diff: ") + filename);
    revlabel1->setText( revA.isEmpty()?
                        QString(i18n("Repository"))
                        : i18n("Revision ")+revA );
    revlabel2->setText( revB.isEmpty()?
                        QString(i18n("Working dir"))
                        : i18n("Revision ")+revB );
    
    KConfig *config = CervisiaPart::config();
    config->setGroup("General");

    // Ok, this is a hack: When the user wants an external diff
    // front end, it is executed from here. Of course, in that
    // case this dialog wouldn't have to be created in the first
    // place, but this design at least makes the handling trans-
    // parent for the calling routines

    QString extdiff = config->readEntry("ExternalDiff", "");
    if (!extdiff.isEmpty())
        {
            QString cmdline = "cvs update -p ";
            QString extcmdline = extdiff;
            extcmdline += " ";
            
            if (!revA.isEmpty() && !revB.isEmpty())
                {
                    // We're comparing two revisions
                    QString revAFilename = tempFileName(QString("-")+revA);
                    QString revBFilename = tempFileName(QString("-")+revB);
                    cmdline += " -r ";
                    cmdline += revA;
                    cmdline += " ";
                    cmdline += KShellProcess::quote(filename);
                    cmdline += " > ";
                    cmdline += revAFilename;
                    cmdline += " ; cvs update -p ";
                    cmdline += " -r ";
                    cmdline += revB;
                    cmdline += " ";
                    cmdline += KShellProcess::quote(filename);
                    cmdline += " > ";
                    cmdline += revBFilename;
                    
                    extcmdline += revAFilename;
                    extcmdline += " ";
                    extcmdline += revBFilename;
                } else {
                    // We're comparing to a file, and perhaps one revision
                    QString revAFilename = tempFileName(revA);
                    if (!revA.isEmpty())
                        {
                            cmdline += " -r ";
                            cmdline += revA;
                        }
                    cmdline += " ";
                    cmdline += KShellProcess::quote(filename);
                    cmdline += " > ";
                    cmdline += revAFilename;
                    
                    extcmdline += revAFilename;
                    extcmdline += " ";
                    extcmdline += KShellProcess::quote(QFileInfo(filename).absFilePath());
                }
            CvsProgressDialog l("Diff", this);
            if (l.execCommand(sandbox, repository, cmdline, "diff"))
                {
                    KShellProcess proc("/bin/sh");
                    proc << extcmdline;
                    proc.start(KProcess::DontCare);
                }
            return false;
        }

    QString cmdline = cvsClient(repository) + " -f diff ";
    cmdline += config->readEntry("DiffOptions", "");
    cmdline += " -U ";
    cmdline += QString().setNum((int)config->readUnsignedNumEntry("ContextLines", 65535));
    if (!revA.isEmpty())
	{
	    cmdline += " -r ";
	    cmdline += revA;
	}
    if (!revB.isEmpty())
	{
	    cmdline += " -r ";
	    cmdline += revB;
	}
    cmdline += " ";
    cmdline += KShellProcess::quote(filename);

    CvsProgressDialog l("Diff", this);
    l.setCaption(i18n("CVS Diff"));
    if (!l.execCommand(sandbox, repository, cmdline, "diff"))
        return false;

    QString line;
    while ( l.getOneLine(&line) && line.left(3) != "+++")
        ;
    
    state = Normal;
    linenoA = linenoB = 0;
    while ( l.getOneLine(&line) )
        {
            if (line.left(2) == "@@")
                {
                    interpretRegion(line, &linenoA, &linenoB);
                    diff1->addLine(line, DiffView::Separator);
                    diff2->addLine(line, DiffView::Separator);
                    continue;
                }
            if (line.length() < 1)
                continue;
            QChar marker = line[0];
            line.remove(0, 1);
            
            if (marker == '-')
                {
                    state = VersionA;
                    linesA.append(line);
                }
            else if (marker == '+')
                {
                    state = VersionB;
                    linesB.append(line);
                }
            else
                {
                    if (!linesA.isEmpty() || !linesB.isEmpty())
                        {
                            DiffItem *item = new DiffItem;
                            item->linenoA = linenoA+1;
                            item->linenoB = linenoB+1;
                            item->linecountA = linesA.count();
                            item->linecountB = linesB.count();
                            items.append(item);
                            QString str = regionAsString(linenoA+1, linesA.count(),
                                                         linenoB+1, linesB.count());
                            itemscombo->insertItem(str);
                        }
                    QStringList::ConstIterator itA = linesA.begin();
                    QStringList::ConstIterator itB = linesB.begin();
                    for (; itA != linesA.end() || itB != linesB.end(); ++itA, ++itB)
                        {
                            if (itA != linesA.end())
                                {
                                    diff1->addLine(*itA, DiffView::Neutral,
                                                   ++linenoA);
                                    if (itB != linesB.end())
                                        diff2->addLine(*itB, DiffView::Change,
                                                       ++linenoB);
                                    else
                                        diff2->addLine("", DiffView::Delete);
                                }
                            else
                                {
                                    diff1->addLine("", DiffView::Neutral);
                                    diff2->addLine(*itB, DiffView::Insert,
                                                   ++linenoB);
                                }
                        }
                    state = Normal;
                    linesA.clear();
                    linesB.clear();
                    diff1->addLine(line, DiffView::Unchanged, ++linenoA);
                    diff2->addLine(line, DiffView::Unchanged, ++linenoB);
                }
            
	}
    
    // Copy & Paste from above
    if (!linesA.isEmpty() || !linesB.isEmpty())
        {
            DiffItem *item = new DiffItem;
            item->linenoA = linenoA+1;
            item->linenoB = linenoB+1;
            item->linecountA = linesA.count();
            item->linecountB = linesB.count();
            items.append(item);
            QString str = regionAsString(linenoA+1, linesA.count(),
                                         linenoB+1, linesB.count());
            itemscombo->insertItem(str);
        }
    QStringList::ConstIterator itA = linesA.begin();
    QStringList::ConstIterator itB = linesB.begin();
    for (; itA != linesA.end() || itB != linesB.end(); ++itA, ++itB)
        {
            if (itA != linesA.end())
                {
                    diff1->addLine(*itA, DiffView::Neutral,
                                   ++linenoA);
                    if (itB != linesB.end())
                        diff2->addLine(*itB, DiffView::Change,
                                       ++linenoB);
                    else
                        diff2->addLine("", DiffView::Delete);
                }
            else
                {
                    diff1->addLine("", DiffView::Neutral);
                    diff2->addLine(*itB, DiffView::Insert,
                                   ++linenoB);
                }
        }

    updateNofN();
    itemscombo->setMinimumSize(itemscombo->sizeHint());
 
    return true;
}


void DiffDialog::updateNofN()
{
    QString str;
    if (markeditem >= 0)
	str = i18n("%1 of %2").arg(markeditem+1).arg(items.count());
    else
	str = i18n("%1 differences").arg(items.count());
    nofnlabel->setText(str);

    itemscombo->setCurrentItem(markeditem==-2? 0 : markeditem+1);
    
    backbutton->setEnabled(markeditem != -1);
    forwbutton->setEnabled(markeditem != -2 && items.count());
}


void DiffDialog::updateHighlight(int newitem)
{
    if (markeditem >= 0)
	{
	    DiffItem *item = items.at(markeditem);
	    for (int i = item->linenoA; i < item->linenoA+item->linecountA; ++i)
		diff1->setInverted(i, false);
	    for (int i = item->linenoB; i < item->linenoB+item->linecountB; ++i)
		diff2->setInverted(i, false);
	}

    markeditem = newitem;
    
    if (markeditem >= 0)
	{
	    DiffItem *item = items.at(markeditem);
	    for (int i = item->linenoA; i < item->linenoA+item->linecountA; ++i)
		diff1->setInverted(i, true);
	    for (int i = item->linenoB; i < item->linenoB+item->linecountB; ++i)
		diff2->setInverted(i, true);
	    diff1->setCenterLine(item->linenoA);
	    diff2->setCenterLine(item->linenoB);
	}
    diff1->repaint();
    diff2->repaint();
    updateNofN();
}


void DiffDialog::backClicked()
{
    int newitem;
    if (markeditem == -1)
        return; // internal error (button not disabled)
    else if (markeditem == -2) // past end
        newitem = items.count()-1;
    else
        newitem = markeditem-1;
    updateHighlight(newitem);
}


void DiffDialog::forwClicked()
{
    int newitem;
    if (markeditem == -2 || (markeditem == -1 && !items.count()))
        return; // internal error (button not disabled)
    else if (markeditem+1 == (int)items.count()) // past end
        newitem = -2;
    else
        newitem = markeditem+1;
    updateHighlight(newitem);
}

// Local Variables:
// c-basic-offset: 4
// End:
