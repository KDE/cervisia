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


#include "resolvedlg.h"

#include <qfile.h>
#include <qkeycode.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qtextcodec.h>
#include <qtextstream.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "diffview.h"
#include "misc.h"

#include <kdeversion.h>
#if KDE_VERSION < KDE_MAKE_VERSION(3,1,90)
#include "configutils.h"
#endif


ResolveDialog::ResolveDialog(KConfig& cfg, QWidget *parent, const char *name)
    : KDialogBase(parent, name, false, QString::null,
                  Close | Help | User1 | User2, Close, true,
                  KStdGuiItem::saveAs(), KStdGuiItem::save())
    , markeditem(-1)
    , partConfig(cfg)
{
    items.setAutoDelete(true);

    QFrame* mainWidget = makeMainWidget();

    QBoxLayout *layout = new QVBoxLayout(mainWidget, 0, spacingHint());

    QSplitter *vertSplitter = new QSplitter(QSplitter::Vertical, mainWidget);

    QSplitter *splitter = new QSplitter(QSplitter::Horizontal, vertSplitter);

    QWidget *versionALayoutWidget = new QWidget(splitter);
    QBoxLayout *versionAlayout = new QVBoxLayout(versionALayoutWidget, 5);

    QLabel *revlabel1 = new QLabel(i18n("Your version (A):"), versionALayoutWidget);
    versionAlayout->addWidget(revlabel1);
    diff1 = new DiffView(true, false, versionALayoutWidget);
    versionAlayout->addWidget(diff1, 10);

    QWidget* versionBLayoutWidget = new QWidget(splitter);
    QBoxLayout *versionBlayout = new QVBoxLayout(versionBLayoutWidget, 5);

    QLabel *revlabel2 = new QLabel(i18n("Other version (B):"), versionBLayoutWidget);
    versionBlayout->addWidget(revlabel2);       
    diff2 = new DiffView(true, false, versionBLayoutWidget);
    versionBlayout->addWidget(diff2, 10);       

    diff1->setPartner(diff2);
    diff2->setPartner(diff1);

    QWidget* mergeLayoutWidget = new QWidget(vertSplitter);
    QBoxLayout *mergeLayout = new QVBoxLayout(mergeLayoutWidget, 5);

    QLabel *mergelabel = new QLabel(i18n("Merged version:"), mergeLayoutWidget);
    mergeLayout->addWidget(mergelabel);

    merge = new DiffView(false, false, mergeLayoutWidget);
    mergeLayout->addWidget(merge, 10);

    layout->addWidget(vertSplitter);

    abutton = new QPushButton("&A", mainWidget);
    connect( abutton, SIGNAL(clicked()), SLOT(aClicked()) );
    
    bbutton = new QPushButton("&B", mainWidget);
    connect( bbutton, SIGNAL(clicked()), SLOT(bClicked()) );

    abbutton = new QPushButton("A+B", mainWidget);
    connect( abbutton, SIGNAL(clicked()), SLOT(abClicked()) );

    babutton = new QPushButton("B+A", mainWidget);
    connect( babutton, SIGNAL(clicked()), SLOT(baClicked()) );

    editbutton = new QPushButton("&Edit", mainWidget);
    connect( editbutton, SIGNAL(clicked()), SLOT(editClicked()) );

    nofnlabel = new QLabel(mainWidget);
    nofnlabel->setAlignment(AlignCenter);
    
    backbutton = new QPushButton("&<<", mainWidget);
    connect( backbutton, SIGNAL(clicked()), SLOT(backClicked()) );
    
    forwbutton = new QPushButton("&>>", mainWidget);
    connect( forwbutton, SIGNAL(clicked()), SLOT(forwClicked()) );

    QBoxLayout *buttonlayout = new QHBoxLayout(layout);
    buttonlayout->addWidget(abutton, 1);
    buttonlayout->addWidget(bbutton, 1);
    buttonlayout->addWidget(abbutton, 1);
    buttonlayout->addWidget(babutton, 1);
    buttonlayout->addWidget(editbutton, 1);
    buttonlayout->addStretch(1);
    buttonlayout->addWidget(nofnlabel, 2);
    buttonlayout->addStretch(1);
    buttonlayout->addWidget(backbutton, 1);
    buttonlayout->addWidget(forwbutton, 1);

    connect( this, SIGNAL(user2Clicked()), SLOT(saveClicked()) );
    connect( this, SIGNAL(user1Clicked()), SLOT(saveAsClicked()) );

    QFontMetrics const fm(fontMetrics());
    setMinimumSize(fm.width('0') * 120,
                   fm.lineSpacing() * 40);

    setHelp("resolvingconflicts");

    setWFlags(Qt::WDestructiveClose | getWFlags());

#if KDE_IS_VERSION(3,1,90)
    QSize size = configDialogSize(partConfig, "ResolveDialog");
#else
    QSize size = Cervisia::configDialogSize(this, partConfig, "ResolveDialog");
#endif
    resize(size);
}


ResolveDialog::~ResolveDialog()
{
#if KDE_IS_VERSION(3,1,90)
    saveDialogSize(partConfig, "ResolveDialog");
#else
    Cervisia::saveDialogSize(this, partConfig, "ResolveDialog");
#endif
}


// One resolve item has a line number range of linenoA:linenoA+linecountA-1
// in A and linenoB:linenoB+linecountB-1 in B. If the user has chosen version A
// for the merged file (indicated by chosenA==true), then the line number
// range in the merged file is offsetM:offsetM+linecountA-1 (accordingly for
// the other case).
class ResolveItem
{
public:
    int linenoA, linecountA;
    int linenoB, linecountB;
    int linecountTotal;
    int offsetM;
    ResolveDialog::ChooseType chosen;
};


bool ResolveDialog::parseFile(const QString &name)
{
    int lineno1, lineno2;
    int advanced1, advanced2;
    enum { Normal, VersionA, VersionB } state;
    
    setCaption(i18n("CVS Resolve: %1").arg(name));

    fname = name;

    QFile f(name);
    if (!f.open(IO_ReadOnly))
        return false;
    QTextStream stream(&f);
    QTextCodec *fcodec = detectCodec(name);
    stream.setCodec(fcodec);

    state = Normal;
    lineno1 = lineno2 = 0;
    advanced1 = advanced2 = 0;
    while (!stream.atEnd())
	{
	    QString line = stream.readLine();
	    if (line.left(7) == "<<<<<<<")
		{
		    state = VersionA;
		    advanced1 = 0;
		}
	    else if (line.left(7) == "=======" && state == VersionA)
		{
		    state = VersionB;
		    advanced2 = 0;
		}
	    else if (line.left(7) == ">>>>>>>")
		{
		    ResolveItem *item = new ResolveItem;
		    item->linenoA = lineno1-advanced1+1;
		    item->linecountA = advanced1;
		    item->linenoB = lineno2-advanced2+1;
		    item->linecountB = advanced2;
		    item->offsetM = item->linenoA-1;
		    item->chosen = ChA;
                    item->linecountTotal = item->linecountA;
		    items.append(item);
		    for (; advanced1 < advanced2; advanced1++)
			diff1->addLine("", DiffView::Neutral);
		    for (; advanced2 < advanced1; advanced2++)
			diff2->addLine("", DiffView::Neutral);
		    state = Normal;
		}
	    else if (state == VersionA)
		{
		    lineno1++;
		    advanced1++;
		    diff1->addLine(line, DiffView::Change, lineno1);
		    merge->addLine(line, DiffView::Change, lineno1);
		}
	    else if (state == VersionB)
		{
		    lineno2++;
		    advanced2++;
		    diff2->addLine(line, DiffView::Change, lineno2);
		}
	    else // state == Normal
		{
		    lineno1++;
		    lineno2++;
		    diff1->addLine(line, DiffView::Unchanged, lineno1);
		    merge->addLine(line, DiffView::Unchanged, lineno1);
		    diff2->addLine(line, DiffView::Unchanged, lineno2);
		}
	}
    f.close();
    updateNofN();
    
    return true; // succesful
}


void ResolveDialog::saveFile(const QString &name)
{
    QFile f(name);
    if (!f.open(IO_WriteOnly))
	{
	    KMessageBox::sorry(this,
			       i18n("Could not open file for writing."),
			       "Cervisia");
	    return;
	}
    QTextStream stream(&f);
    QTextCodec *fcodec = detectCodec(name);
    stream.setCodec(fcodec);
        
    int count = merge->count();
    for (int i = 0; i < count; ++i)
        stream << merge->stringAtOffset(i) << endl;

    f.close();
}


void ResolveDialog::updateNofN()
{
    QString str;
    if (markeditem >= 0)
	str = i18n("%1 of %2").arg(markeditem+1).arg(items.count());
    else
	str = i18n("%1 conflicts").arg(items.count());
    nofnlabel->setText(str);

    backbutton->setEnabled(markeditem != -1);
    forwbutton->setEnabled(markeditem != -2 && items.count());

    bool marked = markeditem >= 0;
    abutton->setEnabled(marked);
    bbutton->setEnabled(marked);
    abbutton->setEnabled(marked);
    babutton->setEnabled(marked);
    editbutton->setEnabled(marked);
}


void ResolveDialog::updateHighlight(int newitem)
{
    if (markeditem >= 0)
	{
	    ResolveItem *item = items.at(markeditem);
	    for (int i = item->linenoA; i < item->linenoA+item->linecountA; ++i)
		diff1->setInverted(i, false);
	    for (int i = item->linenoB; i < item->linenoB+item->linecountB; ++i)
		diff2->setInverted(i, false);
	}

    markeditem = newitem;
    
    if (markeditem >= 0)
	{
	    ResolveItem *item = items.at(markeditem);
	    for (int i = item->linenoA; i < item->linenoA+item->linecountA; ++i)
		diff1->setInverted(i, true);
	    for (int i = item->linenoB; i < item->linenoB+item->linecountB; ++i)
		diff2->setInverted(i, true);
	    diff1->setCenterLine(item->linenoA);
	    diff2->setCenterLine(item->linenoB);
            merge->setCenterOffset(item->offsetM);
	}
    diff1->repaint();
    diff2->repaint();
    merge->repaint();
    updateNofN();
}


void ResolveDialog::backClicked()
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


void ResolveDialog::forwClicked()
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


void ResolveDialog::choose(ChooseType ch)
{
    DiffView *first=0, *second=0;
    int firstno=0, secondno=0;
    int firstcount=0, secondcount=0;
  
    if (markeditem < 0)
	return;

    ResolveItem *item = items.at(markeditem);
    if (item->chosen == ch)
	return;

    switch (ch)
        {
        case ChA:
            first = diff1; 
            firstno = item->linenoA;
            firstcount = item->linecountA;
            break;
        case ChB:
            first = diff2; 
            firstno = item->linenoB;
            firstcount = item->linecountB;
            break;
        case ChAB:
            first = diff1; 
            firstno = item->linenoA;
            firstcount = item->linecountA;
            second = diff2; 
            secondno = item->linenoB;
            secondcount = item->linecountB;
            break;
        case ChBA:
            first = diff2; 
            firstno  = item->linenoB;
            firstcount = item->linecountB;
            second = diff1; 
            secondno = item->linenoA;
            secondcount = item->linecountA;
            break;
        default:
            kdDebug() << "Internal error at switch" << endl;
        }

    int total = firstcount + secondcount;
    int difference = total - item->linecountTotal;
    
    // Remove old variant
    for (int i = 0; i < item->linecountTotal; ++i)
        merge->removeAtOffset(item->offsetM);

    // Insert new
    for (int i = 0; i < firstcount; ++i)
        merge->insertAtOffset(first->stringAtLine(firstno+i), DiffView::Change, item->offsetM+i);
    
    if (second)
        for (int i = 0; i < secondcount; ++i)
            merge->insertAtOffset(second->stringAtLine(secondno+i), DiffView::Change, item->offsetM+firstcount+i);
    
    item->chosen = ch;
    item->linecountTotal = total;
    
    // Adjust other items
    while ( (item = items.next()) != 0 )
        item->offsetM += difference;
    
    merge->repaint();
    
}


void ResolveDialog::aClicked()
{
    choose(ChA);
}


void ResolveDialog::bClicked()
{
    choose(ChB);
}


void ResolveDialog::abClicked()
{
    choose(ChAB);
}


void ResolveDialog::baClicked()
{
    choose(ChBA);
}


void ResolveDialog::editClicked()
{
    if (markeditem < 0)
        return;
    ResolveItem *item = items.at(markeditem);

    QStringList oldContent;
    for (int i = 0; i < item->linecountTotal; ++i)
        oldContent << merge->stringAtOffset(item->offsetM+i);

    ResolveEditorDialog *dlg = new ResolveEditorDialog(partConfig, this, "edit");
    dlg->setContent(oldContent);
    
    if (dlg->exec())
        {
            QStringList newContent = dlg->content();
            int total = newContent.count();
            int difference = total - item->linecountTotal;
            
            // Remove old variant
            for (int i = 0; i < item->linecountTotal; ++i)
                merge->removeAtOffset(item->offsetM);
            
            // Insert new
            for (int i = 0; i < total; ++i)
                merge->insertAtOffset(newContent[i], DiffView::Change, item->offsetM+i);
            
            item->chosen = ChEdit;
            item->linecountTotal = total;
            
            // Adjust other items
            while ( (item = items.next()) != 0 )
                item->offsetM += difference;
            
            merge->repaint();
        }
    
    delete dlg;
}


void ResolveDialog::saveClicked()
{
    saveFile(fname);
}


void ResolveDialog::saveAsClicked()
{
    QString filename =
	KFileDialog::getSaveFileName(0, 0, this, 0);

    if (!filename.isEmpty())
        saveFile(filename);
}


void ResolveDialog::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
	{
	case Key_A:    aClicked();    break;
	case Key_B:    bClicked();    break;
	case Key_Left: backClicked(); break;
	case Key_Right:forwClicked(); break;
        case Key_Up:   diff1->up();   break;
        case Key_Down: diff1->down(); break;
        default:
            KDialogBase::keyPressEvent(e);
	}
}


ResolveEditorDialog::ResolveEditorDialog(KConfig& cfg, QWidget *parent, const char *name)
    : KDialogBase(parent, name, true, QString::null,
                  Ok | Cancel, Ok, true)
    , partConfig(cfg)
{
    edit = new QMultiLineEdit(this);
    edit->setFocus();

    setMainWidget(edit);

    QFontMetrics const fm(fontMetrics());
    setMinimumSize(fm.width('0') * 120,
                   fm.lineSpacing() * 40);

#if KDE_IS_VERSION(3,1,90)
    QSize size = configDialogSize(partConfig, "ResolveEditDialog");
#else
    QSize size = Cervisia::configDialogSize(this, partConfig, "ResolveEditDialog");
#endif
    resize(size);
}


ResolveEditorDialog::~ResolveEditorDialog()
{
#if KDE_IS_VERSION(3,1,90)
    saveDialogSize(partConfig, "ResolveEditDialog");
#else
    Cervisia::saveDialogSize(this, partConfig, "ResolveEditDialog");
#endif
}


void ResolveEditorDialog::setContent(const QStringList &l)
{
    QStringList::ConstIterator it;
    for (it = l.begin(); it != l.end(); ++it)
        edit->insertLine((*it).left((*it).length()-1));
}


QStringList ResolveEditorDialog::content() const
{
    QStringList l;
    for (int i = 0; i < edit->numLines(); ++i)
        l << (edit->textLine(i) + '\n');

    return l;
}

#include "resolvedlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
