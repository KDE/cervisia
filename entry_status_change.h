/*
 * Copyright (c) 2004 André Wöbbeking <Woebbeking@web.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef CERVISIA_ENTRY_STATUS_CHANGE_H
#define CERVISIA_ENTRY_STATUS_CHANGE_H


#include <qstring.h>

#include "entry_status.h"


namespace Cervisia
{


/**
 * Dumb data struct to store a status change of an entry (i.e. for jobs like
 * status, update, ...).
 */
struct EntryStatusChange
{
    /**
     * The name of the changed entry (including the path inside the repository / working copy).
     */
    QString m_name;

    /**
     * The new status of the entry.
     */
    EntryStatus m_status;
};


} // namespace Cervisia


#endif // CERVISIA_ENTRY_STATUS_CHANGE_H
