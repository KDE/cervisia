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
    QString alias() const;
    bool importBinary() const;
    bool useModificationTime() const;
    bool exportOnly() const;
    bool recursive() const;

protected:
    virtual void slotOk();
    
private slots:
    void dirButtonClicked();
    void moduleButtonClicked();
    void branchButtonClicked();
    void branchTextChanged();

private:
    void saveUserInput();
    void restoreUserInput();
    
    QComboBox *repo_combo, *module_combo, *branchCombo;
    KLineEdit *module_edit, *workdir_edit;
    KLineEdit *comment_edit;
    KLineEdit *vendortag_edit, *releasetag_edit, *ignore_edit, *alias_edit;
    QCheckBox *binary_box, *export_box, *recursive_box;
    QCheckBox* m_useModificationTimeBox;
    ActionType act;
    KConfig&   partConfig;

    CvsService_stub *cvsService;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
