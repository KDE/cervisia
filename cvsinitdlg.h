/*
 *  Copyright (c) 2004 Christian Loose <christian.loose@kdemail.net>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef CERVISIA_CVSINITDLG_H
#define CERVISIA_CVSINITDLG_H

#include <kdialogbase.h>

class KLineEdit;


namespace Cervisia
{


class CvsInitDialog : public KDialogBase
{
    Q_OBJECT

public:
    CvsInitDialog(QWidget* parent = 0, const char* name = 0);

    QString directory() const;
   
private slots:
    void dirButtonClicked();
    void lineEditTextChanged(const QString& text);

private:   
    KLineEdit* m_directoryEdit;
};


}

#endif
