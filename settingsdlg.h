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


#ifndef SETTINGSDLG_H
#define SETTINGSDLG_H


#include <qpushbutton.h>
#include <kdialogbase.h>


class QCheckBox;
class QComboBox;
class KIntNumInput;
class KLineEdit;
class KConfig;
class KColorButton;
class KURLRequester;


class FontButton : public QPushButton
{
    Q_OBJECT

public:
    FontButton( const QString &text, QWidget *parent=0, const char *name=0 );

private slots:
    void chooseFont();
};


class SettingsDialog : public KDialogBase
{
    Q_OBJECT

public:
    SettingsDialog( KConfig *conf, QWidget *parent=0, const char *name=0 );
    virtual ~SettingsDialog();

protected slots:
     virtual void done(int res);

private:
    void readSettings();
    void writeSettings();

    void addAdvancedPage();
    void addLookAndFeelPage();
    void addColorPage();

    KConfig *config;
    KIntNumInput *timeoutedit;
    KIntNumInput *contextedit;
    KIntNumInput *tabwidthedit;
    KURLRequester *cvspathedit;
    QComboBox *compressioncombo;
    QCheckBox *usesshagent;
    KLineEdit *usernameedit;
    KURLRequester *editoredit;
    KLineEdit *diffoptedit;
    KURLRequester *extdiffedit;
    QCheckBox *remotestatusbox;
    QCheckBox *localstatusbox;
#if 0
    QCheckBox *usedcopbox;
    KLineEdit *clientedit;
    KLineEdit *objectedit;
#endif
    FontButton *protocolfontbox;
    FontButton *annotatefontbox;
    FontButton *difffontbox;
    FontButton *changelogfontbox;
    QCheckBox *splitterbox;
    KColorButton *conflictbutton;
    KColorButton *localchangebutton;
    KColorButton *remotechangebutton;
    KColorButton *diffchangebutton;
    KColorButton *diffinsertbutton;
    KColorButton *diffdeletebutton;

    KConfig* serviceConfig;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
