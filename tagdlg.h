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


#ifndef TAGDLG_H
#define TAGDLG_H


#include <kdialogbase.h>


class QCheckBox;
class QComboBox;
class QLineEdit;


class TagDialog : public KDialogBase
{
    Q_OBJECT

public:
    enum ActionType { Create, Delete };
    
    TagDialog( ActionType action, const QString &sbox, const QString &repo,
               QWidget *parent=0, const char *name=0 );

    bool branchTag() const;
    bool forceTag() const;
    QString tag() const;

protected:
    virtual void slotOk();

private slots:
    void tagButtonClicked();

private:
    ActionType act;
    QString sandbox, repository;
    
    QCheckBox *branchtag_button;
    QCheckBox *forcetag_button;
    QLineEdit *tag_edit;
    QComboBox *tag_combo;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
