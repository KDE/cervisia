/* 
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef ANNOTATEDLG_H
#define ANNOTATEDLG_H


#include <kdialogbase.h>


class KConfig;
class AnnotateView;
class DCOPRef;


class AnnotateDialog : public KDialogBase
{
public:

    explicit AnnotateDialog( QWidget *parent=0, const char *name=0 );

    virtual ~AnnotateDialog();

    bool parseCvsAnnotate(const QString &sandbox, const QString &repository,
                          const QString &filename, const QString &rev);

    bool parseCvsAnnotate(DCOPRef& cvsService, const QString& fileName,
                          const QString& revision = "");

    static void loadOptions(KConfig *config);
    static void saveOptions(KConfig *config);

private:

    struct Options {
        QSize size;
    };
    static Options *options;

    AnnotateView *annotate;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
