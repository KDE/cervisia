/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@physik.hu-berlin.de
 *  Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "protocolview.h"

#include <qdir.h>
#include <qpopupmenu.h>
#include <dcopref.h>
#include <kconfig.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>

#include "cervisiapart.h"
#include "cvsjob_stub.h"


ProtocolView::ProtocolView(const QCString& appId, QWidget *parent, const char *name)
    : QTextEdit(parent, name)
    , childproc(0)
    , job(0)
{
    setReadOnly(true);
    setUndoRedoEnabled(false);
    setTextFormat(Qt::RichText);
 
    KConfig *config = CervisiaPart::config();
    config->setGroup("LookAndFeel");
    setFont(config->readFontEntry("ProtocolFont"));
    
    config->setGroup("Colors");
    QColor defaultColor = QColor(255, 100, 100);
    conflictColor=config->readColorEntry("Conflict",&defaultColor);
    defaultColor=QColor(190, 190, 237);
    localChangeColor=config->readColorEntry("LocalChange",&defaultColor);
    defaultColor=QColor(255, 240, 190);
    remoteChangeColor=config->readColorEntry("RemoteChange",&defaultColor);
    
    // create a DCOP stub for the non-concurrent cvs job
    job = new CvsJob_stub(appId, "NonConcurrentJob");

    // establish connections to the signals of the cvs job
    connectDCOPSignal(job->app(), job->obj(), "jobExited(bool, int)",
                      "slotJobExited(bool, int)", true);
    connectDCOPSignal(job->app(), job->obj(), "receivedStdout(QString)",
                      "slotReceivedOutput(QString)", true);
    connectDCOPSignal(job->app(), job->obj(), "receivedStderr(QString)",
                      "slotReceivedOutput(QString)", true);
}


ProtocolView::~ProtocolView()
{
    delete job;
    delete childproc;
}


bool ProtocolView::startJob(const QString &sandbox, const QString &repository,
                            const QString &cmdline)
{
    if (childproc)
        {
            KMessageBox::sorry(topLevelWidget(), 
                               i18n("There is already a job running"),
                               "Cervisia");
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
	*childproc << QString("CVS_RSH=" + KShellProcess::quote(rsh));
    *childproc << cmdline;
    
    connect( childproc, SIGNAL(processExited(KProcess *)),
	     SLOT(childExited()) );
    connect( childproc, SIGNAL(receivedStdout(KProcess *, char *, int)),
	     SLOT(receivedOutput(KProcess *, char *, int)) );
    connect( childproc, SIGNAL(receivedStderr(KProcess *, char *, int)),
	     SLOT(receivedOutput(KProcess *, char *, int)) );

    disconnect( SIGNAL(receivedLine(QString)) );
    disconnect( SIGNAL(jobFinished(bool, int)) );

    return childproc->start(KProcess::NotifyOnExit,
                            KProcess::Communication(KProcess::Stdout|KProcess::Stderr));
}


bool ProtocolView::startJob()
{
    // get command line and add it to output buffer
    QString cmdLine = job->cvsCommand();
    buf += cmdLine;
    buf += '\n';
    processOutput();
    
    // disconnect 3rd party slots from our signals
    disconnect( SIGNAL(receivedLine(QString)) );
    disconnect( SIGNAL(jobFinished(bool, int)) );

    return job->execute();
}


void ProtocolView::cancelJob()
{
    if( childproc )
        childproc->kill();
    else
        job->cancel();
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
    emit jobFinished(childproc->normalExit(), childproc->exitStatus());
    delete childproc;
    childproc = 0;
}


void ProtocolView::slotReceivedOutput(QString buffer)
{
    buf += buffer;
    processOutput();
}


void ProtocolView::slotJobExited(bool normalExit, int exitStatus)
{
    QString msg;
    
    if( normalExit )
    {
        if( exitStatus )
            msg = i18n("[Exited with status %1]\n").arg(exitStatus);
        else
            msg = i18n("[Finished]\n");
    }
    else
        msg = i18n("[Aborted]\n");
    
    buf += '\n';
    buf += msg;
    processOutput();

    emit jobFinished(normalExit, exitStatus);
}


void ProtocolView::processOutput()
{
    int pos;
    while ( (pos = buf.find('\n')) != -1)
	{
	    QString line = buf.left(pos);
	    if (!line.isEmpty())
                {
		    appendLine(line);
                    emit receivedLine(line);
                }
	    buf = buf.right(buf.length()-pos-1);
	}

    scrollToBottom();
}


void ProtocolView::appendLine(const QString &line)
{
    QColor color;
    // Colors are the same as in UpdateViewItem::paintCell()
    if (line.startsWith("C "))
        color = conflictColor;
    else if (line.startsWith("M ")
             || line.startsWith("A ") || line.startsWith("R "))
        color = localChangeColor;
    else if (line.startsWith("P ") || line.startsWith("U "))
        color = remoteChangeColor;

    append(color.isValid()
           ? QString("<font color=\"%1\">%2</font>").arg(color.name()).arg(line)
           : QString("%1").arg(line));
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
        QTextEdit::mousePressEvent(e);
}


void ProtocolView::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == KGlobalSettings::contextMenuKey())
        execContextMenu(mapToGlobal(QPoint(10, 10)));
    else if (e->key() == Key_Tab)
        QTextEdit::focusNextPrevChild(true);
    else
        QTextEdit::keyPressEvent(e);
}

#include "protocolview.moc"


// Local Variables:
// c-basic-offset: 4
// End:

    
