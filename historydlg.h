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


#ifndef _HISTORYDLG_H_
#define _HISTORYDLG_H_

#include <qdialog.h>


class QCheckBox;
class KConfig;
class KLineEdit;
class ListView;


class HistoryDialog : public QDialog
{
    Q_OBJECT

public:
    HistoryDialog( QWidget *parent=0, const char *name=0 );

    bool parseHistory(const QString &sandbox, const QString &repository);

    static void loadOptions(KConfig *config);
    static void saveOptions(KConfig *config);
    
private slots:
    virtual void done(int r);
    void choiceChanged();
    void toggled(bool b);
    
private:
    struct Options {
        QSize size;
    };
    static Options *options;

    friend class HistoryItem;
    class ItemCopy {
    public:
        ItemCopy() {}
        bool isCommit();
        bool isCheckout();
        bool isTag();
        bool isOther();
        QString text[7];
        QString index;
    };
    QList<ItemCopy> hiddenitems;
    
    ListView *listview;
    QCheckBox *commit_box, *checkout_box, *tag_box, *other_box;
    QCheckBox *onlyuser_box, *onlyfilenames_box, *onlydirnames_box;
    KLineEdit *user_edit, *filename_edit, *dirname_edit;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
