/* 
 *  Copyright (C) 1999-2002 Bernd Gehrmann
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


#ifndef WATCHDLG_H
#define WATCHDLG_H

#include <qdialog.h>

class QButtonGroup;
class QRadioButton;
class QCheckBox;


class WatchDialog : public QDialog
{
    Q_OBJECT

public:
    enum ActionType { Add, Remove };
    enum Events { None=0, All=1, Commits=2, Edits=4, Unedits=8 };
    
    WatchDialog( ActionType action, QWidget *parent=0, const char *name=0 );
    Events events();

private slots:
    void helpClicked();

private:
    QButtonGroup *group;
    QRadioButton *all_button, *only_button;
    QCheckBox *commitbox, *editbox, *uneditbox;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
