/*
 * Copyright (c) 2004 Andr�W�beking <Woebbeking@web.de>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
        result = i18n("Up to Date");
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
