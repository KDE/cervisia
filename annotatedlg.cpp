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


AnnotateDialog::AnnotateDialog(KConfig& cfg, QWidget *parent, const char *name)
    : KDialogBase(parent, name, false, QString::null,
                  Close | Help, Close, true)
    , partConfig(cfg)
{
    annotate = new AnnotateView(partConfig, this);
    setMainWidget(annotate);

    setHelp("annotate");

    setWFlags(Qt::WDestructiveClose | getWFlags());

    QSize size = configDialogSize(partConfig, "AnnotateDialog");
    resize(size);
}


AnnotateDialog::~AnnotateDialog()
{
    saveDialogSize(partConfig, "AnnotateDialog");
}


void AnnotateDialog::addLine(const Cervisia::LogInfo& logInfo,
                             const QString& content, bool odd)
{
    annotate->addLine(logInfo, content, odd);
}


// Local Variables:
// c-basic-offset: 4
// End:
