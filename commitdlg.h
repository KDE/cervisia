/* 
 *  Copyright (C) 1999-2001 Bernd Gehrmann
 *                          bernd@physik.hu-berlin.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef _COMMITDLG_H_
#define _COMMITDLG_H_

#include <qdialog.h>
#include <qlistbox.h>
#include <qmultilinedit.h>

class QComboBox;

class CommitDialog : public QDialog
{
    Q_OBJECT

public:
    enum ActionType { Commit, Add, AddBinary, Remove };
    
    CommitDialog( ActionType action, QWidget *parent=0, const char *name=0 );
    
    void setFileList(const QStringList &list)
        { listbox->insertStringList(list); }
    void setLogMessage(const QString &msg)
        { edit->setText(msg); }
    QString logMessage() const
        { return edit->text(); }
    void setLogHistory(const QString &sbox, const QString &repo, const QStringList &list);

    static void loadOptions(KConfig *config);
    static void saveOptions(KConfig *config);

protected:
    virtual void done(int r);

private slots:
    void comboActivated(int);
    void fileSelected(int);

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
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
