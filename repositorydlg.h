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


#ifndef _REPOSITORYDLG_H_
#define _REPOSITORYDLG_H_

#include <qdialog.h>
#include <kconfig.h>
#include <klineedit.h>

class ListView;
class QPushButton;

class RepositoryDialog : public QDialog
{
    Q_OBJECT

public:
    
    RepositoryDialog( QWidget *parent=0, const char *name=0 );

    void readConfigFile();
    void readCvsPassFile(); 

    static void loadOptions(KConfig *config);
    static void saveOptions(KConfig *config);

protected:
    virtual void done(int r);
    
private slots:
    void slotAddClicked();
    void slotRemoveClicked();
    void slotSettingsClicked();
    void slotLoginClicked();
    void slotLogoutClicked();
    void slotSelectionChanged();
private:
    struct Options {
        QSize size;
    };
    static Options *options;

    ListView *repolist;
    QPushButton *removebutton,*settingsbutton ;
};


class AddRepositoryDialog : public QDialog
{
    Q_OBJECT

public:
    
    AddRepositoryDialog( QWidget *parent=0, const char *name=0 );

    QString repository() const
        { return repo_edit->text(); }
    QString rsh() const
        { return rsh_edit->text(); }
    
    static void loadOptions(KConfig *config);
    static void saveOptions(KConfig *config);

protected:
    virtual void done(int r);
    
private slots:
    void repoChanged();
    
private:
    struct Options {
        QSize size;
    };
    static Options *options;

    KLineEdit *repo_edit;
    KLineEdit *rsh_edit;
    QPushButton *ok;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
