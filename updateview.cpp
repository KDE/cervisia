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

#include <set>

#include <qapplication.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qptrstack.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>

#include "config.h"
#include "misc.h"
#include "cervisiapart.h"
#include "cvsdir.h"


class UpdateItem : public QListViewItem
{
public:

    UpdateItem(UpdateView* parent, const QString& name)
        : QListViewItem(parent), m_name(name) {}
    UpdateItem(UpdateItem* parent, const QString& name)
        : QListViewItem(parent), m_name(name) {}

    const QString& name() const { return m_name; }

    virtual QString dirPath() const = 0;

    virtual void applyFilter(UpdateView::Filter filter) = 0;

protected:

    UpdateView* updateView() const { return static_cast<UpdateView*>(listView()); }

private:

    const QString m_name;
};


class UpdateDirItem : public UpdateItem
{
public:

    enum { Directory };

    UpdateDirItem( UpdateView *parent, const QString& dirname );
    UpdateDirItem( UpdateDirItem *parent, const QString& dirname );

    virtual QString dirPath() const;

    void syncWithDirectory();
    void syncWithEntries();
    void updateChildItem(const QString& name, UpdateView::Status status, bool isdir);
    void updateEntriesItem(const QString& name, UpdateView::Status status, bool isdir,
                           bool isbin, const QString& rev, const QString& tagname,
                           const QDateTime& date);

    virtual int compare(QListViewItem* i, int col, bool) const;
    virtual QString text(int col) const;
    virtual void setOpen(bool o);

    void maybeScanDir(bool recursive);

    virtual void applyFilter(UpdateView::Filter filter);

private:

    void scanDirectory();

    bool m_opened;
};


class UpdateFileItem : public UpdateItem
{
public:

    enum { File, Status, Revision, TagOrDate, Timestamp };

    UpdateFileItem(UpdateDirItem* parent, const QString& filename);

    virtual QString dirPath() const;
    QString filePath() const;

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


QString UpdateDirItem::dirPath() const
{
    const UpdateDirItem* dirItem = static_cast<UpdateDirItem*>(parent());
    return dirItem ? dirItem->dirPath() + name() + '/' : QString::null;
}


/**
 * Update the status of an item; if it doesn't exist yet, create new one
 */
void UpdateDirItem::updateChildItem(const QString& name, UpdateView::Status status, bool isdir)
{
    for (QListViewItem *item = firstChild(); item;
	 item = item->nextSibling() )
	{
	    if (item->text(UpdateFileItem::File) == name)
		{
		    if (UpdateView::isDirItem(item))
                        ; // ignore
		    else
                        {
                            UpdateFileItem* fileItem = static_cast<UpdateFileItem*>(item);
                            fileItem->setStatus(status);
                        }
		    return;
		}
	}

    // Not found, make new entry
    if (isdir)
        ( new UpdateDirItem(this, name) )->maybeScanDir(true);
    else
        {
            UpdateFileItem* fileItem = new UpdateFileItem(this, name);
            fileItem->setStatus(status);
        }
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
    for (QListViewItem *item = firstChild(); item;
	 item = item->nextSibling() )
	{
	    if (item->text(UpdateFileItem::File) == name)
		{
		    if (UpdateView::isDirItem(item))
                        ; // ignore
		    else
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
                            if (isbin)
                                fileItem->setPixmap(0, SmallIcon("binary"));
                        }
		    return;
		}
	}

    // Not found, make new entry
    if (isdir)
        ( new UpdateDirItem(this, name) )->maybeScanDir(true);
    else
        {
            UpdateFileItem* fileItem = new UpdateFileItem(this, name);
            fileItem->setStatus(status);
        }
}


void UpdateDirItem::scanDirectory()
{
    const QString& path(dirPath());
    if (!path.isEmpty() && !QFile::exists(path))
        return;

    const CvsDir dir(path);

    const QFileInfoList *files = dir.entryInfoList();
    if (files)
	{
	    QFileInfoListIterator it(*files);
	    for (; it.current(); ++it)
		{
		    if ( it.current()->isDir() )
			(void) new UpdateDirItem(this, it.current()->fileName());
		    else
			(void) new UpdateFileItem(this, it.current()->fileName());
		}
	}
}


// Format of the CVS/Entries file:
//   /NAME/REVISION/[CONFLICT+]TIMESTAMP/OPTIONS/TAGDATE

void UpdateDirItem::syncWithEntries()
{
    QFile f(dirPath() + "CVS/Entries");
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

                const bool isBinary(options == "-kb");

                // file date in local time
                // GCC 3.2.1 needs extra (), don't know why
                const QDateTime& fileDate((QFileInfo(dirPath() + name).lastModified()));

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
    const QDir dir( dirPath(), QString::null, QDir::Name,
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
        if (UpdateView::isDirItem(item) == false)
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
        }

    if (recursive)
        {
            for ( QListViewItem *item = firstChild(); item;
                  item = item->nextSibling() )
                if (UpdateView::isDirItem(item))
                    static_cast<UpdateDirItem*>(item)->maybeScanDir(true);
        }
}


void UpdateDirItem::applyFilter(UpdateView::Filter filter)
{
    setVisible(true);

    for (QListViewItem* childItem(firstChild());
         childItem != 0; childItem = childItem->nextSibling())
    {
        static_cast<UpdateItem*>(childItem)->applyFilter(filter);
    }
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
    if (UpdateView::isDirItem(i) == false)
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


UpdateFileItem::UpdateFileItem(UpdateDirItem* parent, const QString& filename)
    : UpdateItem(parent, filename),
      m_undefined(false),
      m_status(UpdateView::NotInCVS)
{
}


QString UpdateFileItem::dirPath() const
{
    return static_cast<UpdateDirItem*>(parent())->dirPath();
}


QString UpdateFileItem::filePath() const
{
    return dirPath() + name();
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
            m_tag = tag.mid(1, 4);
            m_tag += '/';
            m_tag += tag.mid(6, 2);
            m_tag += '/';
            m_tag += tag.mid(9, 2);
            m_tag += ' ';
            m_tag += tag.mid(12, 2);
            m_tag += ':';
            m_tag += tag.mid(15, 2);
            m_tag += ':';
            m_tag += tag.mid(18, 2);
        }
    else if (tag.length() > 1 && tag[0] == 'T')
        m_tag = tag.mid(1, tag.length()-1);
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
    if (UpdateView::isDirItem(i))
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

    QColorGroup mycg(cg);
    if (color.isValid())
        mycg.setColor(QColorGroup::Base, color);

    QListViewItem::paintCell(p, mycg, col, width, align);
}


UpdateView::UpdateView(QWidget *parent, const char *name)
    : ListView(parent, name)
{
    setAllColumnsShowFocus(true);
    setShowSortIndicator(true);
    setSelectionMode(Extended);

    addColumn(i18n("File Name"));
    addColumn(i18n("Status"));
    addColumn(i18n("Revision"));
    addColumn(i18n("Tag/Date"));
    addColumn(i18n("Timestamp"));

    QFontMetrics fm( fontMetrics() );

    int width = 0;
    width = QMAX(width, fm.width(i18n("Updated")));
    width = QMAX(width, fm.width(i18n("Patched")));
    width = QMAX(width, fm.width(i18n("Removed")));
    width = QMAX(width, fm.width(i18n("Needs Update")));
    width = QMAX(width, fm.width(i18n("Needs Patch")));
    width = QMAX(width, fm.width(i18n("Needs Merge")));
    width = QMAX(width, fm.width(i18n("Locally Added")));
    width = QMAX(width, fm.width(i18n("Locally Removed")));
    width = QMAX(width, fm.width(i18n("Locally Modified")));
    width = QMAX(width, fm.width(i18n("Up to date")));
    width = QMAX(width, fm.width(i18n("Conflict")));
    width = QMAX(width, fm.width(i18n("Not in CVS")));
    width = QMAX(width, fm.width(i18n("Unknown")));
    
    setColumnWidth(UpdateFileItem::Status, width+5);
    setPreferredColumn(UpdateFileItem::File);
    setFilter(NoFilter);

    connect( this, SIGNAL(doubleClicked(QListViewItem*)),
             this, SLOT(itemExecuted(QListViewItem*)) );
    connect( this, SIGNAL(returnPressed(QListViewItem*)),
             this, SLOT(itemExecuted(QListViewItem*)) );
    connect( this, SIGNAL(rightButtonPressed(QListViewItem*, const QPoint&, int)),
             this, SIGNAL(contextMenu()) );
}


UpdateView::~UpdateView()
{}


bool UpdateView::isDirItem(QListViewItem *item)
{
    return item->text(UpdateFileItem::Status).isEmpty();
}


void UpdateView::setFilter(Filter filter)
{
    filt = filter;

    if (UpdateDirItem* item = static_cast<UpdateDirItem*>(firstChild()))
    {
        item->applyFilter(filter);
    }

    setSorting(sortColumn(), sortAscending());
}


UpdateView::Filter UpdateView::filter() const
{
    return filt;
}


bool UpdateView::hasSingleSelection() const
{
    bool selfound = false;
    QPtrStack<QListViewItem> s;

    for ( QListViewItem *item = firstChild(); item;
	  item = item->nextSibling()? item->nextSibling() : s.pop() )
	{
	    if (item->firstChild())
                s.push(item->firstChild());

	    if (item->isSelected())
		{
		    if (selfound || item->isExpandable())
			return false;
		    selfound = true;
		}
	}
    return selfound;
}


void UpdateView::getSingleSelection(QString *filename, QString *revision) const
{
    QPtrStack<QListViewItem> s;

    for ( QListViewItem *item = firstChild(); item;
	  item = item->nextSibling()? item->nextSibling() : s.pop() )
	{
	    if (item->firstChild())
		s.push(item->firstChild());
	    else if (item->isSelected())
                {
                    UpdateFileItem* fileItem = static_cast<UpdateFileItem*>(item);
                    *filename = fileItem->filePath();
                    if (revision)
                        *revision = fileItem->revision();
                }
	}
}


QStringList UpdateView::multipleSelection() const
{
    QStringList res;
    QPtrStack<QListViewItem> s;

    for ( QListViewItem *item = firstChild(); item;
          item = item->nextSibling()? item->nextSibling() : s.pop() )
        {
            if (item->firstChild())
                s.push(item->firstChild());

            if (item->isSelected())
                {
                    if (isDirItem(item))
                        {
                            QString dirpath = static_cast<UpdateDirItem*>(item)->dirPath();
                            if (!dirpath.isEmpty())
                                dirpath.truncate(dirpath.length()-1);
                            else
                                dirpath = '.';
                            res.append(dirpath);
                        }
                    else
                        res.append( static_cast<UpdateFileItem*>(item)->filePath() );
                }
        }
    return res;
}


QStringList UpdateView::fileSelection() const
{
    QStringList res;
    QPtrStack<QListViewItem> s;

    for ( QListViewItem *item = firstChild(); item;
          item = item->nextSibling()? item->nextSibling() : s.pop() )
        {
            if (item->firstChild())
                s.push(item->firstChild());

            if (item->isSelected() && !isDirItem(item))
                res.append( static_cast<UpdateFileItem*>(item)->filePath() );
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
            item->setOpen(true);

        ++it;

        qApp->processEvents();
    }

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
                    if (!isDirItem(item))
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
    KConfig* config(CervisiaPart::config());
    config->setGroup("Colors");

    QColor defaultColor = QColor(255, 100, 100);
    m_conflictColor = config->readColorEntry("Conflict", &defaultColor);

    defaultColor = QColor(190, 190, 237);
    m_localChangeColor = config->readColorEntry("LocalChange", &defaultColor);

    defaultColor = QColor(255, 240, 190);
    m_remoteChangeColor = config->readColorEntry("RemoteChange", &defaultColor);
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


void UpdateView::updateItem(const QString &name, Status status, bool isdir)
{
    // bla -> dirpath = "", filename = "bla"
    // bla/foo -> dirpath = "bla/", filename = "foo"

    if (isdir && name == ".")
        return;
    
    const QFileInfo fi(name);
    QString dirpath(fi.dirPath());
    const QString fileName(fi.fileName());

    if (dirpath == ".")
        dirpath = QString::null;
    else
	dirpath += '/';

    UpdateDirItem *longestmatch = 0;
    QString longestmatchPath;
    QPtrStack<QListViewItem> s;
    for ( QListViewItem *item = firstChild(); item;
	  item = item->nextSibling()? item->nextSibling() : s.pop() )
	{
	    if (UpdateView::isDirItem(item))
		{
		    UpdateDirItem *diritem = static_cast<UpdateDirItem*>(item);
                    const QString& diritemPath(diritem->dirPath());
                    if (diritemPath == dirpath)
			{
			    diritem->updateChildItem(fileName, status, isdir);
			    return;
			}
                    else if (!diritemPath.isEmpty() && dirpath.startsWith(diritemPath)
                             && (!longestmatch || diritemPath.length() > longestmatchPath.length()))
                    {
                        longestmatch = diritem;
                        longestmatchPath = diritemPath;
                    }

                    if (item->firstChild())
                        s.push(item->firstChild());
		}
	}

    if (!longestmatch)
        {
            kdDebug() << "no match: " << dirpath << endl;
            return;
        }
    // Item doesn't belong any existing directory in the tree, so we have to create
    // the missing leaves. longestmatch is the directory item where we have to attach
    kdDebug() << "longest match: " << longestmatchPath << endl;
    kdDebug() << "leaves: " <<  dirpath.mid(longestmatchPath.length()) << endl;
    const QStringList& leaves(QStringList::split('/', dirpath.mid(longestmatchPath.length())));
    QString newFileName(longestmatchPath);
    for (QStringList::ConstIterator it(leaves.begin()); it != leaves.end(); ++it)
        {
            newFileName += *it;
            kdDebug() << "add missing " << newFileName << endl;
            updateItem(newFileName, Unknown, true);
            newFileName += '/';
        }
    // Recursive, but now it should work
    updateItem(name, status, isdir);
}


void UpdateView::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == KGlobalSettings::contextMenuKey())
        emit contextMenu();
    else
        ListView::keyPressEvent(e);
}


void UpdateView::itemExecuted(QListViewItem *item)
{
    if (!isDirItem(item))
        emit fileOpened(static_cast<UpdateFileItem*>(item)->filePath());
}

#include "updateview.moc"


// Local Variables:
// c-basic-offset: 4
// End:
