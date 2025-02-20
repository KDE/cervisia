/*
 *  Copyright (C) 1999-2001 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2002-2007 Christian Loose <christian.loose@hamburg.de>
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

#ifndef CHANGELOGDIALOG_H
#define CHANGELOGDIALOG_H

#include <QDialog>

class QPlainTextEdit;
class KConfig;

class ChangeLogDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ChangeLogDialog(KConfig &cfg, QWidget *parent = nullptr);

    ~ChangeLogDialog() override;

    bool readFile(const QString &fileName);
    QString message();

protected Q_SLOTS:
    void slotOk();

private:
    struct Options {
        QSize size;
    };
    static Options *options;

    QString fname;
    QPlainTextEdit *edit;
    KConfig &partConfig;
};

#endif // CHANGELOGDIALOG_H

// Local Variables:
// c-basic-offset: 4
// End:
