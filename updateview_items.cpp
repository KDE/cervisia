/*
 * Copyright (C) 1999-2002 Bernd Gehrmann <bernd@mail.berlios.de>
 * Copyright (c) 2003 André Wöbbeking <Woebbeking@web.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "updateview_items.h"

#include <cassert>
#include <set>

#include <qdir.h>
#include <qpainter.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>

#include "cvsdir.h"
#include "entry.h"
#include "misc.h"
#include "updateview_visitors.h"


using Cervisia::Entry;


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
    return parent() ? dirPath() + m_entry.m_name : QChar('.');
}


// ------------------------------------------------------------------------------
// UpdateDirItem
// ------------------------------------------------------------------------------


UpdateDirItem::UpdateDirItem(UpdateDirItem* parent,
                             const Entry& entry)
    : UpdateItem(parent, entry),
      m_opened(false)
{
    setExpandable(true);
    setPixmap(0, SmallIcon("folder"));
}


UpdateDirItem::UpdateDirItem(UpdateView* parent,
                             const Entry& entry)
    : UpdateItem(parent, entry),
      m_opened(false)
{
    setExpandable(true);
    setPixmap(0, SmallIcon("folder"));
}


/**
 * Update the status of an item; if it doesn't exist yet, create new one
 */
void UpdateDirItem::updateChildItem(const QString& name,
                                    Entry::Status status,
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
            if (fileItem->entry().m_status == Entry::NotInCVS ||
                fileItem->entry().m_status == Entry::LocallyRemoved ||
                entry.m_status == Entry::LocallyAdded ||
                entry.m_status == Entry::LocallyRemoved ||
                entry.m_status == Entry::Conflict)
            {
                fileItem->setStatus(entry.m_status);
            }
            fileItem->setRevTag(entry.m_revision, entry.m_tag);
            fileItem->setDate(entry.m_dateTime);
            fileItem->setPixmap(0, isBinary ? SmallIcon("binary") : 0);
        }
        return;
    }

    // Not found, make new entry
    if (entry.m_type == Entry::Dir)
        createDirItem(entry)->maybeScanDir(true);
    else
    {
        createFileItem(entry);
    }
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
        QFileInfoListIterator it(*files);
        for (; it.current(); ++it)
        {
            Entry entry;
            entry.m_name = it.current()->fileName();
            if (it.current()->isDir())
            {
                entry.m_type = Entry::Dir;
                createDirItem(entry);
            }
            else
            {
                entry.m_type = Entry::File;
                entry.m_status = Entry::NotInCVS;
                createFileItem(entry);
            }
        }
    }
}


UpdateDirItem* UpdateDirItem::createDirItem(const Entry& entry)
{
    UpdateDirItem* dirItem = new UpdateDirItem(this, entry);

    mapItemsByName.insert(std::make_pair(entry.m_name, dirItem));

    return dirItem;
}


UpdateFileItem* UpdateDirItem::createFileItem(const Entry& entry)
{
    UpdateFileItem* fileItem = new UpdateFileItem(this, entry);

    mapItemsByName.insert(std::make_pair(entry.m_name, fileItem));

    return fileItem;
}


UpdateItem* UpdateDirItem::findItem(const QString& name) const
{
    const TMapItemsByName::const_iterator it = mapItemsByName.find(name);

    return (it != mapItemsByName.end()) ? it->second : 0;
}


// Format of the CVS/Entries file:
//   /NAME/REVISION/[CONFLICT+]TIMESTAMP/OPTIONS/TAGDATE

void UpdateDirItem::syncWithEntries()
{
    const QString path(filePath() + QDir::separator());

    QFile f(path + "CVS/Entries");
    if( f.open(IO_ReadOnly) )
    {
        QTextStream stream(&f);
        while( !stream.eof() )
        {
            QString line = stream.readLine();

            Cervisia::Entry entry;

            const bool isDir(line[0] == 'D');

            if( isDir )
                line.remove(0, 1);

            if( line[0] != '/' )
                continue;

            entry.m_type = isDir ? Entry::Dir : Cervisia::Entry::File;
            entry.m_name = line.section('/', 1, 1);

            if (isDir)
            {
                updateEntriesItem(entry, false);
            }
            else
            {
                QString rev(line.section('/', 2, 2));
                const QString timestamp(line.section('/', 3, 3));
                const QString options(line.section('/', 4, 4));
                entry.m_tag = line.section('/', 5, 5);

                const bool isBinary(options.find("-kb") >= 0);

                // file date in local time
                entry.m_dateTime = QFileInfo(path + entry.m_name).lastModified();

                if( rev == "0" )
                    entry.m_status = Entry::LocallyAdded;
                else if( rev.length() > 2 && rev[0] == '-' )
                {
                    entry.m_status = Entry::LocallyRemoved;
                    rev.remove(0, 1);
                }
                else if (timestamp.find('+') >= 0)
                {
                    entry.m_status = Entry::Conflict;
                }
                else
                {
                    const QDateTime date(QDateTime::fromString(timestamp)); // UTC Time
                    QDateTime fileDateUTC;
                    fileDateUTC.setTime_t(entry.m_dateTime.toTime_t(), Qt::UTC);
                    if (date != fileDateUTC)
                        entry.m_status = Entry::LocallyModified;
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
    // Do not use CvsDir here, because CVS/Entries may
    // contain files which are in .cvsignore (stupid
    // idea, but that's possible...)
    const QDir dir( filePath(), QString::null, QDir::Name,
                    QDir::Files|QDir::Hidden|QDir::NoSymLinks );
    const QStringList& files(dir.entryList());

    // copy all files in a set for better performance when we test for existence
    std::set<QString> setFiles;
    QStringList::ConstIterator const itFileEnd = files.end();
    for (QStringList::ConstIterator itFile = files.begin();
         itFile != itFileEnd; ++itFile)
        setFiles.insert(setFiles.end(), *itFile);

    for (QListViewItem *item = firstChild(); item; item = item->nextSibling())
    {
        // only files
        if (isFileItem(item))
        {
            UpdateFileItem* fileItem = static_cast<UpdateFileItem*>(item);

            // is file removed?
            if (setFiles.find(fileItem->entry().m_name) == setFiles.end())
            {
                fileItem->setStatus(Entry::Removed);
                fileItem->setRevTag(QString::null, QString::null);
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

        // sort the created items
        sort();
    }

    if (recursive)
    {
        for ( QListViewItem *item = firstChild(); item;
              item = item->nextSibling() )
            if (isDirItem(item))
                static_cast<UpdateDirItem*>(item)->maybeScanDir(true);
    }
}


void UpdateDirItem::accept(Visitor& visitor)
{
    visitor.visit(this);

    for (QListViewItem* item = firstChild(); item; item = item->nextSibling())
    {
        static_cast<UpdateItem*>(item)->accept(visitor);
    }
}


void UpdateDirItem::applyFilter(UpdateView::Filter filter)
{
    // as QListViewItem::setVisible() is recursive we have to make
    // this UpdateDirItem visible first and later we can make it invisible
    setVisible(true);

    bool hasVisibleChild(false);
    for (QListViewItem* childItem(firstChild());
         childItem != 0; childItem = childItem->nextSibling())
    {
        static_cast<UpdateItem*>(childItem)->applyFilter(filter);

        hasVisibleChild = hasVisibleChild || childItem->isVisible();
    }

    // a UpdateDirItem is visible if
    // - it has visible childs
    // - it is not opened
    // - empty directories are not hidden
    // - it has no parent (top level item)
    const bool visible(hasVisibleChild
                       || m_opened == false
                       || (filter & UpdateView::NoEmptyDirectories) == 0
                       || parent() == 0);

    // only set invisible as QListViewItem::setVisible() is recursive
    // and so maybe overrides the state applied by the filter
    if (visible == false)
        setVisible(false);
}


void UpdateDirItem::setOpen(bool open)
{
    if ( open )
        maybeScanDir(false);

    QListViewItem::setOpen(open);
}


int UpdateDirItem::compare(QListViewItem* i,
                           int /*column*/,
                           bool bAscending) const
{
    // UpdateDirItems are always lesser than UpdateFileItems
    if (isFileItem(i))
        return bAscending ? -1 : 1;

    const UpdateDirItem* item(static_cast<UpdateDirItem*>(i));

    // for every column just compare the directory name
    return entry().m_name.localeAwareCompare(item->entry().m_name);
}


QString UpdateDirItem::text(int column) const
{
    QString result;
    if (column == Name)
        result = entry().m_name;

    return result;
}


// ------------------------------------------------------------------------------
// UpdateFileItem
// ------------------------------------------------------------------------------


UpdateFileItem::UpdateFileItem(UpdateDirItem* parent, const Entry& entry)
    : UpdateItem(parent, entry),
      m_undefined(false)
{
}


void UpdateFileItem::setStatus(Entry::Status status)
{
    if (status != m_entry.m_status)
    {
        m_entry.m_status = status;
        applyFilter(updateView()->filter());
        if (isVisible())
            repaint();
    }
    m_undefined = false;
}


void UpdateFileItem::accept(Visitor& visitor)
{
    visitor.visit(this);
}


void UpdateFileItem::applyFilter(UpdateView::Filter filter)
{
    bool hide(false);
    if (filter & UpdateView::OnlyDirectories)
        hide = true;
    if ((filter & UpdateView::NoUpToDate) && (entry().m_status == Entry::UpToDate))
        hide = true;
    if ((filter & UpdateView::NoRemoved) && (entry().m_status == Entry::Removed))
        hide = true;
    if ((filter & UpdateView::NoNotInCVS) && (entry().m_status == Entry::NotInCVS))
        hide = true;
    setVisible(!hide);
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
            dateTime.setTime_t(dateTimeInSeconds, Qt::UTC);
            const int localUtcOffset(dateTime.secsTo(tagDateTimeUtc));

            const QDateTime tagDateTimeLocal(tagDateTimeUtc.addSecs(localUtcOffset));

            m_entry.m_tag = KGlobal::locale()->formatDateTime(tagDateTimeLocal);
        }
        else
            m_entry.m_tag = tag;
    }
    else if (tag.length() > 1 && tag[0] == 'T')
        m_entry.m_tag = tag.mid(1);
    else
        m_entry.m_tag = tag;

    if (isVisible())
    {
        widthChanged();
        repaint();
    }
}


void UpdateFileItem::setDate(const QDateTime& date)
{
    m_entry.m_dateTime = date;
}


void UpdateFileItem::markUpdated(bool laststage,
                                 bool success)
{
    Entry::Status newstatus = m_entry.m_status;

    if (laststage)
    {
        if (undefinedState() && m_entry.m_status != Entry::NotInCVS)
            newstatus = success? Entry::UpToDate : Entry::Unknown;
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
    case Entry::Conflict:
        iResult = 0;
        break;
    case Entry::LocallyAdded:
        iResult = 1;
        break;
    case Entry::LocallyRemoved:
        iResult = 2;
        break;
    case Entry::LocallyModified:
        iResult = 3;
        break;
    case Entry::Updated:
    case Entry::NeedsUpdate:
    case Entry::Patched:
    case Entry::Removed:
    case Entry::NeedsPatch:
    case Entry::NeedsMerge:
        iResult = 4;
        break;
    case Entry::NotInCVS:
        iResult = 5;
        break;
    case Entry::UpToDate:
    case Entry::Unknown:
        iResult = 6;
        break;
    }

    return iResult;
}


int UpdateFileItem::compare(QListViewItem* i,
                            int column,
                            bool bAscending) const
{
    // UpdateDirItems are always lesser than UpdateFileItems
    if (isDirItem(i))
        return bAscending ? 1 : -1;

    const UpdateFileItem* item = static_cast<UpdateFileItem*>(i);

    int iResult(0);
    switch (column)
    {
    case Name:
        iResult = entry().m_name.localeAwareCompare(item->entry().m_name);
        break;
    case Status:
        if ((iResult = ::compare(statusClass(), item->statusClass())) == 0)
            iResult = entry().m_name.localeAwareCompare(item->entry().m_name);
        break;
    case Revision:
        iResult = ::compareRevisions(entry().m_revision, item->entry().m_revision);
        break;
    case TagOrDate:
        iResult = entry().m_tag.localeAwareCompare(item->entry().m_tag);
        break;
    case Timestamp:
        iResult = ::compare(entry().m_dateTime, item->entry().m_dateTime);
        break;
    }

    return iResult;
}


QString UpdateFileItem::text(int column) const
{
    QString result;
    switch (column)
    {
    case Name:
        result = entry().m_name;
        break;
    case Status:
        result = entry().statusToString();
        break;
    case Revision:
        result = entry().m_revision;
        break;
    case TagOrDate:
        result = entry().m_tag;
        break;
    case Timestamp:
        result = entry().m_dateTime.isValid()
            ? KGlobal::locale()->formatDateTime(entry().m_dateTime)
            : QString::null;
        break;
    }

    return result;
}


void UpdateFileItem::paintCell(QPainter *p,
                               const QColorGroup &cg,
                               int col,
                               int width,
                               int align)
{
    const UpdateView* view(updateView());

    QColor color;
    switch (m_entry.m_status)
    {
    case Entry::Conflict:
        color = view->conflictColor();
        break;
    case Entry::LocallyAdded:
    case Entry::LocallyModified:
    case Entry::LocallyRemoved:
        color = view->localChangeColor();
        break;
    case Entry::NeedsMerge:
    case Entry::NeedsPatch:
    case Entry::NeedsUpdate:
    case Entry::Patched:
    case Entry::Removed:
    case Entry::Updated:
        color = view->remoteChangeColor();
        break;
    case Entry::NotInCVS:
    case Entry::Unknown:
    case Entry::UpToDate:
        break;
    }

    const QFont oldFont(p->font());
    QColorGroup mycg(cg);
    if (color.isValid())
    {
        QFont myFont(oldFont);
        myFont.setBold(true);
        p->setFont(myFont);
        mycg.setColor(QColorGroup::Text, color);
    }

    QListViewItem::paintCell(p, mycg, col, width, align);

    if (color.isValid())
    {
        p->setFont(oldFont);
    }
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

    if (dirPath != QChar('.'))
    {
        const QStringList& dirNames(QStringList::split('/', dirPath));
        const QStringList::const_iterator itDirNameEnd(dirNames.end());
        for (QStringList::const_iterator itDirName(dirNames.begin());
             itDirName != itDirNameEnd; ++itDirName)
        {
            const QString& dirName(*itDirName);

            UpdateItem* item = dirItem->findItem(dirName);
            if (isFileItem(item))
            {
                kdDebug(8050) << "findOrCreateDirItem(): file changed to dir " << dirName << endl;
                delete item;
                item = 0;
            }

            if (!item)
            {
                kdDebug(8050) << "findOrCreateDirItem(): create dir item " << dirName << endl;
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
