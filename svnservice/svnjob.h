/*
 * Copyright (c) 2005 Christian Loose <christian.loose@kdemail.net>
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

#ifndef SVNJOB_H
#define SVNJOB_H

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <dcopobject.h>

class KProcess;


class KDE_EXPORT SvnJob : public QObject, public DCOPObject
{
    Q_OBJECT
    K_DCOP

public:
    explicit SvnJob(unsigned jobNum);
    explicit SvnJob(const QString& objId);
    virtual ~SvnJob();

    void clearCommand();
    void setDirectory(const QString& directory);

    SvnJob& operator<<(const QString& arg);
    SvnJob& operator<<(const char* arg);
    SvnJob& operator<<(const QCString& arg);
    SvnJob& operator<<(const QStringList& args);

k_dcop:
    bool execute();
    void cancel();

    bool isRunning() const;

    QString command() const;

    QStringList output() const;

k_dcop_signals:
    void jobExited(bool normalExit, int status);
    void receivedStdout(const QString& buffer);
    void receivedStderr(const QString& buffer);

private slots:
    void slotProcessExited();
    void slotReceivedStdout(KProcess* proc, char* buffer, int buflen);
    void slotReceivedStderr(KProcess* proc, char* buffer, int buflen);

private:
    struct Private;
    Private* d;
};


#endif
