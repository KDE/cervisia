/* 
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef ADDREMOVEDLG_H
#define ADDREMOVEDLG_H


#include <kdialogbase.h>


class QListBox;
class QStringList;


class AddRemoveDialog : public KDialogBase
{
public:
    enum ActionType { Add, AddBinary, Remove };
    
    explicit AddRemoveDialog( ActionType action, QWidget *parent=0, const char *name=0 );

    void setFileList(const QStringList &list);

private:
    QListBox *listbox;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
