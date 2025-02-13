/*
 *  Copyright (c) 2004 Christian Loose <christian.loose@kdemail.net>
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

#ifndef CVSINITDIALOG_H
#define CVSINITDIALOG_H

#include <QDialog>

class KLineEdit;
class QPushButton;

namespace Cervisia
{

class CvsInitDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CvsInitDialog(QWidget *parent = nullptr);

    QString directory() const;

private Q_SLOTS:
    void dirButtonClicked();
    void lineEditTextChanged(const QString &text);

private:
    KLineEdit *m_directoryEdit;
    QPushButton *okButton;
};
}

#endif // CVSINITDIALOG_H
