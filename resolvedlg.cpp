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
#include <qlayout.h>
#include <qkeycode.h>
#include <qtextstream.h>
#include <qfile.h>
#include <kbuttonbox.h>
#include <kapp.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include "misc.h"

#include "resolvedlg.h"
#include "resolvedlg.moc"


ResolveDialog::Options *ResolveDialog::options = 0;


ResolveDialog::ResolveDialog( QWidget *parent, const char *name)
    : QDialog(parent, name, false,
              WStyle_Customize|WStyle_NormalBorder|WStyle_Title|WStyle_MinMax)
{
    items.setAutoDelete(true);
    markeditem = -1;
    QFontMetrics fm(fontMetrics());

    QBoxLayout *layout = new QVBoxLayout(this, 10);

    QGridLayout *pairlayout = new QGridLayout(2, 2, 10);
    layout->addLayout(pairlayout, 10);

    QLabel *revlabel1 = new QLabel(i18n("Your Version (A):"), this);
    revlabel1->setFixedHeight(revlabel1->sizeHint().height());
    pairlayout->addWidget(revlabel1, 0, 0);
			      
    QLabel *revlabel2 = new QLabel(i18n("Other Version (B):"), this);
    revlabel2->setFixedHeight(revlabel2->sizeHint().height());
    pairlayout->addWidget(revlabel2, 0, 1);

    diff1 = new DiffView(true, false, this);
    pairlayout->addWidget(diff1, 1, 0);

    diff2 = new DiffView(true, false, this);
    pairlayout->addWidget(diff2, 1, 1);

    diff1->setPartner(diff2);
    diff2->setPartner(diff1);
    
    QLabel *mergelabel = new QLabel(i18n("Merged Version:"), this);
    mergelabel->setFixedHeight(mergelabel->sizeHint().height());
    layout->addSpacing(5);
    layout->addWidget(mergelabel);
    
    merge = new DiffView(false, false, this);
    layout->addWidget(merge, 10);

    abutton = new QPushButton("&A", this);
    connect( abutton, SIGNAL(clicked()), SLOT(aClicked()) );
    
    bbutton = new QPushButton("&B", this);
    connect( bbutton, SIGNAL(clicked()), SLOT(bClicked()) );

    abbutton = new QPushButton("A+B", this);
    connect( abbutton, SIGNAL(clicked()), SLOT(abClicked()) );

    babutton = new QPushButton("B+A", this);
    connect( babutton, SIGNAL(clicked()), SLOT(baClicked()) );

    editbutton = new QPushButton("&Edit", this);
    connect( editbutton, SIGNAL(clicked()), SLOT(editClicked()) );

    nofnlabel = new QLabel(this);
    nofnlabel->setAlignment(AlignCenter);
    
    backbutton = new QPushButton("&<<", this);
    connect( backbutton, SIGNAL(clicked()), SLOT(backClicked()) );
    
    forwbutton = new QPushButton("&>>", this);
    connect( forwbutton, SIGNAL(clicked()), SLOT(forwClicked()) );

    QBoxLayout *buttonlayout = new QHBoxLayout();
    layout->addLayout(buttonlayout);
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
    
    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    frame->setMinimumHeight(frame->sizeHint().height());
    layout->addWidget(frame, 0);

    KButtonBox *buttonbox = new KButtonBox(this);
    connect( buttonbox->addButton(i18n("&Save")), SIGNAL(clicked()),
	     SLOT(slotSave()) );
    connect( buttonbox->addButton(i18n("S&ave As...")), SIGNAL(clicked()),
	     SLOT(slotSaveAs()) );
    buttonbox->addStretch();
    connect( buttonbox->addButton(i18n("&Close")), SIGNAL(clicked()),
	     SLOT(accept()) );
    buttonbox->layout();
    layout->addWidget(buttonbox, 0);

    setMinimumSize(fm.width("0123456789")*12,
		   fm.lineSpacing()*40);
    layout->activate();

    if (options)
        resize(options->size);
}


void ResolveDialog::done(int res)
{
    if (!options)
        options = new Options;
    options->size = size();
    
    QDialog::done(res);
    delete this;
}
	    

void ResolveDialog::loadOptions(KConfig *config)
{
    if (!config->readEntry("Customized"))
        return;

    options = new Options;
    options->size = config->readSizeEntry("Size");
}


void ResolveDialog::saveOptions(KConfig *config)
{
    if (!options)
        return;

    config->writeEntry("Customized", true);
    config->writeEntry("Size", options->size);
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
    char buf[512];
    enum { Normal, VersionA, VersionB } state;
    
    setCaption(i18n("CVS Resolve: ") + name);

    fname = name;
    
    FILE *f = fopen(name.latin1(), "r");
    if (!f)
	return false;

    state = Normal;
    lineno1 = lineno2 = 0;
    advanced1 = advanced2 = 0;
    while (fgets(buf, sizeof buf, f))
	{
	    QString line = buf;
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
    fclose(f);
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
    QTextStream t(&f);
        
    int count = merge->count();
    for (int i = 0; i < count; ++i)
        t << merge->stringAtOffset(i);

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

    ResolveEditorDialog *dlg = new ResolveEditorDialog(this, "edit");
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


void ResolveDialog::slotSave()
{
    saveFile(fname);
}


void ResolveDialog::slotSaveAs()
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
	}
}

ResolveEditorDialog::Options *ResolveEditorDialog::options = 0;


ResolveEditorDialog::ResolveEditorDialog(QWidget *parent, const char *name)
    : QDialog(parent, name, true,
              WStyle_Customize|WStyle_NormalBorder|WStyle_Title|WStyle_MinMax)
{
    QFontMetrics fm(fontMetrics());

    QBoxLayout *layout = new QVBoxLayout(this, 10);

    setMinimumSize(fm.width("0123456789")*120,
		   fm.lineSpacing()*120);

    edit = new QMultiLineEdit(this);
    edit->setFocus();
    layout->addWidget(edit, 10);
  
    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    frame->setMinimumHeight(frame->sizeHint().height());
    layout->addWidget(frame, 0);
    
    KButtonBox *buttonbox = new KButtonBox(this);
    buttonbox->addStretch();
    QPushButton *ok = buttonbox->addButton(i18n("&OK"));
    QPushButton *cancel = buttonbox->addButton(i18n("Cancel"));
    ok->setDefault(true);
    connect( ok, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancel, SIGNAL(clicked()), this, SLOT(reject()) );
    buttonbox->layout();
    layout->addWidget(buttonbox, 0);
    layout->activate();

    if (options)
        resize(options->size);
}


void ResolveEditorDialog::done(int r)
{
    if (!options)
        options = new Options;
    options->size = size();
    QDialog::done(r);
}


void ResolveEditorDialog::loadOptions(KConfig *config)
{
    if (!config->readEntry("Customized"))
        return;

    options = new Options;
    options->size = config->readSizeEntry("Size");
}


void ResolveEditorDialog::saveOptions(KConfig *config)
{
    if (!options)
        return;

    config->writeEntry("Customized", true);
    config->writeEntry("Size", options->size);
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


// Local Variables:
// c-basic-offset: 4
// End:
