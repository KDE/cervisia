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


#ifndef UPDATEDLG_H
#define UPDATEDLG_H


#include <kdialogbase.h>


class QComboBox;
class QPushButton;
class QRadioButton;
class KLineEdit;


class UpdateDialog : public KDialogBase
{
    Q_OBJECT

public:
    UpdateDialog( const QString &sbox, const QString &repo,
                  QWidget *parent=0, const char *name=0 );

    bool byTag() const;
    QString tag() const;
    QString date() const;

private slots:
    void toggled();
    void tagButtonClicked();
    void branchButtonClicked();
    
private:
    void buttonClicked(bool branch);
    
    QString sandbox, repository;
    
    QRadioButton *bytag_button, *bybranch_button, *bydate_button;
    QComboBox *tag_combo, *branch_combo;
    QPushButton *tag_button, *branch_button;
    KLineEdit *date_edit;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
