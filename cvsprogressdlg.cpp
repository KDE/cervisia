/* 
 *  Copyright (C) 1999-2002 Bernd Gehrmann
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


#include "cvsprogressdlg.h"

#include <qpushbutton.h>
#include <qdir.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qlistbox.h>
#include <kapplication.h>
#include <kbuttonbox.h>
#include <kprocess.h>
#include <klocale.h>
#include <kconfig.h>
#include <kanimwidget.h>

#include "misc.h"
#include "cervisiapart.h"


CvsProgressDialog::CvsProgressDialog(const QString &text, QWidget *parent)
    : QSemiModal(parent, "", true), childproc(0)
{
    QBoxLayout *layout = new QVBoxLayout(this, 10);

    QBoxLayout *hbox = new QHBoxLayout();
    layout->addLayout(hbox, 0);
    
    QLabel *textlabel = new QLabel(text, this);
    textlabel->setMinimumWidth(textlabel->sizeHint().width());
    textlabel->setFixedHeight(textlabel->sizeHint().height());
    hbox->addWidget(textlabel);

    gear = new KAnimWidget(QString("kde"), 32, this);
    gear->setFixedSize(32, 32);
    hbox->addStretch();
    hbox->addWidget(gear);

    resultbox = new QListBox(this);
    resultbox->setEnabled(false);
    QFontMetrics rb_fm(resultbox->fontMetrics());
    resultbox->setMinimumSize(rb_fm.width("0")*70,
			      rb_fm.lineSpacing()*8);
    layout->addWidget(resultbox, 5);
    
    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    frame->setMinimumHeight(frame->sizeHint().height());
    layout->addWidget(frame, 0);

    KButtonBox *buttonbox = new KButtonBox(this);
    layout->addWidget(buttonbox, 0);
    buttonbox->addStretch();
    cancelbutton = buttonbox->addButton(i18n("Cancel"));
    connect( cancelbutton, SIGNAL(clicked()), SLOT(cancelClicked()) );
    buttonbox->addStretch();
    buttonbox->layout();

    layout->activate();
    resize(sizeHint());

    shown = false;
    cancelled = false;
}


CvsProgressDialog::~CvsProgressDialog()
{
    delete childproc;
}


bool CvsProgressDialog::execCommand(const QString &sandbox, const QString &repository,
                                    const QString &cmdline, const QString &errindicator)
{
    KConfig *config = CervisiaPart::config();
    config->setGroup("General");
    int timeout = (int) config->readUnsignedNumEntry("Timeout", 4000);
    
    indic1 = QString("cvs ") + errindicator + ":";
    indic2 = QString("cvs [") + errindicator + " aborted]:";
    resultbox->insertItem(cmdline);

    config->setGroup("Repository-" + repository);
    QString rsh = config->readEntry("rsh");

    childproc = new KShellProcess("/bin/sh");
    if (!sandbox.isEmpty())
        QDir::setCurrent(sandbox);
    if (!rsh.isEmpty())
	*childproc << QString("CVS_RSH='" + rsh + "'");
    *childproc << cmdline;

    connect( childproc, SIGNAL(processExited(KProcess *)),
	     SLOT(childExited()) );
    // We want to be notified when the child provides error output ...
    connect( childproc, SIGNAL(receivedStdout(KProcess *, char *, int)),
             SLOT(receivedOutputNongui(KProcess *, char *, int)) );
    connect( childproc, SIGNAL(receivedStderr(KProcess *, char *, int)),
             SLOT(receivedOutputNongui(KProcess *, char *, int)) );
    // ... or after 4 seconds (i. e. the timeout set by the user)
    timer = new QTimer(this);
    connect( timer, SIGNAL(timeout()),
             this, SLOT(timeoutOccured()) );
    
    timer->start(timeout, true);
    
    if (!childproc->start(KProcess::NotifyOnExit,
                          KProcess::Communication(KProcess::Stdout|KProcess::Stderr)))
        return false;

    QApplication::setOverrideCursor(waitCursor);
    kapp->enter_loop();
    if (QApplication::overrideCursor())
        QApplication::restoreOverrideCursor();

    return !cancelled;
}


void CvsProgressDialog::stopNonguiPart()
{
    timer->stop();
    disconnect( childproc, SIGNAL(receivedStdout(KProcess *, char *, int)),
                this, SLOT(receivedOutputNongui(KProcess *, char *, int)) );
    disconnect( childproc, SIGNAL(receivedStderr(KProcess *, char *, int)),
                this, SLOT(receivedOutputNongui(KProcess *, char *, int)) );

    kapp->exit_loop();
}


void CvsProgressDialog::startGuiPart()
{
    shown = true;
    connect( childproc, SIGNAL(receivedStdout(KProcess *, char *, int)),
	     SLOT(receivedOutput(KProcess *, char *, int)) );
    connect( childproc, SIGNAL(receivedStderr(KProcess *, char *, int)),
	     SLOT(receivedOutput(KProcess *, char *, int)) );
    show();
    gear->start();
    QApplication::restoreOverrideCursor();
    kapp->enter_loop();
}


void CvsProgressDialog::timeoutOccured()
{
    stopNonguiPart();
    startGuiPart();
}



bool CvsProgressDialog::getOneLine(QString *str)
{
    if (output.isEmpty())
        return false;

    *str = output.first();
    output.remove(output.begin());

    return true;
}


bool CvsProgressDialog::processOutput()
{
    bool err = false;
    int pos;
    while ( (pos = buf.find('\n')) != -1)
	{
	    QString item = buf.left(pos);
            if (item.left(indic1.length()) == indic1 ||
                item.left(indic2.length()) == indic2 ||
                item.left(21) == "cvs [server aborted]:")
                {
                    err = true;
                    resultbox->insertItem(item);
                }
            else if (item.left(11) == "cvs server:")
                resultbox->insertItem(item);
            else
                output.append(item);
	    buf = buf.right(buf.length()-pos-1);
        }
    
    return err;
}


void CvsProgressDialog::receivedOutputNongui(KProcess *, char *buffer, int buflen)
{
    buf += QString::fromLocal8Bit(buffer, buflen);
    if (processOutput())
        {
            stopNonguiPart();
            startGuiPart();
        }
}


void CvsProgressDialog::receivedOutput(KProcess *, char *buffer, int buflen)
{
    buf += QString::fromLocal8Bit(buffer, buflen);
    (void) processOutput();
}
 

void CvsProgressDialog::childExited()
{
    if (!shown)
        stopNonguiPart();

    gear->stop();
    if (!buf.isEmpty())
      {
        buf += '\n';
        (void) processOutput();
      }

    // Close the dialog automatically if there are no
    // error messages or the process has been aborted
    // 'by hand' (e.g.  by clicking the cancel button)
    if (resultbox->count() < 2 || !childproc->normalExit())
        kapp->exit_loop();
}


void CvsProgressDialog::cancelClicked()
{
    cancelled = true;

    if (childproc->isRunning())
        childproc->kill();
    else
        kapp->exit_loop();
}


void CvsProgressDialog::closeEvent(QCloseEvent *e)
{
    cancelClicked();
    QSemiModal::closeEvent(e);
}

#include "cvsprogressdlg.moc"

// Local Variables:
// c-basic-offset: 4
// End:
