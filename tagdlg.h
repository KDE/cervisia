/* 
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef TAGDLG_H
#define TAGDLG_H


#include <kdialogbase.h>


class QCheckBox;
class QComboBox;
class QLineEdit;
class CvsService_stub;

namespace Cervisia
{

class TagDialog : public KDialogBase
{
    Q_OBJECT

public:
    enum ActionType { Create, Delete };
    
    TagDialog( ActionType action, CvsService_stub* service,
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
    CvsService_stub* cvsService;
    
    QCheckBox *branchtag_button;
    QCheckBox *forcetag_button;
    QLineEdit *tag_edit;
    QComboBox *tag_combo;
};

}


#endif


// Local Variables:
// c-basic-offset: 4
// End:
