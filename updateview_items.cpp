/*
 * Copyright (C) 1999-2002 Bernd Gehrmann <bernd@mail.berlios.de>
 * Copyright (c) 2003-2008 André Wöbbeking <Woebbeking@kde.org>
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


#include "updateview_items.h"

#include <cassert>

#include <qdir.h>

#include <QPixmap>
#include <QTextStream>

#include <QDebug>
#include <kcolorscheme.h>

#include "debug.h"
#include "cvsdir.h"
#include "entry.h"
#include "misc.h"
#include "updateview_visitors.h"


using Cervisia::Entry;
using Cervisia::EntryStatus;


// ------------------------------------------------------------------------------
// UpdateItem
// ------------------------------------------------------------------------------


QString UpdateItem::dirPath() const
{
    QString path;

    const UpdateItem* item = static_cast<UpdateItem*>(parent());
    while (item)
    {
        const UpdateItem* parentItem = static_cast<UpdateItem*>(item->parent());
        if (parentItem)
        {
            path.prepend(item->m_entry.m_name + QDir::separator());
        }

        item = parentItem;
    }

    return path;
}


QString UpdateItem::filePath() const
{
    // the filePath of the root item is '.'
    return parent() ? QString(dirPath() + m_entry.m_name) : QLatin1String(".");
}


// ------------------------------------------------------------------------------
// UpdateDirItem
// ------------------------------------------------------------------------------


UpdateDirItem::UpdateDirItem(UpdateDirItem* parent,
                             const Entry& entry)
    : UpdateItem(parent, entry, RTTI),
      m_opened(false)
{
    setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    setIcon(0, QIcon::fromTheme("folder"));
}


UpdateDirItem::UpdateDirItem(UpdateView* parent,
                             const Entry& entry)
    : UpdateItem(parent, entry, RTTI),
      m_opened(false)
{
    setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    setIcon(0, QIcon::fromTheme("folder"));
}


/**
 * Update the status of an item; if it doesn't exist yet, create new one
 */
void UpdateDirItem::updateChildItem(const QString& name,
                                    EntryStatus status,
                                    bool isdir)
{
    if (UpdateItem* item = findItem(name))
    {
        if (isFileItem(item))
        {
            UpdateFileItem* fileItem = static_cast<UpdateFileItem*>(item);
            fileItem->setStatus(status);
        }
        return;
    }

    // Not found, make new entry
    Entry entry;
    entry.m_name = name;
    if (isdir)
    {
        entry.m_type = Entry::Dir;
        createDirItem(entry)->maybeScanDir(true);
    }
    else
    {
        entry.m_type = Entry::File;
        createFileItem(entry)->setStatus(status);
    }
}


/**
 * Update the revision and tag of an item. Use status only to create
 * new items and for items which were NotInCVS.
 */
void UpdateDirItem::updateEntriesItem(const Entry& entry,
                                      bool isBinary)
{
    if (UpdateItem* item = findItem(entry.m_name))
    {
        if (isFileItem(item))
        {
            UpdateFileItem* fileItem = static_cast<UpdateFileItem*>(item);
            if (fileItem->entry().m_status == Cervisia::NotInCVS ||
                fileItem->entry().m_status == Cervisia::LocallyRemoved ||
                fileItem->entry().m_status == Cervisia::Unknown ||
                entry.m_status == Cervisia::LocallyAdded ||
                entry.m_status == Cervisia::LocallyRemoved ||
                entry.m_status == Cervisia::Conflict)
            {
                fileItem->setStatus(entry.m_status);
            }
            fileItem->setRevTag(entry.m_revision, entry.m_tag);
            fileItem->setDate(entry.m_dateTime);
            fileItem->setIcon(0, isBinary ? QIcon::fromTheme("application-octet-stream") 
	                                  : QIcon());
        }
        return;
    }

    // Not found, make new entry
    if (entry.m_type == Entry::Dir)
        createDirItem(entry)->maybeScanDir(true);
    else
        createFileItem(entry);
}


