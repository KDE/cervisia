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


#ifndef HISTORYDLG_H
#define HISTORYDLG_H


#include <kdialogbase.h>


class QCheckBox;
class KConfig;
class KLineEdit;
class KListView;
class CvsService_stub;


class HistoryDialog : public KDialogBase
{
    Q_OBJECT

public:
    explicit HistoryDialog( KConfig& cfg, QWidget *parent=0, const char *name=0 );
    virtual ~HistoryDialog();

    bool parseHistory(CvsService_stub* cvsService);

private slots:
    void choiceChanged();
    void toggled(bool b);

private:
    KListView *listview;
    QCheckBox *commit_box, *checkout_box, *tag_box, *other_box;
    QCheckBox *onlyuser_box, *onlyfilenames_box, *onlydirnames_box;
    KLineEdit *user_edit, *filename_edit, *dirname_edit;
    KConfig& partConfig;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
