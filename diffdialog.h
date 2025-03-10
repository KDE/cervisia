/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2003-2007 Christian Loose <christian.loose@kdemail.net>
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

#ifndef DIFFDIALOG_H
#define DIFFDIALOG_H

#include <QList>

#include <QDialog>

class QLabel;
class QCheckBox;
class KComboBox;
class KConfig;
class DiffItem;
class DiffView;
class OrgKdeCervisia5CvsserviceCvsserviceInterface;

class DiffDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DiffDialog(KConfig &config, QWidget *parent = nullptr, bool modal = false);
    ~DiffDialog() override;

    bool parseCvsDiff(OrgKdeCervisia5CvsserviceCvsserviceInterface *service, const QString &fileName, const QString &revA, const QString &revB);

protected:
    void keyPressEvent(QKeyEvent *e) override;

private Q_SLOTS:
    void toggleSynchronize(bool b);
    void comboActivated(int index);
    void backClicked();
    void forwClicked();
    void saveAsClicked();
    void slotHelp();

private:
    void newDiffHunk(int &linenoA, int &linenoB, const QStringList &linesA, const QStringList &linesB);
    void callExternalDiff(const QString &extdiff,
                          OrgKdeCervisia5CvsserviceCvsserviceInterface *service,
                          const QString &fileName,
                          const QString &revA,
                          const QString &revB);
    void updateNofN();
    void updateHighlight(int newitem);

    QLabel *revlabel1, *revlabel2, *nofnlabel;
    QCheckBox *syncbox;
    KComboBox *itemscombo;
    QPushButton *backbutton, *forwbutton;
    DiffView *diff1, *diff2;

    QList<DiffItem *> items;
    int markeditem;
    KConfig &partConfig;
    QStringList m_diffOutput;
};

#endif

// Local Variables:
// c-basic-offset: 4
// End:
