/* 
 *  Copyright (c) 1999-2002 Bernd Gehrmann <bernd@mail.berlios.de>
 *  Copyright (c) 2002      Christian Loose <christian.loose@hamburg.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef PROGRESSDLG_H
#define PROGRESSDLG_H

#include <dcopobject.h>
#include <kdialogbase.h>

class QString;
class QWidget;
class DCOPRef;


class ProgressDialog : public KDialogBase, public DCOPObject
{
    K_DCOP
    Q_OBJECT

public:
    ProgressDialog(QWidget* parent, const QString& heading, const DCOPRef& job,
                   const QString& errorIndicator, const QString& caption = "");
    ~ProgressDialog();

    bool execute();
    bool getLine(QString& line);

k_dcop:
    void slotReceivedOutputNonGui(QString buffer);
    void slotReceivedOutput(QString buffer);
    void slotJobExited(bool normalExit, int status);

protected slots:
    virtual void slotCancel();

private slots:
    void slotTimeoutOccurred();

private:
    void setupGui(const QString& heading);
    void stopNonGuiPart();
    void startGuiPart();
    void processOutput();

    struct Private;
    Private* d;
};


#endif

