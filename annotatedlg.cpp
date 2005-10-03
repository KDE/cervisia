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
