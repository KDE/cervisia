/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2002 Christian Loose <christian.loose@hamburg.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef ADDREPOSITORYDLG_H
#define ADDREPOSITORYDLG_H

#include <kdialogbase.h>

class QButtonGroup;
class KConfig;
class KLineEdit;


class AddRepositoryDialog : public KDialogBase
{
    Q_OBJECT

public:
    AddRepositoryDialog( KConfig& cfg, const QString &repo, QWidget *parent=0, 
                         const char *name=0 );
    virtual ~AddRepositoryDialog();

    void setRepository(const QString &repo);
    void setRsh(const QString &rsh);
    void setCompression(int compression);
    
    QString repository() const;
    QString rsh() const;
    int compression() const;

private slots:
    void repoChanged();
    
private:
    KLineEdit *repo_edit;
    KLineEdit *rsh_edit;
    QButtonGroup *compression_group;
    KConfig& partConfig;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
