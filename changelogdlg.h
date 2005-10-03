/* 
 *  Copyright (C) 1999-2001 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2002-2003 Christian Loose <christian.loose@hamburg.de>
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


#ifndef CHANGELOGDLG_H
#define CHANGELOGDLG_H

#include <kdialogbase.h>

class KTextEdit;
class KConfig;


class ChangeLogDialog : public KDialogBase
{
public:
    explicit ChangeLogDialog( KConfig& cfg, QWidget *parent=0, const char *name=0 );

    virtual ~ChangeLogDialog();

    bool readFile(const QString &fileName);
    QString message();

protected:
    virtual void slotOk();

private:
    struct Options {
        QSize size;
    };
    static Options *options;

    QString fname;
    KTextEdit *edit;
    KConfig&   partConfig;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
