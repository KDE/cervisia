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


#ifndef MERGEDLG_H
#define MERGEDLG_H


#include <kdialogbase.h>


class QComboBox;
class QPushButton;
class QRadioButton;


class MergeDialog : public KDialogBase
{
    Q_OBJECT

public:
    MergeDialog( const QString &sbox, const QString &repo,
                 QWidget *parent=0, const char *name=0 );

    bool byBranch() const;
    QString branch() const;
    QString tag1() const;
    QString tag2() const;

private slots:
    void toggled();
    void tagButtonClicked();
    void branchButtonClicked();
    
private:

    QString sandbox, repository;
    
    QRadioButton *bybranch_button, *bytags_button;
    QComboBox *branch_combo, *tag1_combo, *tag2_combo;
    QPushButton *tag_button, *branch_button;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
