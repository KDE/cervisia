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

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdialog.h>
#include <klineedit.h>


class TagDialog : public QDialog
{
    Q_OBJECT

public:
    enum ActionType { Create, Delete };
    
    TagDialog( ActionType action, const QString &sbox, const QString &repo,
               QWidget *parent=0, const char *name=0 );
    
    bool branchTag() const
        { return branchtag_button && branchtag_button->isChecked(); } 
    bool forceTag() const
        { return forcetag_button && forcetag_button->isChecked(); }
    QString tag() const
        { return act==Delete? tag_combo->currentText() : tag_edit->text(); }

protected:
    virtual void done(int r);

private slots:
    void tagButtonClicked();
    void helpClicked();
    
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