void UpdateDirItem::scanDirectory()
{
    const QString& path(filePath());
    if (!QFile::exists(path))
        return;

    const CvsDir dir(path);

    const QFileInfoList *files = dir.entryInfoList();
    if (files)
    {
        Q_FOREACH (QFileInfo info, *files)
        {
            Entry entry;
            entry.m_name = info.fileName();
            if (info.isDir())
            {
                entry.m_type = Entry::Dir;
                createDirItem(entry);
            }
            else
            {
                entry.m_type = Entry::File;
                entry.m_status = Cervisia::NotInCVS;
                createFileItem(entry);
            }
        }
    }
}


UpdateDirItem* UpdateDirItem::createDirItem(const Entry& entry)
{
    UpdateItem* item(insertItem(new UpdateDirItem(this, entry)));
    assert(isDirItem(item));
    return static_cast<UpdateDirItem*>(item);
}


UpdateFileItem* UpdateDirItem::createFileItem(const Entry& entry)
{
    UpdateItem* item(insertItem(new UpdateFileItem(this, entry)));
    assert(isFileItem(item));
    return static_cast<UpdateFileItem*>(item);
}


UpdateItem* UpdateDirItem::insertItem(UpdateItem* item)
{
    const TMapItemsByName::iterator it = m_itemsByName.find(item->entry().m_name);
    if (it != m_itemsByName.end())
    {
        // OK, an item with that name already exists. If the item type is the
        // same then keep the old one to preserve it's status information
        UpdateItem* existingItem = *it;
        if (existingItem->type() == item->type())
        {
            delete item;
            item = existingItem;
        }
        else
        {
            // avoid dangling pointers in the view
            updateView()->replaceItem(existingItem, item);

            delete existingItem;
            *it = item;
        }
    }
    else
    {
        m_itemsByName.insert(item->entry().m_name, item);
    }

    return item;
}


UpdateItem* UpdateDirItem::findItem(const QString& name) const
{
    const TMapItemsByName::const_iterator it = m_itemsByName.find(name);

    return (it != m_itemsByName.end()) ? *it : 0;
}


// Format of the CVS/Entries file:
//   /NAME/REVISION/[CONFLICT+]TIMESTAMP/OPTIONS/TAGDATE

void UpdateDirItem::syncWithEntries()
{
    const QString path(filePath() + QDir::separator());

    QFile f(path + "CVS/Entries");
    if( f.open(QIODevice::ReadOnly) )
    {
        QTextStream stream(&f);
        while( !stream.atEnd() )
        {
            QString line = stream.readLine();

            Cervisia::Entry entry;

            const bool isDir(line[0] == 'D');

            if( isDir )
                line.remove(0, 1);

            if( line[0] != '/' )
                continue;

            entry.m_type = isDir ? Entry::Dir : Entry::File;

            // since QString::section() always calls split internally, let's do it only once
            const QStringList sections = line.split(QLatin1Char('/'), QString::KeepEmptyParts, Qt::CaseSensitive);

            entry.m_name = sections[1];

            if (isDir)
            {
                updateEntriesItem(entry, false);
            }
            else
            {
                QString rev(sections[2]);
                const QString timestamp(sections[3]);
                const QString options(sections[4]);
                entry.m_tag = sections[5];

                const bool isBinary = options.contains("-kb");

                // file date in local time
                entry.m_dateTime = QFileInfo(path + entry.m_name).lastModified();

                // set milliseconds to 0 since CVS/Entries does only contain seconds resolution
                QTime t =  entry.m_dateTime.time();
                t.setHMS(t.hour(), t.minute(), t.second());
                entry.m_dateTime.setTime(t);

                if( rev == "0" )
                    entry.m_status = Cervisia::LocallyAdded;
                else if( rev.length() > 2 && rev[0] == '-' )
                {
                    entry.m_status = Cervisia::LocallyRemoved;
                    rev.remove(0, 1);
                }
                else if (timestamp.contains('+'))
                {
                    entry.m_status = Cervisia::Conflict;
                }
                else
                {
                    QDateTime date(QDateTime::fromString(timestamp)); // UTC Time
                    date.setTimeSpec(Qt::UTC);
                    const QDateTime fileDateUTC(entry.m_dateTime.toUTC());
                    if (date != fileDateUTC)
                        entry.m_status = Cervisia::LocallyModified;
                }

                entry.m_revision = rev;

                updateEntriesItem(entry, isBinary);
            }
        }
    }
}


/**
 * Test if files was removed from repository.
 */
void UpdateDirItem::syncWithDirectory()
{
    QDir dir(filePath());

    for (TMapItemsByName::iterator it(m_itemsByName.begin()),
                                   itEnd(m_itemsByName.end());
         it != itEnd; ++it)
    {
        // only files
        if (isFileItem(*it))
        {
            UpdateFileItem* fileItem = static_cast<UpdateFileItem*>(*it);

            // is file removed?
            if (!dir.exists(it.key()))
            {
                fileItem->setStatus(Cervisia::Removed);
                fileItem->setRevTag(QString(), QString());
            }
        }
    }
}


/**
 * Read in the content of the directory. If recursive is false, this
 * is shallow, otherwise all child directories are scanned recursively.
 */
void UpdateDirItem::maybeScanDir(bool recursive)
{
    if (!m_opened)
    {
        m_opened = true;
        scanDirectory();
        syncWithEntries();
    }

    if (recursive)
    {
        for (TMapItemsByName::iterator it(m_itemsByName.begin()),
                                       itEnd(m_itemsByName.end());
             it != itEnd; ++it)
        {
            if (isDirItem(*it))
                static_cast<UpdateDirItem*>(*it)->maybeScanDir(true);
        }
    }
}


void UpdateDirItem::accept(Visitor& visitor)
{
    visitor.preVisit(this);

    for (TMapItemsByName::iterator it(m_itemsByName.begin()),
                                   itEnd(m_itemsByName.end());
         it != itEnd; ++it)
    {
        (*it)->accept(visitor);
    }

    visitor.postVisit(this);
}


void UpdateDirItem::setOpen(bool open)
{
    if ( open )
    {
        const bool openFirstTime(!wasScanned());

        maybeScanDir(false);

        // if new items were created their visibility must be checked
        // (not while unfoldTree() as this could be slow and unfoldTree()
        // calls setFilter() itself)
        UpdateView* view = updateView();
        if (openFirstTime && !view->isUnfoldingTree())
            view->setFilter(view->filter());
    }

    setExpanded(open);
}


bool UpdateDirItem::operator<(const QTreeWidgetItem &other) const
{
    // UpdateDirItems are always lesser than UpdateFileItems
    if (isFileItem(&other))
        return true;

    const UpdateDirItem &item(static_cast<const UpdateDirItem &>(other));

    // for every column just compare the directory name
    return entry().m_name.localeAwareCompare(item.entry().m_name) < 0;
}


QVariant UpdateDirItem::data(int column, int role) const
{
    if ( (role == Qt::DisplayRole) && (column == Name) )
        return entry().m_name;

    return QTreeWidgetItem::data(column, role);
}


// ------------------------------------------------------------------------------
// UpdateFileItem
// ------------------------------------------------------------------------------


UpdateFileItem::UpdateFileItem(UpdateDirItem* parent, const Entry& entry)
    : UpdateItem(parent, entry, RTTI),
      m_undefined(false)
{
}


void UpdateFileItem::setStatus(EntryStatus status)
{
    if (status != m_entry.m_status)
    {
        m_entry.m_status = status;
        emitDataChanged();
    }
    m_undefined = false;
}


void UpdateFileItem::accept(Visitor& visitor)
{
    visitor.visit(this);
}


bool UpdateFileItem::applyFilter(UpdateView::Filter filter)
{
    bool visible(true);
    if (filter & UpdateView::OnlyDirectories)
        visible = false;

    bool unmodified = (entry().m_status == Cervisia::UpToDate) ||
                      (entry().m_status == Cervisia::Unknown);
    if ((filter & UpdateView::NoUpToDate) && unmodified)
        visible = false;
    if ((filter & UpdateView::NoRemoved) && (entry().m_status == Cervisia::Removed))
        visible = false;
    if ((filter & UpdateView::NoNotInCVS) && (entry().m_status == Cervisia::NotInCVS))
        visible = false;

    setHidden(!visible);

    return visible;
}


void UpdateFileItem::setRevTag(const QString& rev, const QString& tag)
{
    m_entry.m_revision = rev;

    if (tag.length() == 20 && tag[0] == 'D' && tag[5] == '.'
        && tag[8] == '.' && tag[11] == '.' && tag[14] == '.'
        && tag[17] == '.')
    {
        const QDate tagDate(tag.mid(1, 4).toInt(),
                            tag.mid(6, 2).toInt(),
                            tag.mid(9, 2).toInt());
        const QTime tagTime(tag.mid(12, 2).toInt(),
                            tag.mid(15, 2).toInt(),
                            tag.mid(18, 2).toInt());
        const QDateTime tagDateTimeUtc(tagDate, tagTime);

        if (tagDateTimeUtc.isValid())
        {
            // This is in UTC and must be converted to local time.
            //
            // A bit strange but I didn't find anything easier which is portable.
            // Compute the difference between UTC and local timezone for this
            // tag date.
            const unsigned int dateTimeInSeconds(tagDateTimeUtc.toTime_t());
            QDateTime dateTime;
            dateTime.setTime_t(dateTimeInSeconds);
            const int localUtcOffset(dateTime.secsTo(tagDateTimeUtc));

            const QDateTime tagDateTimeLocal(tagDateTimeUtc.addSecs(localUtcOffset));

            m_entry.m_tag = QLocale().toString(tagDateTimeLocal);
        }
        else
            m_entry.m_tag = tag;
    }
    else if (tag.length() > 1 && tag[0] == 'T')
        m_entry.m_tag = tag.mid(1);
    else
        m_entry.m_tag = tag;

    emitDataChanged();
}


void UpdateFileItem::setDate(const QDateTime& date)
{
    m_entry.m_dateTime = date;
}


void UpdateFileItem::markUpdated(bool laststage,
                                 bool success)
{
    EntryStatus newstatus = m_entry.m_status;

    if (laststage)
    {
        if (undefinedState() && m_entry.m_status != Cervisia::NotInCVS)
            newstatus = success? Cervisia::UpToDate : Cervisia::Unknown;
        setStatus(newstatus);
    }
    else
        setUndefinedState(true);
}


int UpdateFileItem::statusClass() const
{
    int iResult(0);
    switch (entry().m_status)
    {
    case Cervisia::Conflict:
        iResult = 0;
        break;
    case Cervisia::LocallyAdded:
        iResult = 1;
        break;
    case Cervisia::LocallyRemoved:
        iResult = 2;
        break;
    case Cervisia::LocallyModified:
        iResult = 3;
        break;
    case Cervisia::Updated:
    case Cervisia::NeedsUpdate:
    case Cervisia::Patched:
    case Cervisia::Removed:
    case Cervisia::NeedsPatch:
    case Cervisia::NeedsMerge:
        iResult = 4;
        break;
    case Cervisia::NotInCVS:
        iResult = 5;
        break;
    case Cervisia::UpToDate:
    case Cervisia::Unknown:
        iResult = 6;
        break;
    }

    return iResult;
}


