/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "updateview.h"

#include <cassert>
#include <map>
#include <set>

#include <qapplication.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qpainter.h>
#include <qptrstack.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>

#include "misc.h"
#include "cvsdir.h"


class UpdateDirItem;


namespace
{
    const int g_dirItemRtti(10000);
    const int g_fileItemRtti(10001);


    bool isDirItem(const QListViewItem* item)
    {
        return item && item->rtti() == g_dirItemRtti;
    }


    bool isFileItem(const QListViewItem* item)
    {
        return item && item->rtti() == g_fileItemRtti;
    }
}


// don't know how to put this function in the unnamed namespace and make
// it a friend of UpdateDirItem
static UpdateDirItem* findOrCreateDirItem(const QString& dirPath, UpdateDirItem* rootItem);


class UpdateItem : public QListViewItem
{
public:

    UpdateItem(UpdateView* parent, const QString& name)
        : QListViewItem(parent), m_name(name) {}
    UpdateItem(UpdateItem* parent, const QString& name)
        : QListViewItem(parent), m_name(name) {}

    const QString& name() const { return m_name; }

    // Returns the path (relative to the repository).
    // QString::null for the root item and its (direct) children.
    // If it's not QString::null it ends with '/'.
    QString dirPath() const;

    // Returns the file name, including the path (relative to the repository) 
    QString filePath() const;

    virtual void applyFilter(UpdateView::Filter filter) = 0;

protected:

    UpdateView* updateView() const { return static_cast<UpdateView*>(listView()); }

private:

    const QString m_name;
};


class UpdateFileItem;


class UpdateDirItem : public UpdateItem
{
public:

    enum { Directory };

    UpdateDirItem( UpdateView *parent, const QString& dirname );
    UpdateDirItem( UpdateDirItem *parent, const QString& dirname );

    void syncWithDirectory();
    void syncWithEntries();
    void updateChildItem(const QString& name, UpdateView::Status status, bool isdir);
    void updateEntriesItem(const QString& name, UpdateView::Status status, bool isdir,
                           bool isbin, const QString& rev, const QString& tagname,
                           const QDateTime& date);

    bool wasScanned() const { return m_opened; }

    virtual int compare(QListViewItem* i, int col, bool) const;
    virtual QString text(int col) const;
    virtual void setOpen(bool o);
    virtual int rtti() const { return g_dirItemRtti; }

    void maybeScanDir(bool recursive);

    virtual void applyFilter(UpdateView::Filter filter);

private:

    void scanDirectory();

    UpdateDirItem* createDirItem(const QString& name);
    UpdateFileItem* createFileItem(const QString& name);

    UpdateItem* findItem(const QString& name) const;

    typedef std::map<const QString, UpdateItem*> TMapItemsByName;

    TMapItemsByName mapItemsByName;

    bool m_opened;

    friend UpdateDirItem* findOrCreateDirItem(const QString&, UpdateDirItem*);
};


class UpdateFileItem : public UpdateItem
{
public:

    enum { File, Status, Revision, TagOrDate, Timestamp };

    UpdateFileItem(UpdateDirItem* parent, const QString& filename);

    UpdateView::Status status() const
        { return m_status; }
    QString revision() const
        { return m_revision; }
    bool undefinedState() const
    { return m_undefined; }

    virtual int compare(QListViewItem* i, int col, bool) const;
    virtual QString text(int col) const;
    virtual void paintCell(QPainter *p, const QColorGroup &cg,
			   int col, int width, int align);
    virtual int rtti() const { return g_fileItemRtti; }

    void setStatus(UpdateView::Status status);
    void setRevTag(const QString& rev, const QString& tag);
    void setDate(const QDateTime& date);
    void setUndefinedState(bool b)
    { m_undefined = b; }

    void markUpdated(bool laststage, bool success);

    virtual void applyFilter(UpdateView::Filter filter);

private:

    int statusClass() const;

    QString m_revision;
    QString m_tag;
    bool m_undefined;
    UpdateView::Status m_status;
    QDateTime m_date;
};


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
            path.prepend(item->name() + QDir::separator());
        }

        item = parentItem;
    }

    return path;
}


QString UpdateItem::filePath() const
{
    // the filePath of the root item is '.'
    return parent() ? dirPath() + name() : QString::fromLatin1(".");
}


// ------------------------------------------------------------------------------
// UpdateDirItem
// ------------------------------------------------------------------------------


UpdateDirItem::UpdateDirItem( UpdateDirItem *parent, const QString& dirname )
    : UpdateItem(parent, dirname),
      m_opened(false)
{
    setExpandable(true);
    setPixmap(0, SmallIcon("folder"));
}


UpdateDirItem::UpdateDirItem( UpdateView *parent, const QString& dirname )
    : UpdateItem(parent, dirname),
      m_opened(false)
{
    setExpandable(true);
    setPixmap(0, SmallIcon("folder"));
}


/**
 * Update the status of an item; if it doesn't exist yet, create new one
 */
void UpdateDirItem::updateChildItem(const QString& name, UpdateView::Status status, bool isdir)
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
    if (isdir)
        createDirItem(name)->maybeScanDir(true);
    else
        createFileItem(name)->setStatus(status);
}


/**
 * Update the revision and tag of an item. Use status only to create
 * new items and for items which were NotInCVS.
 */
void UpdateDirItem::updateEntriesItem(const QString& name,
                                      UpdateView::Status status,
                                      bool isdir,
                                      bool isbin,
                                      const QString& rev,
                                      const QString& tagname,
                                      const QDateTime& date)
{
    if (UpdateItem* item = findItem(name))
    {
        if (isFileItem(item))
        {
            UpdateFileItem* fileItem = static_cast<UpdateFileItem*>(item);
            if (fileItem->status() == UpdateView::NotInCVS ||
                fileItem->status() == UpdateView::LocallyRemoved ||
                status == UpdateView::LocallyAdded ||
                status == UpdateView::LocallyRemoved ||
                status == UpdateView::Conflict)
            {
                fileItem->setStatus(status);
            }
            fileItem->setRevTag(rev, tagname);
            fileItem->setDate(date);
            fileItem->setPixmap(0, isbin ? SmallIcon("binary") : 0);
        }
        return;
    }

    // Not found, make new entry
    if (isdir)
        createDirItem(name)->maybeScanDir(true);
    else
    {
        UpdateFileItem* fileItem = createFileItem(name);
        fileItem->setStatus(status);
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
		    if ( it.current()->isDir() )
			createDirItem(it.current()->fileName());
		    else
			createFileItem(it.current()->fileName());
		}
	}
}


UpdateDirItem* UpdateDirItem::createDirItem(const QString& name)
{
    UpdateDirItem* dirItem = new UpdateDirItem(this, name);

    mapItemsByName.insert(std::make_pair(name, dirItem));

    return dirItem;
}


UpdateFileItem* UpdateDirItem::createFileItem(const QString& name)
{
    UpdateFileItem* fileItem = new UpdateFileItem(this, name);

    mapItemsByName.insert(std::make_pair(name, fileItem));

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

            const bool isDir(line[0] == 'D');
            if( isDir )
                line.remove(0, 1);

            if( line[0] != '/' )
                continue;

            const QString name(line.section('/', 1, 1));

            if (isDir)
            {
                updateEntriesItem(name, UpdateView::Unknown,
                                  isDir, false,
                                  QString::null,
                                  QString::null,
                                  QDateTime());
            }
            else
            {
                QString rev(line.section('/', 2, 2));
                const QString& timestamp(line.section('/', 3, 3));
                const QString& options(line.section('/', 4, 4));
                const QString& tagdate(line.section('/', 5, 5));

                const bool isBinary(options.find("-kb") >= 0);

                // file date in local time
                const QDateTime& fileDate(QFileInfo(path + name).lastModified());

                UpdateView::Status status(UpdateView::Unknown);
                if( rev == "0" )
                    status = UpdateView::LocallyAdded;
                else if( rev.length() > 2 && rev[0] == '-' )
                {
                    status = UpdateView::LocallyRemoved;
                    rev.remove(0, 1);
                }
                else if (timestamp.find('+') >= 0)
                {
                    status = UpdateView::Conflict;
                }
                else
                {
                    const QDateTime& date(QDateTime::fromString(timestamp)); // UTC Time
                    QDateTime fileDateUTC;
                    fileDateUTC.setTime_t(fileDate.toTime_t(), Qt::UTC);
                    if (date != fileDateUTC)
                        status = UpdateView::LocallyModified;
                }

                updateEntriesItem(name, status, isDir, isBinary, rev, tagdate, fileDate);
            }
        }

        f.close();
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
            // is file removed?
            if (setFiles.find(item->text(UpdateFileItem::File)) == setFiles.end())
            {
                UpdateFileItem* fileItem = static_cast<UpdateFileItem*>(item);
                fileItem->setStatus(UpdateView::Removed);
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


void UpdateDirItem::setOpen(bool o)
{
    if ( o )
        maybeScanDir(false);

    QListViewItem::setOpen( o );
}


int UpdateDirItem::compare(QListViewItem* i, int, bool bAscending) const
{
    // UpdateDirItems are always lesser than UpdateFileItems
    if (isFileItem(i))
        return bAscending ? -1 : 1;

    const UpdateDirItem* item(static_cast<UpdateDirItem*>(i));

    // for every column just compare the directory name
    return name().localeAwareCompare(item->name());
}


QString UpdateDirItem::text(int col) const
{
    switch (col)
	{
	case Directory:  return name();
	default: return QString::null;
	}
}


// ------------------------------------------------------------------------------
// UpdateFileItem
// ------------------------------------------------------------------------------


UpdateFileItem::UpdateFileItem(UpdateDirItem* parent, const QString& filename)
    : UpdateItem(parent, filename),
      m_undefined(false),
      m_status(UpdateView::NotInCVS)
{
}


void UpdateFileItem::setStatus(UpdateView::Status newstatus)
{
    if (newstatus != m_status)
        {
            m_status = newstatus;
            applyFilter(updateView()->filter());
            if (isVisible())
                repaint();
        }
    m_undefined = false;
}


void UpdateFileItem::applyFilter(UpdateView::Filter filter)
{
    bool hide = false;
    if (filter & UpdateView::OnlyDirectories)
        hide = true;
    if ((filter & UpdateView::NoUpToDate) && (m_status == UpdateView::UpToDate))
        hide = true;
    if ((filter & UpdateView::NoRemoved) && (m_status == UpdateView::Removed))
        hide = true;
    if ((filter & UpdateView::NoNotInCVS) && (m_status == UpdateView::NotInCVS))
        hide = true;
    setVisible(!hide);
}


void UpdateFileItem::setRevTag(const QString& rev, const QString& tag)
{
    m_revision = rev;

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

            m_tag = KGlobal::locale()->formatDateTime(tagDateTimeLocal);
        }
        else
            m_tag = tag;
    }
    else if (tag.length() > 1 && tag[0] == 'T')
        m_tag = tag.mid(1);
    else
        m_tag = tag;

    if (isVisible())
    {
        widthChanged();
        repaint();
    }
}

void UpdateFileItem::setDate(const QDateTime& date)
{
    m_date = date;
}

void UpdateFileItem::markUpdated(bool laststage, bool success)
{
    UpdateView::Status newstatus = m_status;

    if (laststage)
        {
            if (undefinedState() && m_status != UpdateView::NotInCVS)
                newstatus = success? UpdateView::UpToDate : UpdateView::Unknown;
            setStatus(newstatus);
        }
    else
        setUndefinedState(true);
}


int UpdateFileItem::statusClass() const
{
    int iResult(0);
    switch (m_status)
    {
    case UpdateView::Conflict:
        iResult = 0;
        break;
    case UpdateView::LocallyAdded:
        iResult = 1;
        break;
    case UpdateView::LocallyRemoved:
        iResult = 2;
        break;
    case UpdateView::LocallyModified:
        iResult = 3;
        break;
    case UpdateView::Updated:
    case UpdateView::NeedsUpdate:
    case UpdateView::Patched:
    case UpdateView::Removed:
    case UpdateView::NeedsPatch:
    case UpdateView::NeedsMerge:
        iResult = 4;
        break;
    case UpdateView::NotInCVS:
        iResult = 5;
        break;
    case UpdateView::UpToDate:
    case UpdateView::Unknown:
        iResult = 6;
        break;
    }

    return iResult;
}


