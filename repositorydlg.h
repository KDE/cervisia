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


class QButtonGroup;
class QListViewItem;
class KConfig;
class KLineEdit;

class ListView;


class RepositoryDialog : public KDialogBase
{
    Q_OBJECT

public:
    explicit RepositoryDialog( QWidget *parent=0, const char *name=0 );

    virtual ~RepositoryDialog();

    void readConfigFile();
    void readCvsPassFile(); 

    static void loadOptions(KConfig *config);
    static void saveOptions(KConfig *config);

protected:
    virtual void slotOk();

private slots:
    void slotAddClicked();
    void slotRemoveClicked();
    void slotDoubleClicked(QListViewItem *item);
    void slotSettingsClicked();
    void slotLoginClicked();
    void slotLogoutClicked();

private:
    struct Options {
        QSize size;
    };
    static Options *options;

    ListView *repolist;
};


class AddRepositoryDialog : public KDialogBase
{
    Q_OBJECT

public:
    explicit AddRepositoryDialog( const QString &repo, QWidget *parent=0, const char *name=0 );

    virtual ~AddRepositoryDialog();

    void setRepository(const QString &repo);
    void setRsh(const QString &rsh);
    void setCompression(int compression);
    QString repository() const;
    QString rsh() const;
    int compression() const;

    static void loadOptions(KConfig *config);
    static void saveOptions(KConfig *config);

private slots:
    void repoChanged();
    
private:
    struct Options {
        QSize size;
    };
    static Options *options;

    KLineEdit *repo_edit;
    KLineEdit *rsh_edit;
    QButtonGroup *compression_group;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
