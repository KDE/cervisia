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

#include <qdialog.h>
#include <qradiobutton.h>
#include <qcombobox.h>

class QButtonGroup;


class MergeDialog : public QDialog
{
    Q_OBJECT

public:
    MergeDialog( const QString &sbox, const QString &repo,
                 QWidget *parent=0, const char *name=0 );
    ~MergeDialog();
    
    bool byBranch() const
        { return bybranch_button->isChecked(); }
    QString branch() const
        { return branch_combo->currentText(); }
    QString tag1() const
        { return tag1_combo->currentText(); }
    QString tag2() const
        { return tag2_combo->currentText(); }

private slots:
    void toggled();
    void tagButtonClicked();
    void branchButtonClicked();
    
private:
    void buttonClicked(bool branch);
    
    QString sandbox, repository;
    
    QRadioButton *bybranch_button, *bytags_button;
    QComboBox *branch_combo, *tag1_combo, *tag2_combo;
    QPushButton *tag_button, *branch_button;
    QButtonGroup *group;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
