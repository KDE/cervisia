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

#ifndef EDITWITHMENU_H
#define EDITWITHMENU_H

#include <qobject.h>
#include <ktrader.h>
#include <kurl.h>

class QPopupMenu;


namespace Cervisia
{


class EditWithMenu : public QObject
{
    Q_OBJECT

public:
    EditWithMenu(const KURL& url, QWidget* parent);
    QPopupMenu* menu();

private slots:
    void itemActivated(int);

private:
    KTrader::OfferList m_offers;
    QPopupMenu*        m_menu;
    KURL               m_url;
};


}


#endif

