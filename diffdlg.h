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


#ifndef DIFFDLG_H
#define DIFFDLG_H

#include <qdialog.h>
#include <qptrlist.h>

class QLabel;
class QCheckBox;
class QComboBox;
class KConfig;
class DiffItem;
class DiffView;


class DiffDialog : public QDialog
{
    Q_OBJECT

public:
    DiffDialog( QWidget *parent=0, const char *name=0, bool modal=false );

    bool parseCvsDiff(const QString &sandbox, const QString &repository,
                      const QString &filename, const QString &revA, const QString &revB);

    static void loadOptions(KConfig *config);
    static void saveOptions(KConfig *config);

protected slots:
    virtual void done(int r);

protected:
    virtual void keyPressEvent(QKeyEvent *e);

private slots:
    void toggleSynchronize(bool b);
    void comboActivated(int index);
    void backClicked();
    void forwClicked();
    void helpClicked();
    
private:
    struct Options {
        QSize size;
        bool sync;
    };
    static Options *options;

    void updateNofN();
    void updateHighlight(int newitem);

    QLabel *revlabel1, *revlabel2, *nofnlabel;
    QCheckBox *syncbox;
    QComboBox *itemscombo;
    QPushButton *backbutton, *forwbutton;
    DiffView *diff1, *diff2;

    QPtrList<DiffItem> items;
    int markeditem;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
