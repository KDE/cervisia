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


#include "protocolview.h"

#include <qdir.h>
#include <qpopupmenu.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>

#include "repositories.h"
#include "misc.h"
#include "cervisiapart.h"


ProtocolView::ProtocolView(QWidget *parent, const char *name)
    : QTextEdit(parent, name), childproc(0)
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
    disconnect( SIGNAL(jobFinished(bool)) );

    return childproc->start(KProcess::NotifyOnExit,
                            KProcess::Communication(KProcess::Stdout|KProcess::Stderr));
}


QPopupMenu* ProtocolView::createPopupMenu(const QPoint &pos)
{
    QPopupMenu* menu = QTextEdit::createPopupMenu(pos);

    int id = menu->insertItem(i18n("Clear"), this, SLOT( clear() ), 0, -1, 0);

    if( text().isEmpty() )
        menu->setItemEnabled(id, false);

    return menu;
}


void ProtocolView::cancelJob()
{
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

    append(color.isValid()?
        QString("<FONT COLOR=\"#%1\">%2</FONT><BR>")
           .arg(colorAsString(color)).arg(line) :
        QString("%1").arg(line));
}


#include "protocolview.moc"


// Local Variables:
// c-basic-offset: 4
// End:

    
