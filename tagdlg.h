/* 
 *  Copyright (C) 1999-2001 Bernd Gehrmann
 *                          bernd@physik.hu-berlin.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef _TAGDLG_H_
#define _TAGDLG_H_

#include <qdialog.h>

class QCheckBox;
class QComboBox;
class KLineEdit;


class TagDialog : public QDialog
{
    Q_OBJECT

public:
    enum ActionType { Create, Delete };
    
    TagDialog( ActionType action, const QString &sbox, const QString &repo,
               QWidget *parent=0, const char *name=0 );
    
    bool branchTag() const
        { return branchtag_button->isChecked(); }
    QString tag() const
        { return act==Delete? tag_combo->currentText() : tag_edit->text(); }

protected:
    virtual void done(int r);

private slots:
    void tagButtonClicked();
    
private:
    ActionType act;
    QString sandbox, repository;
    
    QCheckBox *branchtag_button;
    QLineEdit *tag_edit;
    QComboBox *tag_combo;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
