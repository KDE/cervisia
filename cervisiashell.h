// -*- c++ -*-

#ifndef CERVISIASHELL_H
#define CERVISIASHELL_H

#include <kparts/mainwindow.h>

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
    void slotToggleToolbar( bool visible );
    void slotConfigureKeys();
    void slotConfigureToolBars();
    void slotExit();

protected:
    void setupActions();

    bool queryExit();
    virtual void readProperties(KConfig *config);
    virtual void saveProperties(KConfig *config);

private:
    CervisiaPart *part;

};

#endif // CERVISIASHELL_H

// Local Variables:
// c-basic-offset: 4
// End:
