/*
 * Copyright (c) 2003-2005 André Wöbbeking <Woebbeking@web.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef CERVISIA_ENTRY_H
#define CERVISIA_ENTRY_H


#include <qdatetime.h>
#include <qstring.h>

#include "entry_status.h"


namespace Cervisia
{


/**
 * Dumb data struct to store an entry controlled by a version control system.
 */
struct Entry
{
    enum Type
    {
        Dir,
        File
    };

    /**
     * Sets status to \a EntryStatus::Unknown and type to \a File.
     */
    Entry();

    /**
     * The name of this entry (without path).
     */
    QString m_name;

    /**
     * The type of this entry.
     */
    Type m_type;

    /**
     * The status of this entry.
     */
    EntryStatus m_status;

    /**
     * The revision of this entry.
     */
    QString m_revision;

    /**
     * The modification date/time of this entry (in user's local time).
     */
    QDateTime m_dateTime;

    /**
     * The tag/branch of this entry.
     */
    QString m_tag;
};


} // namespace Cervisia


#endif // CERVISIA_ENTRY_H
