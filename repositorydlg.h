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


#ifndef REPOSITORYDLG_H
#define REPOSITORYDLG_H


#include <kdialogbase.h>


class QListViewItem;
class QPushButton;
class KConfig;
class KListView;


class RepositoryDialog : public KDialogBase
{
    Q_OBJECT

public:
    explicit RepositoryDialog( KConfig& cfg, QWidget *parent=0, const char *name=0 );

    virtual ~RepositoryDialog();

    void readConfigFile();
    void readCvsPassFile();

protected:
    virtual void slotOk();

private slots:
    void slotAddClicked();
    void slotModifyClicked();
    void slotRemoveClicked();
    void slotDoubleClicked(QListViewItem *item);
    void slotLoginClicked();
    void slotLogoutClicked();
    void slotSelectionChanged();

private:
    KListView *repolist;
    KConfig* serviceConfig;
    KConfig& partConfig;

    QPushButton* m_modifyButton;
    QPushButton* m_removeButton;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
