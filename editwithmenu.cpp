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

