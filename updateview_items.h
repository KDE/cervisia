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


#ifndef UPDATEVIEW_ITEMS_H
#define UPDATEVIEW_ITEMS_H


#include <qdatetime.h>
#include <QTreeWidgetItem>
#include <qmap.h>

#include "entry.h"
#include "updateview.h"


class UpdateDirItem;
class UpdateFileItem;
class Visitor;


UpdateDirItem* findOrCreateDirItem(const QString&, UpdateDirItem*);


class UpdateItem : public QTreeWidgetItem
{
public:

    UpdateItem(UpdateView* parent, const Cervisia::Entry& entry, int type)
        : QTreeWidgetItem(parent, type), m_entry(entry), itemDepth(0) { }

    UpdateItem(UpdateItem* parent, const Cervisia::Entry& entry, int type)
        : QTreeWidgetItem(parent, type), m_entry(entry), itemDepth(parent->depth() + 1) { }

    const Cervisia::Entry& entry() const { return m_entry; }

    // Returns the path (relative to the repository).
    // QString() for the root item and its (direct) children.
    // If it's not QString() it ends with '/'.
    QString dirPath() const;

    // Returns the file name, including the path (relative to the repository)
    QString filePath() const;

    virtual void accept(Visitor&) = 0;

    virtual void setOpen(bool ) { }

    int depth() const
    {
      return itemDepth;
    }

protected:

    UpdateView* updateView() const { return static_cast<UpdateView*>(treeWidget()); }

    Cervisia::Entry m_entry;

    int itemDepth;
};


class UpdateDirItem : public UpdateItem
{
public:

    enum { Name };

    UpdateDirItem(UpdateView* parent, const Cervisia::Entry& entry);
    UpdateDirItem(UpdateDirItem* parent, const Cervisia::Entry& entry);

    void syncWithDirectory();
    void syncWithEntries();
    void updateChildItem(const QString& name, Cervisia::EntryStatus status, bool isdir);
    void updateEntriesItem(const Cervisia::Entry& entry, bool isBinary);

    bool wasScanned() const { return m_opened; }

    bool operator<(const QTreeWidgetItem &other) const override;

    QVariant data(int column, int role) const override;

    void setOpen(bool o) override;

    void maybeScanDir(bool recursive);

    void accept(Visitor&) override;

    enum { RTTI = 10000 };

private:

    void scanDirectory();

    UpdateDirItem* createDirItem(const Cervisia::Entry& entry);
    UpdateFileItem* createFileItem(const Cervisia::Entry& entry);

    UpdateItem* insertItem(UpdateItem* item);

    UpdateItem* findItem(const QString& name) const;

    typedef QMap<QString, UpdateItem*> TMapItemsByName;

    TMapItemsByName m_itemsByName;

    bool m_opened;

    friend UpdateDirItem* findOrCreateDirItem(const QString&, UpdateDirItem*);
};


class UpdateFileItem : public UpdateItem
{
public:

    enum { Name, Status, Revision, TagOrDate, Timestamp };

    UpdateFileItem(UpdateDirItem* parent, const Cervisia::Entry& entry);

    bool undefinedState() const
    { return m_undefined; }

    bool operator<(const QTreeWidgetItem &other) const override;

    QVariant data(int column, int role) const override;

    void setStatus(Cervisia::EntryStatus status);
    void setRevTag(const QString& rev, const QString& tag);
    void setDate(const QDateTime& date);
    void setUndefinedState(bool b)
    { m_undefined = b; }

    void markUpdated(bool laststage, bool success);

    void accept(Visitor&) override;

    bool applyFilter(UpdateView::Filter filter);

    enum { RTTI = 10001 };

private:

    int statusClass() const;

    bool m_undefined;
};


inline bool isDirItem(const QTreeWidgetItem *item)
{
    return item && item->type() == UpdateDirItem::RTTI;
}


inline bool isFileItem(const QTreeWidgetItem *item)
{
    return item && item->type() == UpdateFileItem::RTTI;
}


#endif // UPDATEVIEW_ITEMS_H
