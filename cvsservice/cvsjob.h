/*
 * Copyright (c) 2002 Christian Loose <christian.loose@hamburg.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#ifndef CVSJOB_H
#define CVSJOB_H

#include <qobject.h>
#include <dcopobject.h>

class QString;
class KProcess;


class CvsJob : public QObject, public DCOPObject
{
    Q_OBJECT
    K_DCOP

public:
    CvsJob(unsigned jobId, const QString& cvsCommand);
    ~CvsJob();

    void setCvsCommand(const QString& cvsCommand);

k_dcop:
    bool execute();
    void cancel();

    bool isRunning() const;

    QString cvsCommand() const;

k_dcop_signals:
    void jobExited(bool normalExit, int status);
    void receivedStdout(QString buffer);
    void receivedStderr(QString buffer);

private slots:
    void slotProcessExited();
    void slotReceivedStdout(KProcess* proc, char* buffer, int buflen);
    void slotReceivedStderr(KProcess* proc, char* buffer, int buflen);

private:
    struct Private;
    Private* d;
};


#endif

