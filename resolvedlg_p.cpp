/* 
 *  Copyright (c) 2004 Christian Loose <christian.loose@kdemail.net>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "resolvedlg_p.h"
using namespace Cervisia;

#include <ktextedit.h>


ResolveEditorDialog::ResolveEditorDialog(KConfig& cfg, QWidget *parent, const char *name)
    : KDialogBase(parent, name, true, QString::null,
                  Ok | Cancel, Ok, true)
    , m_partConfig(cfg)
{
    m_edit = new KTextEdit(this);
    m_edit->setFocus();

    setMainWidget(m_edit);

    QFontMetrics const fm(fontMetrics());
    setMinimumSize(fm.width('0') * 120,
                   fm.lineSpacing() * 40);

    QSize size = configDialogSize(m_partConfig, "ResolveEditDialog");
    resize(size);
}


ResolveEditorDialog::~ResolveEditorDialog()
{
    saveDialogSize(m_partConfig, "ResolveEditDialog");
}


void ResolveEditorDialog::setContent(const QString& text)
{
    m_edit->setText(text);
}


QString ResolveEditorDialog::content() const
{
    return m_edit->text();
}
