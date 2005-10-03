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
using namespace Cervisia;

#include <qpopupmenu.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <krun.h>
#include <kurl.h>


EditWithMenu::EditWithMenu(const KURL& url, QWidget* parent)
    : QObject(parent)
    , m_menu(0)
    , m_url(url)
{
    KMimeType::Ptr type = KMimeType::findByURL(url, 0, true);
    if( type->name() == KMimeType::defaultMimeType() )
    {
        kdDebug() << "Couldn't find mime type!" << endl;
        return;
    }

    m_offers = KTrader::self()->query(type->name(), "Type == 'Application'");

    if( !m_offers.isEmpty() )
    {
        m_menu = new QPopupMenu();

        KTrader::OfferList::ConstIterator it = m_offers.begin();
        for( int i = 0 ; it != m_offers.end(); ++it, ++i )
        {
            int id = m_menu->insertItem(SmallIcon((*it)->icon()),
                                        (*it)->name(),
                                        this, SLOT(itemActivated(int)));
            m_menu->setItemParameter(id, i);
        }
    }
}


QPopupMenu* EditWithMenu::menu()
{
    return m_menu;
}


void EditWithMenu::itemActivated(int item)
{
    KService::Ptr service = m_offers[item];

    KURL::List list;
    list.append(m_url);

    KRun::run(*service, list);
}


#include "editwithmenu.moc"

