/*
 * Copyright (c) 2004 Andr� W�bbeking <Woebbeking@web.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "entry_status.h"

#include <qstring.h>

#include <klocale.h>


namespace Cervisia
{


QString toString(EntryStatus entryStatus)
{
    QString result;
    switch (entryStatus)
    {
    case LocallyModified:
        result = i18n("Locally Modified");
        break;
    case LocallyAdded:
        result = i18n("Locally Added");
        break;
    case LocallyRemoved:
        result = i18n("Locally Removed");
        break;
    case NeedsUpdate:
        result = i18n("Needs Update");
        break;
    case NeedsPatch:
        result = i18n("Needs Patch");
        break;
    case NeedsMerge:
        result = i18n("Needs Merge");
        break;
    case UpToDate:
        result = i18n("Up to date");
        break;
    case Conflict:
        result = i18n("Conflict");
        break;
    case Updated:
        result = i18n("Updated");
        break;
    case Patched:
        result = i18n("Patched");
        break;
    case Removed:
        result = i18n("Removed");
        break;
    case NotInCVS:
        result = i18n("Not in CVS");
        break;
    case Unknown:
        result = i18n("Unknown");
        break;
    }

    return result;
}


} // namespace Cervisia
