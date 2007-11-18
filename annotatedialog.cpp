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


#include "annotatedialog.h"

#include "annotateview.h"

#include <kconfig.h>
#include <kconfiggroup.h>


AnnotateDialog::AnnotateDialog(KConfig& cfg, QWidget *parent)
    : KDialog(parent)
    , partConfig(cfg)
{
    setButtons(Close | Help);
    setDefaultButton(Close);
    showButtonSeparator(true);

    annotate = new AnnotateView(this);
    setMainWidget(annotate);

    setHelp("annotate");

    setAttribute(Qt::WA_DeleteOnClose, true);

    KConfigGroup cg(&partConfig, "AnnotateDialog");
    restoreDialogSize(cg);
}


AnnotateDialog::~AnnotateDialog()
{
    KConfigGroup cg(&partConfig, "AnnotateDialog");
    saveDialogSize(cg);
}


void AnnotateDialog::addLine(const Cervisia::LogInfo& logInfo,
                             const QString& content, bool odd)
{
    annotate->addLine(logInfo, content, odd);
}


// Local Variables:
// c-basic-offset: 4
// End:
