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
