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


#ifndef CERVISIASHELL_H
#define CERVISIASHELL_H

#include <kparts/mainwindow.h>

class QLabel;
class CervisiaPart;
class KRecentFilesAction;

/**
 * A basic shell that embeds the Cervisia part directly to make a standalone
 * GUI available.
 */
class CervisiaShell : public KParts::MainWindow
{
    Q_OBJECT

public:
    CervisiaShell( const char *name=0 );
    virtual ~CervisiaShell();

    void restorePseudo(const QString &dirname);

public slots:
    void slotOpenSandbox();
    void slotConfigureKeys();
    void slotConfigureToolBars();
    void slotExit();

protected slots:
    void slotNewToolbarConfig();
    void slotChangeFilterStatus(QString status);

protected:
    void setupActions();

    bool queryExit();
    virtual void readProperties(KConfig *config);
    virtual void saveProperties(KConfig *config);

private:
    CervisiaPart *part;
    QLabel *filterLabel;

};

#endif // CERVISIASHELL_H

// Local Variables:
// c-basic-offset: 4
// End:
