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


#ifndef CHECKOUTDLG_H
#define CHECKOUTDLG_H

#include <qdialog.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <klineedit.h>

class KConfig;


class CheckoutDialog : public QDialog
{
    Q_OBJECT

public:
    enum ActionType { Checkout, Import };
    
    CheckoutDialog( ActionType action, QWidget *parent=0, const char *name=0 );

    QString workingDirectory() const
        { return workdir_edit->text(); }
    QString repository() const
        { return repo_combo->currentText(); }
    QString module() const
        { return act==Import? module_edit->text() : module_combo->currentText(); }
    QString branch() const
        { return branch_edit->text(); }
    QString vendorTag() const
        { return vendortag_edit->text(); }
    QString releaseTag() const
        { return releasetag_edit->text(); }
    QString ignoreFiles() const
        { return ignore_edit->text(); }
    QString comment() const
        { return comment_edit->text(); }
    bool importBinary() const
        { return binary_box->isChecked(); }

    static void loadOptions(KConfig *config);
    static void saveOptions(KConfig *config);
    
protected:
    virtual void done(int r);
    
private slots:
    void dirButtonClicked();
    void moduleButtonClicked();

private:
    struct Options {
        QString repo;
        QString module;
        QString branch;
        QString workdir;
        QString vendortag;
        QString releasetag;
        QString ignorefiles;
        bool binary;
    };
    static Options *options;

    QComboBox *repo_combo, *module_combo;
    QLineEdit *module_edit, *workdir_edit;
    QLineEdit *branch_edit;
    QLineEdit *comment_edit;
    QLineEdit *vendortag_edit, *releasetag_edit, *ignore_edit;
    QCheckBox *binary_box;
    QPushButton *ok_button, *cancel_button;
    ActionType act;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
