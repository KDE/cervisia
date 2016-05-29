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

#ifndef CHECKOUTDIALOG_H
#define CHECKOUTDIALOG_H

#include <QDialog>

class QCheckBox;
class KComboBox;
class KConfig;
class KLineEdit;
class QLineEdit;
class OrgKdeCervisia5CvsserviceCvsserviceInterface;

class CheckoutDialog : public QDialog
{
    Q_OBJECT

public:
    enum ActionType { Checkout, Import };
    
    CheckoutDialog( KConfig& cfg, OrgKdeCervisia5CvsserviceCvsserviceInterface* service, ActionType action,
                    QWidget *parent=0);

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

protected slots:
    void slotOk();
    
private slots:
    void slotHelp();
    void dirButtonClicked();
    void moduleButtonClicked();
    void branchButtonClicked();
    void branchTextChanged();

private:
    void saveUserInput();
    void restoreUserInput();
    
    KComboBox *repo_combo, *module_combo, *branchCombo;
    KLineEdit *workdir_edit;
    QLineEdit *module_edit, *comment_edit;
    QLineEdit *vendortag_edit, *releasetag_edit, *ignore_edit, *alias_edit;
    QCheckBox *binary_box, *export_box, *recursive_box;
    QCheckBox* m_useModificationTimeBox;
    ActionType act;
    KConfig&   partConfig;
    QString helpTopic;

    OrgKdeCervisia5CvsserviceCvsserviceInterface *cvsService;
};

#endif // CHECKOUTDIALOG_H

// Local Variables:
// c-basic-offset: 4
// End:
