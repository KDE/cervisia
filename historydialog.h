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

#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QDialog>

class QCheckBox;
class KConfig;
class QLineEdit;
class QTreeWidget;
class OrgKdeCervisia5CvsserviceCvsserviceInterface;

class HistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HistoryDialog(KConfig &cfg, QWidget *parent = nullptr);
    ~HistoryDialog() override;

    bool parseHistory(OrgKdeCervisia5CvsserviceCvsserviceInterface *cvsService);

private Q_SLOTS:
    void slotHelp();
    void choiceChanged();
    void toggled(bool b);

private:
    QTreeWidget *listview;
    QCheckBox *commit_box, *checkout_box, *tag_box, *other_box;
    QCheckBox *onlyuser_box, *onlyfilenames_box, *onlydirnames_box;
    QLineEdit *user_edit, *filename_edit, *dirname_edit;
    KConfig &partConfig;
};

#endif // HISTORYDIALOG_H

// Local Variables:
// c-basic-offset: 4
// End:
