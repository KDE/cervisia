/*
 * Copyright (c) 2003-2004 André Wöbbeking <Woebbeking@web.de>
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


namespace Cervisia
{


/**
 * Dumb data struct to store an entry controlled by a version control system
 * plus some convenience methods.
 */
struct Entry
{
    enum Status
    {
        LocallyModified,
        LocallyAdded,
        LocallyRemoved,
        NeedsUpdate,
        NeedsPatch,
        NeedsMerge,
        UpToDate,
        Conflict,
        Updated,
        Patched,
        Removed,
        NotInCVS,
        Unknown
    };

    enum Type
    {
        Dir,
        File
    };

    /**
     * Sets status to \a Unknown and type to \a File.
     */
    Entry();

    /**
     * The status as translated string.
     */
    QString statusToString() const;

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
    Status m_status;

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
