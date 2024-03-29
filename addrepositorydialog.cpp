/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2002-2007 Christian Loose <christian.loose@kdemail.net>
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

#include "addrepositorydialog.h"

#include <QLabel>
#include <QVBoxLayout>
#include <qcheckbox.h>

#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

AddRepositoryDialog::AddRepositoryDialog(KConfig &cfg, const QString &repo, QWidget *parent)
    : QDialog(parent)
    , partConfig(cfg)
{
    setWindowTitle(i18n("Add Repository"));
    setModal(true);

    auto mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    auto repo_label = new QLabel(i18n("&Repository:"));
    mainLayout->addWidget(repo_label);

    repo_edit = new QLineEdit;
    mainLayout->addWidget(repo_edit);

    repo_edit->setFocus();
    repo_label->setBuddy(repo_edit);
    if (!repo.isNull()) {
        repo_edit->setText(repo);
        repo_edit->setEnabled(false);
    }
    mainLayout->addWidget(repo_edit);

    auto rsh_label = new QLabel(i18n("Use remote &shell (only for :ext: repositories):"));
    mainLayout->addWidget(rsh_label);
    mainLayout->addWidget(rsh_label);

    rsh_edit = new QLineEdit;
    mainLayout->addWidget(rsh_edit);
    rsh_label->setBuddy(rsh_edit);
    mainLayout->addWidget(rsh_edit);

    auto server_label = new QLabel(i18n("Invoke this program on the server side:"));
    mainLayout->addWidget(server_label);

    server_edit = new QLineEdit;
    mainLayout->addWidget(server_edit);
    server_label->setBuddy(server_edit);
    mainLayout->addWidget(server_edit);

    auto compressionBox = new QHBoxLayout;
    mainLayout->addLayout(compressionBox);
    m_useDifferentCompression = new QCheckBox(i18n("Use different &compression level:"));

    m_compressionLevel = new QSpinBox();
    m_compressionLevel->setRange(0, 9);

    compressionBox->addWidget(m_useDifferentCompression);
    compressionBox->addWidget(m_compressionLevel);

    m_retrieveCvsignoreFile = new QCheckBox(i18n("Download cvsignore file from server"));
    mainLayout->addWidget(m_retrieveCvsignoreFile);

    mainLayout->addWidget(buttonBox);
    okButton->setDefault(true);

    connect(repo_edit, SIGNAL(textChanged(QString)), this, SLOT(repoChanged()));
    connect(m_useDifferentCompression, SIGNAL(toggled(bool)), this, SLOT(compressionToggled(bool)));

    repoChanged();

    KConfigGroup cg(&partConfig, "AddRepositoryDialog");
    restoreGeometry(cg.readEntry<QByteArray>("geometry", QByteArray()));
}

AddRepositoryDialog::~AddRepositoryDialog()
{
    KConfigGroup cg(&partConfig, "AddRepositoryDialog");
    cg.writeEntry("geometry", saveGeometry());
}

void AddRepositoryDialog::setRsh(const QString &rsh)
{
    rsh_edit->setText(rsh);
}

void AddRepositoryDialog::setServer(const QString &server)
{
    server_edit->setText(server);
}

void AddRepositoryDialog::setCompression(int compression)
{
    if (compression < 0) {
        // TODO: use KConfigXT to retrieve default compression level
        m_compressionLevel->setValue(0);
        m_useDifferentCompression->setChecked(false);
    } else {
        m_useDifferentCompression->setChecked(true);
        m_compressionLevel->setValue(compression);
    }

    compressionToggled(m_useDifferentCompression->isChecked());
}

void AddRepositoryDialog::setRetrieveCvsignoreFile(bool enabled)
{
    m_retrieveCvsignoreFile->setChecked(enabled);
}

QString AddRepositoryDialog::repository() const
{
    return repo_edit->text();
}

QString AddRepositoryDialog::rsh() const
{
    return rsh_edit->text();
}

QString AddRepositoryDialog::server() const
{
    return server_edit->text();
}

int AddRepositoryDialog::compression() const
{
    if (m_useDifferentCompression->isChecked())
        return m_compressionLevel->value();
    else
        return -1;
}

bool AddRepositoryDialog::retrieveCvsignoreFile() const
{
    return m_retrieveCvsignoreFile->isChecked();
}

void AddRepositoryDialog::setRepository(const QString &repo)
{
    setWindowTitle(i18n("Repository Settings"));

    repo_edit->setText(repo);
    repo_edit->setEnabled(false);
}

void AddRepositoryDialog::repoChanged()
{
    QString repo = repository();
    rsh_edit->setEnabled((!repo.startsWith(QLatin1String(":pserver:"))) && repo.contains(":"));
    m_useDifferentCompression->setEnabled(repo.contains(":"));
    if (!repo.contains(":"))
        m_compressionLevel->setEnabled(false);
    else
        compressionToggled(m_useDifferentCompression->isChecked());
}

void AddRepositoryDialog::compressionToggled(bool checked)
{
    m_compressionLevel->setEnabled(checked);
}

// Local Variables:
// c-basic-offset: 4
// End:
