/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2002-2003 Christian Loose <christian.loose@hamburg.de>
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
    void openURL();
    void openURL(const KURL& url);
    void slotConfigureKeys();
    void slotConfigureToolBars();

protected slots:
    void slotNewToolbarConfig();

protected:
    void setupActions();
    
    bool queryExit();

private:
    void readSettings();
    void writeSettings();

    KParts::ReadOnlyPart *part;
    QString               m_lastOpenDir;
};

#endif // CERVISIASHELL_H

// Local Variables:
// c-basic-offset: 4
// End:
