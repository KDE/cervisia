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


#ifndef COMMITDLG_H
#define COMMITDLG_H


#include <kdialogbase.h>

#include <qstringlist.h>


class QComboBox;
class QListBox;
class QMultiLineEdit;
class KConfig;


class CommitDialog : public KDialogBase
{
    Q_OBJECT

public:   
    explicit CommitDialog( QWidget *parent=0, const char *name=0 );

    virtual ~CommitDialog();

    void setFileList(const QStringList &list);
    void setLogMessage(const QString &msg);
    QString logMessage() const;
    void setLogHistory(const QString &sbox, const QString &repo, const QStringList &list);

    static void loadOptions(KConfig *config);
    static void saveOptions(KConfig *config);

private slots:
    void comboActivated(int);
    void fileSelected(int);
    void fileHighlighted(int);
    void diffClicked();

private:
    struct Options {
        QSize size;
    };
    static Options *options;

    QListBox *listbox;
    QMultiLineEdit *edit;
    QComboBox *combo;
    QStringList commits;
    int current_index;
    QString current_text;
    QString sandbox;
    QString repository;
    int highlightedFile;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
