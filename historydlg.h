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
class ListView;


class HistoryDialog : public KDialogBase
{
    Q_OBJECT

public:

    explicit HistoryDialog( QWidget *parent=0, const char *name=0 );

    virtual ~HistoryDialog();

    bool parseHistory(const QString &sandbox, const QString &repository);

    static void loadOptions(KConfig *config);
    static void saveOptions(KConfig *config);
    
private slots:
    void choiceChanged();
    void toggled(bool b);

private:
    struct Options {
        QSize size;
    };
    static Options *options;

    ListView *listview;
    QCheckBox *commit_box, *checkout_box, *tag_box, *other_box;
    QCheckBox *onlyuser_box, *onlyfilenames_box, *onlydirnames_box;
    KLineEdit *user_edit, *filename_edit, *dirname_edit;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
