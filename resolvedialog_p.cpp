/*
 *  Copyright (c) 2004 Christian Loose <christian.loose@kdemail.net>
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

#include "resolvedialog_p.h"
#include "cervisiasettings.h"

#include <kconfig.h>
#include <QPlainTextEdit>
#include <kconfiggroup.h>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace Cervisia;


ResolveEditorDialog::ResolveEditorDialog(KConfig& cfg, QWidget *parent)
    : QDialog(parent)
    , m_partConfig(cfg)
{
    setModal(true);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    //PORTING SCRIPT: WARNING mainLayout->addWidget(buttonBox) must be last item in layout. Please move it.
    mainLayout->addWidget(buttonBox);
    okButton->setDefault(true);

    m_edit = new QPlainTextEdit(this);
    m_edit->setFont(CervisiaSettings::diffFont());
    m_edit->setFocus();

    mainLayout->addWidget(m_edit);

    QFontMetrics const fm(fontMetrics());
    setMinimumSize(fm.width('0') * 120,
                   fm.lineSpacing() * 40);

    KConfigGroup cg(&m_partConfig, "ResolveEditorDialog");
    restoreGeometry(cg.readEntry<QByteArray>("geometry", QByteArray()));
}


ResolveEditorDialog::~ResolveEditorDialog()
{
    KConfigGroup cg(&m_partConfig, "ResolveEditorDialog");
    cg.writeEntry("geometry", saveGeometry());
}


void ResolveEditorDialog::setContent(const QString& text)
{
    m_edit->setPlainText(text);
}


QString ResolveEditorDialog::content() const
{
    return m_edit->toPlainText();
}
