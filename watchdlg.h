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


#ifndef WATCHDLG_H
#define WATCHDLG_H


#include <kdialogbase.h>


class QRadioButton;
class QCheckBox;


class WatchDialog : public KDialogBase
{
public:
    enum ActionType { Add, Remove };
    enum Events { None=0, All=1, Commits=2, Edits=4, Unedits=8 };

    explicit WatchDialog( ActionType action, QWidget *parent=0, const char *name=0 );

    Events events() const;

private:
    QRadioButton *all_button, *only_button;
    QCheckBox *commitbox, *editbox, *uneditbox;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