int UpdateFileItem::compare(QListViewItem* i, int col, bool bAscending) const
{
    // UpdateDirItems are always lesser than UpdateFileItems
    if (isDirItem(i))
        return bAscending ? 1 : -1;

    const UpdateFileItem* pItem = static_cast<UpdateFileItem*>(i);

    int iResult(0);
    switch (col)
    {
    case File:
        iResult = name().localeAwareCompare(pItem->name());
        break;
    case Status:
        if ((iResult = ::compare(statusClass(), pItem->statusClass())) == 0)
            iResult = name().localeAwareCompare(pItem->name());
        break;
    case Revision:
        iResult = ::compareRevisions(m_revision, pItem->m_revision);
        break;
    case TagOrDate:
        iResult = m_tag.localeAwareCompare(pItem->m_tag);
        break;
    case Timestamp:
        iResult = ::compare(m_date, pItem->m_date);
        break;
    }

    return iResult;
}


QString UpdateFileItem::text(int col) const
{
    switch (col)
	{
	case File:
	    return name();
	case Status:
	    switch (m_status)
		{
		case UpdateView::LocallyModified:
		    return i18n("Locally Modified");
		case UpdateView::LocallyAdded:
		    return i18n("Locally Added");
		case UpdateView::LocallyRemoved:
		    return i18n("Locally Removed");
		case UpdateView::NeedsUpdate:
		    return i18n("Needs Update");
		case UpdateView::NeedsPatch:
		    return i18n("Needs Patch");
		case UpdateView::NeedsMerge:
		    return i18n("Needs Merge");
		case UpdateView::UpToDate:
		    return i18n("Up to date");
		case UpdateView::Conflict:
		    return i18n("Conflict");
		case UpdateView::Updated:
		    return i18n("Updated");
		case UpdateView::Patched:
		    return i18n("Patched");
		case UpdateView::Removed:
		    return i18n("Removed");
		case UpdateView::NotInCVS:
		    return i18n("Not in CVS");
		default:  return i18n("Unknown");
		}
	case Revision:
		return m_revision;
	case TagOrDate:
		return m_tag;
	case Timestamp:
            {
                return m_date.isValid() ? KGlobal::locale()->formatDateTime(m_date) : QString::null;
	    }
	default:
	    return QString::null;
	}
}



