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

#ifndef TAGDIALOG_H
#define TAGDIALOG_H

#include <QDialog>

class QCheckBox;
class KComboBox;
class QLineEdit;
class OrgKdeCervisia5CvsserviceCvsserviceInterface;

namespace Cervisia
{

class TagDialog : public QDialog
{
    Q_OBJECT

public:
    enum ActionType { Create, Delete };

    TagDialog(ActionType action, OrgKdeCervisia5CvsserviceCvsserviceInterface *service, QWidget *parent = nullptr);

    bool branchTag() const;
    bool forceTag() const;
    QString tag() const;

protected Q_SLOTS:
    void slotOk();
    void slotHelp();

private Q_SLOTS:
    void tagButtonClicked();

private:
    ActionType act;
    OrgKdeCervisia5CvsserviceCvsserviceInterface *cvsService;

    QCheckBox *branchtag_button;
    QCheckBox *forcetag_button;
    QLineEdit *tag_edit;
    KComboBox *tag_combo;
};

}

#endif // TAGDIALOG_H

// Local Variables:
// c-basic-offset: 4
// End:
