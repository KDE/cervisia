/*
 * Copyright (c) 2004-2005 André Wöbbeking <Woebbeking@web.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef CERVISIA_ENTRY_STATUS_H
#define CERVISIA_ENTRY_STATUS_H


class QString;


namespace Cervisia
{


/**
 * All stati a an entry could have.
 */
enum EntryStatus
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

/**
 * The entry status as translated string.
 *
 * @param entryStatus The entry status to translate.
 *
 * @return The translated string.
 */
QString toString(EntryStatus entryStatus);


} // namespace Cervisia


#endif // CERVISIA_ENTRY_STATUS_H