void UpdateFileItem::paintCell(QPainter *p, const QColorGroup &cg,
			       int col, int width, int align)
{
    const UpdateView* view(updateView());

    QColor color;
    switch (m_status)
    {
    case UpdateView::Conflict:
        color = view->conflictColor();
        break;
    case UpdateView::LocallyAdded:
    case UpdateView::LocallyModified:
    case UpdateView::LocallyRemoved:
        color = view->localChangeColor();
        break;
    case UpdateView::NeedsMerge:
    case UpdateView::NeedsPatch:
    case UpdateView::NeedsUpdate:
    case UpdateView::Patched:
    case UpdateView::Removed:
    case UpdateView::Updated:
        color = view->remoteChangeColor();
        break;
    case UpdateView::NotInCVS:
    case UpdateView::Unknown:
    case UpdateView::UpToDate:
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


// ------------------------------------------------------------------------------
// UpdateView
// ------------------------------------------------------------------------------


UpdateView::UpdateView(KConfig& partConfig, QWidget *parent, const char *name)
    : KListView(parent, name),
      m_partConfig(partConfig)
{
    setAllColumnsShowFocus(true);
    setShowSortIndicator(true);
    setSelectionModeExt(Extended);

    addColumn(i18n("File Name"));
    addColumn(i18n("Status"));
    addColumn(i18n("Revision"));
    addColumn(i18n("Tag/Date"));
    addColumn(i18n("Timestamp"));

    setFilter(NoFilter);

    connect( this, SIGNAL(doubleClicked(QListViewItem*)),
             this, SLOT(itemExecuted(QListViewItem*)) );
    connect( this, SIGNAL(returnPressed(QListViewItem*)),
             this, SLOT(itemExecuted(QListViewItem*)) );

    // without this restoreLayout() can't change the column widths
    for (int col = 0; col < columns(); ++col)
        setColumnWidthMode(col, QListView::Manual);

    restoreLayout(&m_partConfig, QString::fromLatin1("UpdateView"));
}


UpdateView::~UpdateView()
{
    saveLayout(&m_partConfig, QString::fromLatin1("UpdateView"));
}


void UpdateView::setFilter(Filter filter)
{
    filt = filter;

    if (UpdateDirItem* item = static_cast<UpdateDirItem*>(firstChild()))
    {
        item->applyFilter(filter);
    }

    setSorting(columnSorted(), ascendingSort());
}


UpdateView::Filter UpdateView::filter() const
{
    return filt;
}


// returns true iff exactly one UpdateFileItem is selected
bool UpdateView::hasSingleSelection() const
{
    const QPtrList<QListViewItem>& listSelectedItems(selectedItems());

    return (listSelectedItems.count() == 1) && isFileItem(listSelectedItems.getFirst());
}


void UpdateView::getSingleSelection(QString *filename, QString *revision) const
{
    const QPtrList<QListViewItem>& listSelectedItems(selectedItems());

    QString tmpFileName;
    QString tmpRevision;
    if ((listSelectedItems.count() == 1) && isFileItem(listSelectedItems.getFirst()))
    {
        UpdateFileItem* fileItem(static_cast<UpdateFileItem*>(listSelectedItems.getFirst()));
        tmpFileName = fileItem->filePath();
        tmpRevision = fileItem->revision();
    }

    *filename = tmpFileName;
    if (revision)
        *revision = tmpRevision;
}


QStringList UpdateView::multipleSelection() const
{
    QStringList res;

    const QPtrList<QListViewItem>& listSelectedItems(selectedItems());
    for (QPtrListIterator<QListViewItem> it(listSelectedItems);
         it.current() != 0; ++it)
    {
        res.append(static_cast<UpdateItem*>(*it)->filePath());
    }

    return res;
}


QStringList UpdateView::fileSelection() const
{
    QStringList res;

    const QPtrList<QListViewItem>& listSelectedItems(selectedItems());
    for (QPtrListIterator<QListViewItem> it(listSelectedItems);
         it.current() != 0; ++it)
    {
        QListViewItem* item(*it);

        if (isFileItem(item))
            res.append(static_cast<UpdateFileItem*>(item)->filePath());
    }

    return res;
}


const QColor& UpdateView::conflictColor() const
{
    return m_conflictColor;
}


const QColor& UpdateView::localChangeColor() const
{
    return m_localChangeColor;
}


const QColor& UpdateView::remoteChangeColor() const
{
    return m_remoteChangeColor;
}


void UpdateView::unfoldTree()
{
    QApplication::setOverrideCursor(waitCursor);

    const bool updatesEnabled(isUpdatesEnabled());

    setUpdatesEnabled(false);

    QListViewItemIterator it(this);
    while (QListViewItem* item = it.current())
    {
        if (isDirItem(item))
        {
            UpdateDirItem* dirItem(static_cast<UpdateDirItem*>(item));

            // if this dir wasn't scanned already scan it recursive
            // (this is only a hack to reduce the processEvents() calls,
            // setOpen() would scan the dir too)
            if (dirItem->wasScanned() == false)
            {
                const bool recursive(true);
                dirItem->maybeScanDir(recursive);

                // scanning can take some time so keep the gui alive
                qApp->processEvents();
            }

            dirItem->setOpen(true);
        }

        ++it;
    }

    // maybe some UpdateDirItem was opened the first time so check the whole tree
    // (this is needed for the filter NoEmptyDirectories)
    if (filter() & NoEmptyDirectories)
        setFilter(filter());

    setUpdatesEnabled(updatesEnabled);

    triggerUpdate();

    QApplication::restoreOverrideCursor();
}


void UpdateView::foldTree()
{
    QListViewItemIterator it(this);
    while (QListViewItem* item = it.current())
    {
        // don't close the top level directory
        if (isDirItem(item) && item->parent())
            item->setOpen(false);

        ++it;
    }
}


/**
 * Clear the tree view and insert the directory dirname
 * into it as the new root item
 */
void UpdateView::openDirectory(const QString& dirname)
{
    clear();

    // do this each time as the configuration could be changed
    updateColors();

    UpdateDirItem *item = new UpdateDirItem(this, dirname);
    item->setOpen(true);
    setCurrentItem(item);
    setSelected(item, true);
}


/**
 * Start a job. We want to be able to change the status field
 * correctly afterwards, so we have to remember the current
 * selection (which the user may change during the update).
 * In the recursive case, we collect all relevant directories.
 * Furthermore, we have to change the items to undefined state.
 */
void UpdateView::prepareJob(bool recursive, Action action)
{
    act = action;

    // Scan recursively all entries - there's no way around this here
    if (recursive)
        static_cast<UpdateDirItem*>(firstChild())->maybeScanDir(true);

    rememberSelection(recursive);
    if (act != Add)
        markUpdated(false, false);
}


/**
 * Finishes a job. What we do depends a bit on
 * whether the command was successful or not.
 */
void UpdateView::finishJob(bool normalExit, int exitStatus)
{
    // cvs exitStatus == 1 only means that there're conflicts
    const bool success(normalExit && (exitStatus == 0 || exitStatus == 1));
    if (act != Add)
        markUpdated(true, success);
    syncSelection();

    // visibility of items could be changed so check the whole tree
    // (this is needed for the filter NoEmptyDirectories)
    if (filter() & NoEmptyDirectories)
        setFilter(filter());
}


/**
 * Marking non-selected items in a directory updated (as a consequence
 * of not appearing in 'cvs update' output) is done in two steps: In the
 * first, they are marked as 'indefinite', so that their status on the screen
 * isn't misrepresented. In the second step, they are either set
 * to 'UpToDate' (success=true) or 'Unknown'.
 */
void UpdateView::markUpdated(bool laststage, bool success)
{
    QPtrListIterator<QListViewItem> it(relevantSelection);
    for ( ; it.current(); ++it)
        if (isDirItem(it.current()))
            {
                for (QListViewItem *item = it.current()->firstChild(); item;
                     item = item->nextSibling() )
                    if (isFileItem(item))
                        {
                            UpdateFileItem* fileItem = static_cast<UpdateFileItem*>(item);
                            fileItem->markUpdated(laststage, success);
                        }
            }
        else
            {
                UpdateFileItem* fileItem = static_cast<UpdateFileItem*>(it.current());
                fileItem->markUpdated(laststage, success);
            }
}


/**
 * Remember the selection, see prepareJob()
 */
void UpdateView::rememberSelection(bool recursive)
{
    std::set<QListViewItem*> setItems;
    for (QListViewItemIterator it(this); it.current(); ++it)
    {
        QListViewItem* item(it.current());

        // if this item is selected and if it was not inserted already
        // and if we work recursive and if it is a dir item then insert
        // all sub dirs
        // DON'T CHANGE TESTING ORDER
        if (item->isSelected()
            && setItems.insert(item).second
            && recursive
            && isDirItem(item))
        {
            QPtrStack<QListViewItem> s;
            for (QListViewItem* childItem = item->firstChild(); childItem;
                 childItem = childItem->nextSibling() ? childItem->nextSibling() : s.pop())
            {
                // if this item is a dir item and if it is was not
                // inserted already then insert all sub dirs
                // DON'T CHANGE TESTING ORDER
                if (isDirItem(childItem) && setItems.insert(childItem).second)
                {
                    if (QListViewItem* childChildItem = childItem->firstChild())
                        s.push(childChildItem);
                }
            }
        }
    }

    // Copy the set to the list
    relevantSelection.clear();
    std::set<QListViewItem*>::const_iterator const itItemEnd = setItems.end();
    for (std::set<QListViewItem*>::const_iterator itItem = setItems.begin();
         itItem != itItemEnd; ++itItem)
        relevantSelection.append(*itItem);

#if 0
    DEBUGOUT("Relevant:");
    QPtrListIterator<QListViewItem> it44(relevantSelection);
    for (; it44.current(); ++it44)
        DEBUGOUT("  " << (*it44)->text(UpdateFileItem::File));
    DEBUGOUT("End");
#endif
}


/**
 * Use the remembered selection to resynchronize
 * with the actual directory and Entries content.
 */
void UpdateView::syncSelection()
{
    // compute all directories which are selected or contain a selected file
    // (in recursive mode this includes all sub directories)
    std::set<UpdateDirItem*> setDirItems;
    for (QPtrListIterator<QListViewItem> itItem(relevantSelection);
         itItem.current(); ++itItem)
    {
        QListViewItem* item(itItem.current());

        UpdateDirItem* dirItem(0);
        if (isDirItem(item))
            dirItem = static_cast<UpdateDirItem*>(item);
        else if (QListViewItem* parentItem = item->parent())
            dirItem = static_cast<UpdateDirItem*>(parentItem);

        if (dirItem)
            setDirItems.insert(dirItem);
    }

    QApplication::setOverrideCursor(waitCursor);

    std::set<UpdateDirItem*>::const_iterator const itDirItemEnd = setDirItems.end();
    for (std::set<UpdateDirItem*>::const_iterator itDirItem = setDirItems.begin();
         itDirItem != itDirItemEnd; ++itDirItem)
    {
        UpdateDirItem* dirItem = *itDirItem;

        dirItem->syncWithDirectory();
        dirItem->syncWithEntries();

        qApp->processEvents();
    }

    QApplication::restoreOverrideCursor();
}


/**
 * Get the colors from the configuration each time the list view items
 * are created.
 */
void UpdateView::updateColors()
{
    KConfigGroupSaver cs(&m_partConfig, "Colors");
    m_partConfig.setGroup("Colors");

    QColor defaultColor = QColor(255, 130, 130);
    m_conflictColor = m_partConfig.readColorEntry("Conflict", &defaultColor);

    defaultColor = QColor(130, 130, 255);
    m_localChangeColor = m_partConfig.readColorEntry("LocalChange", &defaultColor);

    defaultColor = QColor(70, 210, 70);
    m_remoteChangeColor = m_partConfig.readColorEntry("RemoteChange", &defaultColor);
}


/**
 * Process one line from the output of 'cvs update'. If parseAsStatus
 * is true, it is assumed that the output is from a command
 * 'cvs update -n', i.e. cvs actually changes no files.
 */
void UpdateView::processUpdateLine(QString str)
{
    if (str.length() > 2 && str[1] == ' ')
        {
            const QChar statuschar = str[0];
            Status status = UpdateView::Unknown;
            if (statuschar == 'C')
                status = UpdateView::Conflict;
            else if (statuschar == 'A')
                status = UpdateView::LocallyAdded;
            else if (statuschar == 'R')
                status = UpdateView::LocallyRemoved;
            else if (statuschar == 'M')
                status = UpdateView::LocallyModified;
            else if (statuschar == 'U')
                status = (act==UpdateNoAct)?
                    UpdateView::NeedsUpdate : UpdateView::Updated;
            else if (statuschar == 'P')
                status = (act==UpdateNoAct)?
                    UpdateView::NeedsPatch : UpdateView::Patched;
            else if (statuschar == '?')
                status = UpdateView::NotInCVS;
            else
                return;
            updateItem(str.right(str.length()-2), status, false);
        }
#if 0
    else if (str.left(21) == "cvs server: Updating " ||
             str.left(21) == "cvs update: Updating ")
        updateItem(str.right(str.length()-21), Unknown, true);
#endif
}


void UpdateView::updateItem(const QString& filePath, Status status, bool isdir)
{
    if (isdir && filePath == QChar('.'))
        return;

    const QFileInfo fileInfo(filePath);

    UpdateDirItem* rootItem = static_cast<UpdateDirItem*>(firstChild());
    UpdateDirItem* dirItem = findOrCreateDirItem(fileInfo.dirPath(), rootItem);

    dirItem->updateChildItem(fileInfo.fileName(), status, isdir);
}


void UpdateView::itemExecuted(QListViewItem *item)
{
    if (isFileItem(item))
        emit fileOpened(static_cast<UpdateFileItem*>(item)->filePath());
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
                item = dirItem->createDirItem(dirName);
            }

            assert(isDirItem(item));

            dirItem = static_cast<UpdateDirItem*>(item);
        }
    }

    return dirItem;
}


#include "updateview.moc"


// Local Variables:
// c-basic-offset: 4
// End:
