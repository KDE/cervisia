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


#include "annotatedlg.h"

#include "annotateview.h"


AnnotateDialog::AnnotateDialog(QWidget *parent, const char *name)
    : KDialogBase(parent, name, false, QString::null,
                  Close | Help, Close, true)
{
    annotate = new AnnotateView(this);
    setMainWidget(annotate);

    setHelp("annotate");

    setWFlags(Qt::WDestructiveClose | getWFlags());

    QSize size = configDialogSize("AnnotateDialog");
    resize(size);
}


AnnotateDialog::~AnnotateDialog()
{
    saveDialogSize("AnnotateDialog", true);
}


void AnnotateDialog::addLine(const QString &rev, const QString &author, const QDate &date,
                             const QString &content, const QString &comment, bool odd)
{
    annotate->addLine(rev, author, date, content, comment, odd);
}


// Local Variables:
// c-basic-offset: 4
// End:
