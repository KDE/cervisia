/* 
 *  Copyright (C) 1999-2001 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2002-2003 Christian Loose <christian.loose@hamburg.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
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
