/* 
 *  Copyright (C) 1999-2001 Bernd Gehrmann
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


#ifndef _CHANGELOGDLG_H_
#define _CHANGELOGDLG_H_

#include <qdialog.h>


class ChangeLogDialog : public QDialog
{
    Q_OBJECT
    
public:
    ChangeLogDialog( QWidget *parent=0, const char *name=0 );

    bool readFile(const QString &filename);
    QString message();

    static void loadOptions(KConfig *config);
    static void saveOptions(KConfig *config);
    
protected slots:
    virtual void done(int res);

private:
    struct Options {
        QSize size;
    };
    static Options *options;

    QString fname;
    QMultiLineEdit *edit;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
