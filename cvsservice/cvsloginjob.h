/*
 * Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#ifndef CVSLOGINJOB_H
#define CVSLOGINJOB_H

#include <qstring.h>
#include <qstringlist.h>
#include <dcopobject.h>

#include <kdesu/process.h>


class CvsLoginJob : public DCOPObject
{
    K_DCOP

public:
    explicit CvsLoginJob(unsigned jobNum);
    virtual ~CvsLoginJob();

    void setServer(const QString& server);

    void setCvsClient(const QCString& cvsClient);
    void setRepository(const QCString& repository);

k_dcop:
    bool execute();
    QStringList output();

private:
    PtyProcess*    m_Proc;
    QString        m_Server;
    QString        m_Rsh;
    QCString       m_CvsClient;
    QCStringList   m_Arguments;
    QStringList    m_output;
};


#endif
