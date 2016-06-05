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

#include "editwithmenu.h"
#include "debug.h"
using namespace Cervisia;

#include <QMenu>

#include <QDebug>

#include <KMimeTypeTrader>
#include <KLocalizedString>
#include <krun.h>
#include <QMimeDatabase>
#include <QMimeType>


EditWithMenu::EditWithMenu(const QUrl& url, QWidget* parent)
    : QObject(parent)
    , m_menu(0)
    , m_url(url)
{
    QMimeDatabase db;
    QMimeType type = db.mimeTypeForFile(url.path(), QMimeDatabase::MatchExtension);
    if ( !type.isValid() )
    {
        qCDebug(log_cervisia) << "Couldn't find mime type!";
        return;
    }

    m_offers = KMimeTypeTrader::self()->query(type.name());

    if( !m_offers.isEmpty() )
    {
        m_menu = new QMenu(i18n("Edit With"));

        KService::List::ConstIterator it = m_offers.constBegin();
        for( int i = 0 ; it != m_offers.constEnd(); ++it, ++i )
        {
            QAction* pAction = m_menu->addAction(QIcon::fromTheme((*it)->icon()), (*it)->name());
            pAction->setData(i);
        }

        connect(m_menu, SIGNAL(triggered(QAction*)),
                this, SLOT(actionTriggered(QAction*)));
    }
}


QMenu* EditWithMenu::menu()
{
    return m_menu;
}


void EditWithMenu::actionTriggered(QAction* action)
{
    const KService::Ptr service = m_offers[action->data().toInt()];

    QList<QUrl> list;
    list.append(m_url);

    KRun::runService(*service, list, 0);
}



