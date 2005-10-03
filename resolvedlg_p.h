/* 
 *  Copyright (c) 2004 Christian Loose <christian.loose@kdemail.net>
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
