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

#include <kconfig.h>

#include "annotateview.h"


AnnotateDialog::Options *AnnotateDialog::options = 0;


AnnotateDialog::AnnotateDialog(QWidget *parent, const char *name)
    : KDialogBase(parent, name, false, QString::null,
                  Close | Help, Close, true)
{
    annotate = new AnnotateView(this);
    setMainWidget(annotate);

    setHelp("annotate");

    setWFlags(Qt::WDestructiveClose | getWFlags());

    if (options)
        resize(options->size);
}


AnnotateDialog::~AnnotateDialog()
{
    if (!options)
        options = new Options;
    options->size = size();
}


void AnnotateDialog::loadOptions(KConfig *config)
{
    if (!config->readEntry("Customized"))
        return;

    options = new Options;
    options->size = config->readSizeEntry("Size");
}


void AnnotateDialog::saveOptions(KConfig *config)
{
    if (!options)
        return;

    config->writeEntry("Customized", true);
    config->writeEntry("Size", options->size);
}


void AnnotateDialog::addLine(const QString &rev, const QString &author, const QDate &date,
                             const QString &content, const QString &comment, bool odd)
{
    annotate->addLine(rev, author, date, content, comment, odd);
}


// Local Variables:
// c-basic-offset: 4
// End:
