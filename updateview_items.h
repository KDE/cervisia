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


#ifndef UPDATEVIEW_ITEMS_H
#define UPDATEVIEW_ITEMS_H


#include <qdatetime.h>
#include <qlistview.h>

#include <map>

#include "entry.h"
#include "updateview.h"


class UpdateDirItem;
class UpdateFileItem;
class Visitor;


UpdateDirItem* findOrCreateDirItem(const QString&, UpdateDirItem*);


class UpdateItem : public QListViewItem
{
public:

    UpdateItem(UpdateView* parent, const Cervisia::Entry& entry)
        : QListViewItem(parent), m_entry(entry) {}
    UpdateItem(UpdateItem* parent, const Cervisia::Entry& entry)
        : QListViewItem(parent), m_entry(entry) {}

    const Cervisia::Entry& entry() const { return m_entry; }

    // Returns the path (relative to the repository).
    // QString::null for the root item and its (direct) children.
    // If it's not QString::null it ends with '/'.
    QString dirPath() const;

    // Returns the file name, including the path (relative to the repository)
    QString filePath() const;

    virtual void accept(Visitor&) = 0;

    virtual void applyFilter(UpdateView::Filter filter) = 0;

protected:

    UpdateView* updateView() const { return static_cast<UpdateView*>(listView()); }

    Cervisia::Entry m_entry;
};


class UpdateDirItem : public UpdateItem
{
public:

    enum { Name };

    UpdateDirItem(UpdateView* parent, const Cervisia::Entry& entry);
    UpdateDirItem(UpdateDirItem* parent, const Cervisia::Entry& entry);

    void syncWithDirectory();
    void syncWithEntries();
    void updateChildItem(const QString& name, Cervisia::Entry::Status status, bool isdir);
    void updateEntriesItem(const Cervisia::Entry& entry, bool isBinary);

    bool wasScanned() const { return m_opened; }

    virtual int compare(QListViewItem* i, int col, bool) const;
    virtual QString text(int col) const;
    virtual void setOpen(bool o);
    virtual int rtti() const { return RTTI; }

    void maybeScanDir(bool recursive);

    virtual void accept(Visitor&);

    virtual void applyFilter(UpdateView::Filter filter);

    enum { RTTI = 10000 };

private:

    void scanDirectory();

    UpdateDirItem* createDirItem(const Cervisia::Entry& entry);
    UpdateFileItem* createFileItem(const Cervisia::Entry& entry);

    UpdateItem* findItem(const QString& name) const;

    typedef std::map<const QString, UpdateItem*> TMapItemsByName;

    TMapItemsByName mapItemsByName;

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

    virtual int compare(QListViewItem* i, int col, bool) const;
    virtual QString text(int col) const;
    virtual void paintCell(QPainter *p, const QColorGroup &cg,
                           int col, int width, int align);
    virtual int rtti() const { return RTTI; }

    void setStatus(Cervisia::Entry::Status status);
    void setRevTag(const QString& rev, const QString& tag);
    void setDate(const QDateTime& date);
    void setUndefinedState(bool b)
    { m_undefined = b; }

    void markUpdated(bool laststage, bool success);

    virtual void accept(Visitor&);

    virtual void applyFilter(UpdateView::Filter filter);

    enum { RTTI = 10001 };

private:

    int statusClass() const;

    bool m_undefined;
};


inline bool isDirItem(const QListViewItem* item)
{
    return item && item->rtti() == UpdateDirItem::RTTI;
}


inline bool isFileItem(const QListViewItem* item)
{
    return item && item->rtti() == UpdateFileItem::RTTI;
}


#endif // UPDATEVIEW_ITEMS_H
