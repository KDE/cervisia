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


#ifndef RESOLVEDLG_H
#define RESOLVEDLG_H


#include <kdialogbase.h>

#include <qptrlist.h>


class DiffView;

class QLabel;
class QMultiLineEdit;
class QTextCodec;
class KConfig;
class ResolveItem;


class ResolveDialog : public KDialogBase
{
    Q_OBJECT

public:
    enum ChooseType { ChA, ChB, ChAB, ChBA, ChEdit };

    explicit ResolveDialog( KConfig& cfg, QWidget *parent=0, const char *name=0 );

    virtual ~ResolveDialog();

    bool parseFile(const QString &name);

protected:
    virtual void keyPressEvent(QKeyEvent *e);

private slots:
    void backClicked();
    void forwClicked();
    void aClicked();
    void bClicked();
    void abClicked();
    void baClicked();
    void editClicked();
    void saveClicked();
    void saveAsClicked();
    
private:
    void updateNofN();
    void updateHighlight(int newitem);
    void choose(ChooseType ch);
    void chooseEdit();
    void saveFile(const QString &name);
    
    QLabel *nofnlabel;
    QPushButton *backbutton, *forwbutton;
    QPushButton *abutton, *bbutton, *abbutton, *babutton, *editbutton;
    DiffView *diff1, *diff2, *merge;

    QPtrList<ResolveItem> items;
    QString fname;
    QTextCodec *fcodec;
    int markeditem;
    KConfig& partConfig;
};


class ResolveEditorDialog : public KDialogBase
{
public:

    explicit ResolveEditorDialog( KConfig& cfg, QWidget *parent=0, const char *name=0 );

    virtual ~ResolveEditorDialog();

    void setContent(const QStringList &l);
    QStringList content() const;

private:   
    QMultiLineEdit *edit;
    KConfig& partConfig;
};  

#endif


// Local Variables:
// c-basic-offset: 4
// End:
