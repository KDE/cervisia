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


#ifndef CHECKOUTDLG_H
#define CHECKOUTDLG_H


#include <kdialogbase.h>


class QCheckBox;
class QComboBox;
class KConfig;
class KLineEdit;
class CvsService_stub;


class CheckoutDialog : public KDialogBase
{
    Q_OBJECT

public:
    enum ActionType { Checkout, Import };
    
    CheckoutDialog( KConfig& cfg, CvsService_stub* service, ActionType action,
                    QWidget *parent=0, const char *name=0 );

    QString workingDirectory() const;
    QString repository() const;
    QString module() const;
    QString branch() const;
    QString vendorTag() const;
    QString releaseTag() const;
    QString ignoreFiles() const;
    QString comment() const;
    bool importBinary() const;

protected:
    virtual void slotOk();
    
private slots:
    void dirButtonClicked();
    void moduleButtonClicked();

private:
    QComboBox *repo_combo, *module_combo;
    KLineEdit *module_edit, *workdir_edit;
    KLineEdit *branch_edit, *comment_edit;
    KLineEdit *vendortag_edit, *releasetag_edit, *ignore_edit;
    QCheckBox *binary_box;
    ActionType act;
    KConfig&   partConfig;

    CvsService_stub *cvsService;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