bool UpdateFileItem::operator<(const QTreeWidgetItem &other) const
{
    // UpdateDirItems are always lesser than UpdateFileItems
    if (isDirItem(&other))
        return false;

    const UpdateFileItem &item = static_cast<const UpdateFileItem &>(other);

    switch ( treeWidget()->sortColumn() )
    {
    case Name:
        return entry().m_name.localeAwareCompare(item.entry().m_name) < 0;

    case Status:
        if (::compare(statusClass(), item.statusClass()) == 0)
            return entry().m_name.localeAwareCompare(item.entry().m_name) < 0;
        else
            return false;

    case Revision:
        return ::compareRevisions(entry().m_revision, item.entry().m_revision) < 0;

    case TagOrDate:
        return entry().m_tag.localeAwareCompare(item.entry().m_tag) < 0;

    case Timestamp:
        return ::compare(entry().m_dateTime, item.entry().m_dateTime) < 0;
    }

    return false;
}


QVariant UpdateFileItem::data(int column, int role) const
{
    if ( role == Qt::DisplayRole )
    {
        switch (column)
        {
        case Name:
            return entry().m_name;

        case Status:
            return toString(entry().m_status);

        case Revision:
            return entry().m_revision;

        case TagOrDate:
            return entry().m_tag;

        case Timestamp:
            if (entry().m_dateTime.isValid())
                return QLocale().toString(entry().m_dateTime);
            break;
        }
    }
    else if ( (role == Qt::ForegroundRole) || (role == Qt::FontRole) )
    {
        const UpdateView* view = static_cast<const UpdateView *>(treeWidget());

        QColor color;
        switch (m_entry.m_status)
        {
        case Cervisia::Conflict:
            color = view->conflictColor();
            break;
        case Cervisia::LocallyAdded:
        case Cervisia::LocallyModified:
        case Cervisia::LocallyRemoved:
            color = view->localChangeColor();
            break;
        case Cervisia::NeedsMerge:
        case Cervisia::NeedsPatch:
        case Cervisia::NeedsUpdate:
        case Cervisia::Patched:
        case Cervisia::Removed:
        case Cervisia::Updated:
            color = view->remoteChangeColor();
            break;
        case Cervisia::NotInCVS:
            color = view->notInCvsColor();
            break;
        case Cervisia::Unknown:
        case Cervisia::UpToDate:
            break;
        }

        // potentially slow - cache it
        static QColor schemeForeCol = KColorScheme(QPalette::Active, KColorScheme::View).foreground().color();

        if ( (role == Qt::FontRole) && color.isValid() && (color != schemeForeCol) )
        {
            QFont f = view->font();
            f.setBold(true);
            return f;
        }

        if ( role == Qt::ForegroundRole )
            return color;
    }

    return QTreeWidgetItem::data(column, role);
}


/**
 * Finds or creates the UpdateDirItem with path \a dirPath. If \a dirPath
 * is "." \a rootItem is returned.
 */
UpdateDirItem* findOrCreateDirItem(const QString& dirPath,
                                   UpdateDirItem* rootItem)
{
    assert(!dirPath.isEmpty());
    assert(rootItem);

    UpdateDirItem* dirItem(rootItem);

    if (dirPath != QLatin1String("."))
    {
        const QStringList& dirNames(dirPath.split('/'));
        const QStringList::const_iterator itDirNameEnd(dirNames.end());
        for (QStringList::const_iterator itDirName(dirNames.begin());
             itDirName != itDirNameEnd; ++itDirName)
        {
            const QString& dirName(*itDirName);

            UpdateItem* item = dirItem->findItem(dirName);
            if (isFileItem(item))
            {
                // this happens if you
                // - add a directory outside of Cervisia
                // - update status (a file item is created for the directory)
                // - add new directory in Cervisia
                // - update status
                qCDebug(log_cervisia) << "file changed to dir " << dirName;

                // just create a new dir item, createDirItem() will delete the
                // file item and update the m_itemsByName map
                item = 0;
            }

            if (!item)
            {
                qCDebug(log_cervisia) << "create dir item " << dirName;
                Entry entry;
                entry.m_name = dirName;
                entry.m_type = Entry::Dir;
                item = dirItem->createDirItem(entry);
            }

            assert(isDirItem(item));

            dirItem = static_cast<UpdateDirItem*>(item);
        }
    }

    return dirItem;
}
