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
#include <KConfigGroup>
#include <KHelpClient>
#include <KLocalizedString>

#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QDialogButtonBox>

AnnotateDialog::AnnotateDialog(KConfig& cfg, QWidget *parent)
    : QDialog(parent)
    , partConfig(cfg)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Close);

    QPushButton *user1Button = new QPushButton;
    user1Button->setText(i18n("Go to Line..."));
    user1Button->setAutoDefault(false);
    buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);

    QPushButton *user2Button = new QPushButton;
    user2Button->setText(i18n("Find Prev"));
    user2Button->setAutoDefault(false);
    buttonBox->addButton(user2Button, QDialogButtonBox::ActionRole);

    QPushButton *user3Button = new QPushButton;
    user3Button->setText(i18n("Find Next"));
    buttonBox->addButton(user3Button, QDialogButtonBox::ActionRole);

    buttonBox->button(QDialogButtonBox::Help)->setAutoDefault(false);

    connect(buttonBox, &QDialogButtonBox::rejected, this, &AnnotateDialog::reject);
    connect(buttonBox, &QDialogButtonBox::helpRequested, this, &AnnotateDialog::slotHelp);

    findEdit = new QLineEdit;
    findEdit->setClearButtonEnabled(true);
    findEdit->setPlaceholderText(i18n("Search"));

    annotate = new AnnotateView(this);

    mainLayout->addWidget(findEdit);
    mainLayout->addWidget(annotate);
    mainLayout->addWidget(buttonBox);

    connect(user3Button, SIGNAL(clicked()), this, SLOT(findNext()));
    connect(user2Button, SIGNAL(clicked()), this, SLOT(findPrev()));
    connect(user1Button, SIGNAL(clicked()), this, SLOT(gotoLine()));

    setAttribute(Qt::WA_DeleteOnClose, true);

    KConfigGroup cg(&partConfig, "AnnotateDialog");
    restoreGeometry(cg.readEntry<QByteArray>("geometry", QByteArray()));

    findEdit->setFocus();
}


AnnotateDialog::~AnnotateDialog()
{
    KConfigGroup cg(&partConfig, "AnnotateDialog");
    cg.writeEntry("geometry", saveGeometry());
}

void AnnotateDialog::slotHelp()
{
  KHelpClient::invokeHelp(QLatin1String("annotate"));
}


void AnnotateDialog::addLine(const Cervisia::LogInfo& logInfo,
                             const QString& content, bool odd)
{
    annotate->addLine(logInfo, content, odd);
}


void AnnotateDialog::findNext()
{
    if ( !findEdit->text().isEmpty() )
        annotate->findText(findEdit->text(), false);
}

void AnnotateDialog::findPrev()
{
    if ( !findEdit->text().isEmpty() )
        annotate->findText(findEdit->text(), true);
}

void AnnotateDialog::gotoLine()
{
    bool ok = false;
    int line = QInputDialog::getInt(this, i18n("Go to Line"), i18n("Go to line number:"),
                                    annotate->currentLine(), 1, annotate->lastLine(), 1, &ok);

    if ( ok )
        annotate->gotoLine(line);
}


// Local Variables:
// c-basic-offset: 4
// End:
