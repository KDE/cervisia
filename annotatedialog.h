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

#ifndef ANNOTATEDIALOG_H
#define ANNOTATEDIALOG_H

#include <QDialog>

class AnnotateView;
class KConfig;
class QLineEdit;

namespace Cervisia
{
struct LogInfo;
}

class AnnotateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AnnotateDialog(KConfig &cfg, QWidget *parent = nullptr);

    ~AnnotateDialog() override;

    void addLine(const Cervisia::LogInfo &logInfo, const QString &content, bool odd);

private Q_SLOTS:
    void slotHelp();
    void findNext();
    void findPrev();
    void gotoLine();

private:
    QLineEdit *findEdit;
    AnnotateView *annotate;
    KConfig &partConfig;
};

#endif // ANNOTATEDIALOG_H

// Local Variables:
// c-basic-offset: 4
// End:
