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


#ifndef REPOSITORYDLG_H
#define REPOSITORYDLG_H

#include <qdialog.h>
#include <qbuttongroup.h>
#include <klineedit.h>

class QListViewItem;
class KConfig;
class ListView;


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
    void slotDoubleClicked(QListViewItem *item);
    void slotSettingsClicked();
    void slotLoginClicked();
    void slotLogoutClicked();
    void helpClicked();

private:
    struct Options {
        QSize size;
    };
    static Options *options;

    ListView *repolist;
};


class AddRepositoryDialog : public QDialog
{
    Q_OBJECT

public:
    AddRepositoryDialog( const QString &repo, QWidget *parent=0, const char *name=0 );

    void setRepository(const QString &repo);
    void setRsh(const QString &rsh)
        { rsh_edit->setText(rsh); }
    void setCompression(int compression)
        { compression_group->setButton(compression+1); }
    QString repository() const
        { return repo_edit->text(); }
    QString rsh() const
        { return rsh_edit->text(); }
    int compression() const
        { return compression_group->id(compression_group->selected())-1; }
    
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
    QButtonGroup *compression_group;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
