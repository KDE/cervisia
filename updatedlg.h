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

#include <qcombobox.h>
#include <qdialog.h>
#include <qradiobutton.h>
#include <klineedit.h>

class QButtonGroup;


class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    UpdateDialog( const QString &sbox, const QString &repo,
                  QWidget *parent=0, const char *name=0 );
    ~UpdateDialog();
    
    bool byTag() const
        { return bybranch_button->isChecked() || bytag_button->isChecked(); }
    QString tag() const
        { return bybranch_button->isChecked()?
              branch_combo->currentText() : tag_combo->currentText(); }
    QString date() const
        { return date_edit->text(); }

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
    QButtonGroup *group;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
