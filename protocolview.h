/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2003-2004 Christian Loose <christian.loose@kdemail.net>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef PROTOCOLVIEW_H
#define PROTOCOLVIEW_H

#include <qtextedit.h>
#include <dcopobject.h>

class QPoint;
class QPopupMenu;
class CvsJob_stub;


class ProtocolView : public QTextEdit, public DCOPObject
{
    K_DCOP
    Q_OBJECT

public:
    explicit ProtocolView(const QCString& appId, QWidget *parent=0, const char *name=0);
    ~ProtocolView();

    bool startJob(bool isUpdateJob = false);

protected:
    virtual QPopupMenu* createPopupMenu(const QPoint &pos);

k_dcop:
    void slotReceivedOutput(QString buffer);
    void slotJobExited(bool normalExit, int exitStatus);

signals:
    void receivedLine(QString line);
    void jobFinished(bool normalExit, int exitStatus);

private:
    void processOutput();
    void appendLine(const QString &line);

    QString buf;

    QColor conflictColor;
    QColor localChangeColor;
    QColor remoteChangeColor;

    CvsJob_stub* job;

    bool   m_isUpdateJob;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
