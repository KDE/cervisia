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

#include <qstringlist.h>
#include <kdialogbase.h>


class QComboBox;
class QListBox;
class QMultiLineEdit;
class KConfig;
class CvsService_stub;


class CommitDialog : public KDialogBase
{
    Q_OBJECT

public:   
    CommitDialog( KConfig& cfg, CvsService_stub* service, QWidget *parent=0, 
                  const char *name=0 );

    virtual ~CommitDialog();

    void setFileList(const QStringList &list);
    void setLogMessage(const QString &msg);
    QString logMessage() const;
    void setLogHistory(const QStringList &list);

private slots:
    void comboActivated(int);
    void fileSelected(int);
    void fileHighlighted(int);
    void diffClicked();

private:
    void showDiffDialog(const QString& fileName);

    QListBox *listbox;
    QMultiLineEdit *edit;
    QComboBox *combo;
    QStringList commits;
    int current_index;
    QString current_text;
    int highlightedFile;
    
    KConfig&            partConfig;
    CvsService_stub*    cvsService;     // for diff dialog
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
