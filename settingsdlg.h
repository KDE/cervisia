/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2002-2004 Christian Loose <christian.loose@kdemail.net>
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

#ifndef SETTINGSDLG_H
#define SETTINGSDLG_H


#include <qpushbutton.h>
#include <kdialogbase.h>


class QCheckBox;
class KIntNumInput;
class KLineEdit;
class KConfig;
class KColorButton;
class KURLRequester;
class AdvancedPage;


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

    void addGeneralPage();
    void addDiffPage();
    void addStatusPage();
    void addAdvancedPage();
    void addLookAndFeelPage();

    KConfig *config;
    KIntNumInput *contextedit;
    KIntNumInput *tabwidthedit;
    KURLRequester *cvspathedit;
    KLineEdit *usernameedit;
    KLineEdit *diffoptedit;
    KURLRequester *extdiffedit;
    QCheckBox *remotestatusbox;
    QCheckBox *localstatusbox;
    FontButton*   m_protocolFontBox;
    FontButton*   m_annotateFontBox;
    FontButton*   m_diffFontBox;
    FontButton*   m_changelogFontBox;

    KColorButton* m_conflictButton;
    KColorButton* m_localChangeButton;
    KColorButton* m_remoteChangeButton;
    KColorButton* m_notInCvsButton;
    KColorButton* m_diffChangeButton;
    KColorButton* m_diffInsertButton;
    KColorButton* m_diffDeleteButton;

    QCheckBox*    m_splitterBox;
    AdvancedPage* m_advancedPage;

    KConfig* serviceConfig;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
