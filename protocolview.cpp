/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@physik.hu-berlin.de
 *  Copyright (c) 2003-2004 Christian Loose <christian.loose@kdemail.net>
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


#include "protocolview.h"

#include <qdir.h>
#include <q3popupmenu.h>
#include <qtextdocument.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "cervisiapart.h"
#include "cvsjobinterface.h"


ProtocolView::ProtocolView(/*const DCOPCString& appId,*/ QWidget *parent, const char *name)
    : Q3TextEdit(parent, name)
    , job(0)
    , m_isUpdateJob(false)
{
    setReadOnly(true);
    setUndoRedoEnabled(false);
    setTabChangesFocus(true);
    setTextFormat(Qt::LogText);

    KConfig *config = CervisiaPart::config();
    config->setGroup("LookAndFeel");
    setFont(config->readEntry("ProtocolFont",QFont()));

    config->setGroup("Colors");
    conflictColor=config->readEntry("Conflict", QColor(255, 130, 130));
    localChangeColor=config->readEntry("LocalChange", QColor(130, 130, 255));
    remoteChangeColor=config->readEntry("RemoteChange", QColor(70, 210, 70));
#warning "kde4: port it"
#if 0
    // create a DCOP stub for the non-concurrent cvs job
    job = new OrgKdeCervisiaCvsserviceCvsjobInterface(appId, "NonConcurrentJob");
    // establish connections to the signals of the cvs job
    connectDCOPSignal(job->app(), job->obj(), "jobExited(bool, int)",
                      "slotJobExited(bool, int)", true);
    connectDCOPSignal(job->app(), job->obj(), "receivedStdout(QString)",
                      "slotReceivedOutput(QString)", true);
    connectDCOPSignal(job->app(), job->obj(), "receivedStderr(QString)",
                      "slotReceivedOutput(QString)", true);
#endif
}


ProtocolView::~ProtocolView()
{
    delete job;
}


bool ProtocolView::startJob(bool isUpdateJob)
{
    m_isUpdateJob = isUpdateJob;

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


Q3PopupMenu* ProtocolView::createPopupMenu(const QPoint &pos)
{
    Q3PopupMenu* menu = Q3TextEdit::createPopupMenu(pos);

    int id = menu->insertItem(i18n("Clear"), this, SLOT( clear() ), 0, -1, 0);

    if( length() == 0 )
        menu->setItemEnabled(id, false);

    return menu;
}


void ProtocolView::cancelJob()
{
    job->cancel();
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
            msg = i18n("[Exited with status %1]\n", exitStatus);
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
    while ( (pos = buf.indexOf('\n')) != -1)
	{
	    QString line = buf.left(pos);
	    if (!line.isEmpty())
                {
		    appendLine(line);
                    emit receivedLine(line);
                }
	    buf = buf.right(buf.length()-pos-1);
	}
}


void ProtocolView::appendLine(const QString &line)
{
    // Escape output line, so that html tags in commit
    // messages aren't interpreted
    const QString escapedLine = Qt::escape(line);

    // When we don't get the output from an update job then
    // just add it to the text edit.
    if( !m_isUpdateJob )
    {
        append(escapedLine);
        return;
    }

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
           ? QString("<font color=\"%1\"><b>%2</b></font>").arg(color.name())
                                                           .arg(escapedLine)
           : escapedLine);
}


#include "protocolview.moc"


// Local Variables:
// c-basic-offset: 4
// End:


