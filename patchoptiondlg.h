/*
 *  Copyright (C) 2004 Christian Loose <christian.loose@kdemail.net>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef PATCHOPTIONDLG_H
#define PATCHOPTIONDLG_H

#include <kdialogbase.h>

class QCheckBox;
class QVButtonGroup;
class KIntNumInput;


namespace Cervisia
{


class PatchOptionDialog : public KDialogBase
{
    Q_OBJECT
    
public:
    explicit PatchOptionDialog(QWidget* parent = 0, const char* name = 0);
    virtual ~PatchOptionDialog();
    
    QString diffOptions() const;
    QString formatOption() const;

private slots:
    void formatChanged(int buttonId);
       
private:
    QVButtonGroup* m_formatBtnGroup;
    KIntNumInput*  m_contextLines;
    QCheckBox*     m_blankLineChk;
    QCheckBox*     m_allSpaceChk;
    QCheckBox*     m_spaceChangeChk;
};


}

#endif
