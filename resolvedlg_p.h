/* 
 *  Copyright (c) 2004 Christian Loose <christian.loose@kdemail.net>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef CERVISIA_RESOLVEEDITORDIALOG_H
#define CERVISIA_RESOLVEEDITORDIALOG_H

#include <kdialogbase.h>

class KTextEdit;
class QStringList;
class KConfig;


namespace Cervisia
{


class ResolveEditorDialog : public KDialogBase
{
public:
    explicit ResolveEditorDialog(KConfig& cfg, QWidget* parent=0, const char* name=0);
    virtual ~ResolveEditorDialog();

    void setContent(const QString& text);
    QString content() const;

private:   
    KTextEdit* m_edit;
    KConfig&   m_partConfig;
};  


}


#endif
