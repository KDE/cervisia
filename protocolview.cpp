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


#include <qdir.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#if QT_VERSION < 300
#include <qtableview.h>
#endif

#include <kapp.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kprocess.h>

#include "repositories.h"
#include "misc.h"
#include "cervisiapart.h"

#include "protocolview.h"
#include "protocolview.moc"


ProtocolView::ProtocolView(QWidget *parent, const char *name)
    : QMultiLineEdit(parent, name), childproc(0)
{
    setReadOnly(true);
#if QT_VERSION < 300
    setUndoEnabled(false);
#endif

    KConfig *config = CervisiaPart::config();
    config->setGroup("LookAndFeel");
    setFont(config->readFontEntry("ProtocolFont"));
}


ProtocolView::~ProtocolView()
{
    delete childproc;
}


bool ProtocolView::startJob(const QString &sandbox, const QString &repository,
                            const QString &cmdline)
{
    if (childproc)
        {
            QMessageBox::information(topLevelWidget(), "Cervisia", "There is already a job running");
            return false;
        }

    buf += cmdline;
    buf += '\n';
    processOutput();

    KConfig *config = CervisiaPart::config();
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
    connect( childproc, SIGNAL(receivedStdout(KProcess *, char *, int)),
	     SLOT(receivedOutput(KProcess *, char *, int)) );
    connect( childproc, SIGNAL(receivedStderr(KProcess *, char *, int)),
	     SLOT(receivedOutput(KProcess *, char *, int)) );

    disconnect( SIGNAL(receivedLine(QString)) );
    disconnect( SIGNAL(jobFinished(bool)) );

    return childproc->start(KProcess::NotifyOnExit,
                            KProcess::Communication(KProcess::Stdout|KProcess::Stderr));
}


//#include <iostream.h>
void ProtocolView::cancelJob()
{
    //    cout << "Canceled" << endl;
    childproc->kill();
}


void ProtocolView::receivedOutput(KProcess *, char *buffer, int buflen)
{
    buf += QCString(buffer, buflen+1);
    processOutput();
}


void ProtocolView::childExited()
{
    QString s;
    
    if (childproc->normalExit())
        {
            if (childproc->exitStatus())
                s = i18n("[Exited with status %1]\n").arg(childproc->exitStatus());
            else
                s = i18n("[Finished]\n");
        }
    else
        s = i18n("[Aborted]\n");
    
    buf += '\n';
    buf += s;
    processOutput();
    emit jobFinished(childproc->normalExit() && !childproc->exitStatus());
    delete childproc;
    childproc = 0;
}


void ProtocolView::processOutput()
{
    int pos;
    while ( (pos = buf.find('\n')) != -1)
	{
	    QString line = buf.left(pos);
	    if (!line.isEmpty())
                {
		    insertLine(line);
                    emit receivedLine(line);
                }
	    buf = buf.right(buf.length()-pos-1);
	}
    setCursorPosition(numLines(), 0, false);
}


void ProtocolView::execContextMenu(const QPoint &p)
{
    QPopupMenu *popup = new QPopupMenu(this);
    int clearId = popup->insertItem(i18n("Clear"));
    int selallId = popup->insertItem(i18n("Select All"));
    
    int res = popup->exec(p);
    if (res == clearId)
        clear();
    else if (res == selallId)
        selectAll();
    
    delete popup;
}


void ProtocolView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == RightButton)
        execContextMenu(e->globalPos());
    else
        QMultiLineEdit::mousePressEvent(e);
}


void ProtocolView::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == KGlobalSettings::contextMenuKey())
        execContextMenu(mapToGlobal(QPoint(10, 10)));
    else if (e->key() == Key_Tab) {
#if QT_VERSION < 300
        QTableView::focusNextPrevChild(true);
#endif
    }
    else
        QMultiLineEdit::keyPressEvent(e);
}

// Local Variables:
// c-basic-offset: 4
// End:

    
