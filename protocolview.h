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


#ifndef PROTOCOLVIEW_H
#define PROTOCOLVIEW_H

#include <qtextedit.h>
#include <kprocess.h>


class ProtocolView : public QTextEdit
{
    Q_OBJECT
    
public:
    ProtocolView( QWidget *parent=0, const char *name=0 );
    ~ProtocolView();

    bool startJob(const QString &sandbox, const QString &repository, const QString &cmdline);

signals:
    void receivedLine(QString line);
    void jobFinished(bool success);

protected:
    void mousePressEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    
private slots:
    void receivedOutput(KProcess *proc, char *buffer, int buflen);
    void childExited();
    void cancelJob();
    
private:
    void processOutput();
    void appendLine(const QString &line);
    void execContextMenu(const QPoint &pos);
    
    KShellProcess *childproc;
    QString buf;
    
    QColor conflictColor;
    QColor localChangeColor;
    QColor remoteChangeColor;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
