/* 
 *  Copyright (C) 2001 Colin MacLeod
 *                     colin.macleod@ivata.com
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _REPOSITORY_SETTINGS_DLG_H
#define _REPOSITORY_SETTINGS_DLG_H

#include <qvariant.h>
#include <qdialog.h>
#include <qstring.h>
class QBoxLayout; 
class QGridLayout; 
class QButtonGroup;
class QPushButton;
class QRadioButton;

class RepositorySettingsDialog : public QDialog
{ 
    Q_OBJECT

public:
    RepositorySettingsDialog( QWidget* parent = 0, const char* name = 0 );
    ~RepositorySettingsDialog();

    QButtonGroup* groupCompression;
    QRadioButton* radioCompressionDefault;
    QRadioButton* radioCompression0;
    QRadioButton* radioCompression1;
    QRadioButton* radioCompression2;
    QRadioButton* radioCompression3;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;

// accessor methods
    void setRepository( QString sNew );

// convenience inline methods
    inline static QString getConfigGroup( QString sRepository ) {
      return QString( "Repository-" ) + sRepository;
    }
public slots:
    virtual void slotOkClicked();

protected:
    QBoxLayout* RepositorySettingsDialogLayout;

private:
    QString sRepository;

};

#endif // REPOSITORY_SETTINGS_DLG_H
