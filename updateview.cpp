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

#include <sys/stat.h>
#include <unistd.h>
#include <locale.h>
#include <time.h>
#include <qapplication.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qheader.h>
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


class UpdateDirItem : public QListViewItem
{
public:

    enum { Directory };

    UpdateDirItem( UpdateView *parent, QString dirname );
    UpdateDirItem( UpdateDirItem *parent, QString dirname );

    QString dirPath();
    void syncWithDirectory();
    void syncWithEntries();
    void updateChildItem(QString name, UpdateView::Status status, bool isdir);
    void updateEntriesItem(QString name, UpdateView::Status status, bool isdir,
                           bool isbin, QString rev, QString tagname, time_t timestamp);

    virtual int compare(QListViewItem* i, int col, bool) const;
    virtual QString text(int col) const;
    virtual void setOpen(bool o);
    virtual void setup();
 
    void maybeScanDir(bool recursive);

    void applyFilter(UpdateView::Filter filter);

private:
    UpdateView *updateView() const
        { return static_cast<UpdateView*>(listView()); }
    void scanDirectory();
    
    QString m_dirname;
    bool m_opened;
};


class UpdateViewItem : public QListViewItem
{
public:

    enum { File, Status, Revision, TagOrDate, Timestamp };

    UpdateViewItem( QListViewItem *parent, QString filename );

    QString filePath();
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

    void setStatus(UpdateView::Status status, UpdateView::Filter filter);
    void applyFilter(UpdateView::Filter filter);
    void setRevTag(QString rev, QString tag);
    void setTimestamp(time_t timestamp);
    void setUndefinedState(bool b)
    { m_undefined = b; }
    void markUpdated(bool laststage, bool success, UpdateView::Filter filter);

private:

    int statusClass() const;

    QString m_filename;
    QString m_revision;
    QString m_tag;
    bool m_undefined;
    UpdateView::Status m_status;
    time_t m_timestamp;
    
    QColor conflictColor;
    QColor localChangeColor;
    QColor remoteChangeColor;
};


UpdateDirItem::UpdateDirItem( UpdateDirItem *parent, QString dirname )
    : QListViewItem(parent)
{
    setPixmap(0, SmallIcon("folder"));
    m_dirname = dirname;
    m_opened = false;
}

 
UpdateDirItem::UpdateDirItem( UpdateView *parent, QString dirname )
    : QListViewItem(parent)
{
    setPixmap(0, SmallIcon("folder"));
    m_dirname = dirname;
    m_opened = false;
}


QString UpdateDirItem::dirPath()
{
    UpdateDirItem *diritem = static_cast<UpdateDirItem*>(parent());

    return parent()? diritem->dirPath() + m_dirname + "/" : QString::null;
}                      


/**
 * Update the status of an item; if it doesn't exist yet, create new one
 */
void UpdateDirItem::updateChildItem(QString name, UpdateView::Status status, bool isdir)
{
    for (QListViewItem *item = firstChild(); item;
	 item = item->nextSibling() )
	{
	    if (item->text(UpdateViewItem::File) == name)
		{
		    if (UpdateView::isDirItem(item))
                        ; // ignore
		    else
                        {
                            UpdateViewItem *viewitem = static_cast<UpdateViewItem*>(item);
                            UpdateView::Filter filter = updateView()->filter();
                            viewitem->setStatus(status, filter);
                        }
		    return;
		}
	}

    // Not found, make new entry
    if (isdir)
        ( new UpdateDirItem(this, name) )->maybeScanDir(true);
    else
        {
            UpdateViewItem *viewitem = new UpdateViewItem(this, name);
            UpdateView::Filter filter = updateView()->filter();
            viewitem->setStatus(status, filter);
        }
}


/**
 * Update the revision and tag of an item. Use status only to create
 * new items and for items which were NotInCVS.
 */
void UpdateDirItem::updateEntriesItem(QString name, UpdateView::Status status, bool isdir,
                                      bool isbin, QString rev, QString tagname, time_t timestamp)
{
    for (QListViewItem *item = firstChild(); item;
	 item = item->nextSibling() )
	{
	    if (item->text(UpdateViewItem::File) == name)
		{
		    if (UpdateView::isDirItem(item))
                        ; // ignore
		    else
                        {
                            UpdateViewItem *viewitem = static_cast<UpdateViewItem*>(item);
                            if (viewitem->status() == UpdateView::NotInCVS ||
                                viewitem->status() == UpdateView::LocallyRemoved ||
                                status == UpdateView::LocallyAdded ||
                                status == UpdateView::LocallyRemoved ||
                                status == UpdateView::Conflict)
                                {
                                    UpdateView::Filter filter = updateView()->filter();
                                    viewitem->setStatus(status, filter);
                                }
                            viewitem->setRevTag(rev, tagname);
                            viewitem->setTimestamp(timestamp);
                            if (isbin)
                                viewitem->setPixmap(0, SmallIcon("binary"));
                        }
		    return;
		}
	}

    // Not found, make new entry
    if (isdir)
        ( new UpdateDirItem(this, name) )->maybeScanDir(true);
    else
        {
            UpdateViewItem *viewitem = new UpdateViewItem(this, name);
            UpdateView::Filter filter = updateView()->filter();
            viewitem->setStatus(status, filter);
        }
}


void UpdateDirItem::scanDirectory()
{
    if (!dirPath().isEmpty() && !QFile::exists(dirPath()))
        return;

    CvsDir dir( dirPath() );
    
    const QFileInfoList *files = dir.entryInfoList();
    if (files)
	{
	    QFileInfoListIterator it(*files);
	    for (; it.current(); ++it)
		{
		    if ( it.current()->isDir() )
			(void) new UpdateDirItem(this, it.current()->fileName());
		    else
			(void) new UpdateViewItem(this, it.current()->fileName());
		}
	}
}


static QString lastModifiedStr(const QString &fname)
{
    struct stat st;
    if (lstat(fname.local8Bit(), &st) != 0)
	return QString::null;
    struct tm *tm_p = gmtime(&st.st_mtime);
    char *p = asctime(tm_p);
    p[24] = '\0';
    return p;
}


// Format of the CVS/Entries file:
//   /NAME/REVISION/TIMESTAMP[+CONFLICT]/OPTIONS/TAGDATE

void UpdateDirItem::syncWithEntries()
{
    char buf[512];
    QString name, rev, timestamp, options, tagdate;
    UpdateView::Status status;

    FILE *f = fopen(QString(dirPath() + QString("CVS/Entries")).local8Bit(), "r");
    if (!f)
	return;

    while (fgets(buf, sizeof buf, f))
	{
            char *nextp, *p = buf;
            bool isdir = (*p == 'D');
            if (isdir)
                ++p;
	    if (*p != '/')
		continue;
            ++p;
	    if ( (nextp = strchr(p, '/')) == 0)
		continue;
	    *nextp = '\0';
	    name = QString(p); p = nextp+1;
	    if ( (nextp = strchr(p, '/')) == 0)
		continue;
	    *nextp = '\0';
	    rev = QString(p); p = nextp+1;
	    if ( (nextp = strchr(p, '/')) == 0)
		continue;
	    *nextp = '\0';
	    timestamp = QString(p); p = nextp+1;
	    if ( (nextp = strchr(p, '/')) == 0)
		continue;
	    *nextp = '\0';
	    options = QString(p); p = nextp+1;
            bool isbin = (options == "-kb");
	    if ( (nextp = strchr(p, '\n')) == 0)
		continue;
	    *nextp = '\0';
	    tagdate = QString(p); p = nextp+1;

	    if (rev == "0")
		status = UpdateView::LocallyAdded;
	    else if (rev.length() > 2 && rev[0] == '-')
                {
                    status = UpdateView::LocallyRemoved;
                    rev.remove(0, 1);
                }
	    else if (timestamp.find('+') != -1)
                {
		    status = UpdateView::Conflict;
		    timestamp.truncate(timestamp.find('+'));
                }
	    else if (timestamp != lastModifiedStr(dirPath() + name))
		status = UpdateView::LocallyModified;
	    else
		status = UpdateView::Unknown;

	    // Convert timestamp into a time.
	    char *oldLocale;
	    struct tm tmp;
	    time_t time;

	    oldLocale = setlocale(LC_TIME, "C");
	    strptime(timestamp.local8Bit(), "%c" , &tmp);
	    setlocale(LC_TIME, oldLocale);
	    time = mktime(&tmp);
	    updateEntriesItem(name, status, isdir, isbin, rev, tagdate, time);
	}
    fclose(f);
}


void UpdateDirItem::syncWithDirectory()
{
    // Do not use CvsDir here, because CVS/Entries may
    // contain files which are in .cvsignore (stupid
    // idea, but that's possible...)
    QDir dir( dirPath(), QString::null, QDir::Name,
              QDir::Files|QDir::Hidden|QDir::NoSymLinks );

    const QFileInfoList *files = dir.exists()? dir.entryInfoList() : 0;

    for (QListViewItem *item = firstChild(); item;
         item = item->nextSibling() )
        {
            // Look if file still exists. We never remove directories!
            bool exists = false;
            if (UpdateView::isDirItem(item))
                exists = true;
            else if (files)
                {
                    QFileInfoListIterator it(*files);
                    for ( ; it.current(); ++it)
                        if (it.current()->fileName() == item->text(UpdateViewItem::File))
                            {
                                exists = true;
                                break;
                            }
                }
            if (!exists)
                {
                    UpdateViewItem *viewitem = static_cast<UpdateViewItem*>(item);
                    UpdateView::Filter filter = updateView()->filter();
                    viewitem->setStatus(UpdateView::Removed, filter);
                    viewitem->setRevTag(QString::null, QString::null);
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
        if (UpdateView::isDirItem(childItem))
        {
            static_cast<UpdateDirItem*>(childItem)->applyFilter(filter);
        }
        else
        {
            static_cast<UpdateViewItem*>(childItem)->applyFilter(filter);
        }
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
    // UpdateDirItems are always lesser than UpdateViewItems
    if (UpdateView::isDirItem(i) == false)
        return bAscending ? -1 : 1;

    UpdateDirItem const* pItem = static_cast<UpdateDirItem*>(i);

    // for every column just compare the directory name
    return m_dirname.localeAwareCompare(pItem->m_dirname);
}


QString UpdateDirItem::text(int col) const
{
    switch (col)
	{
	case Directory:  return m_dirname;
	default: return QString::null;
	}
}


void UpdateDirItem::setup()
{
    setExpandable(true);
    QListViewItem::setup();
}


UpdateViewItem::UpdateViewItem( QListViewItem *parent, QString filename )
    : QListViewItem(parent)
{
    m_status = UpdateView::NotInCVS;
    m_filename = filename;
    m_undefined = false;

    KConfig *config = CervisiaPart::config();
    config->setGroup("Colors");
     QColor defaultColor = QColor(255, 100, 100);
    conflictColor=config->readColorEntry("Conflict",&defaultColor);
    defaultColor=QColor(190, 190, 237);
    localChangeColor=config->readColorEntry("LocalChange",&defaultColor);
    defaultColor=QColor(255, 240, 190);
    remoteChangeColor=config->readColorEntry("RemoteChange",&defaultColor);
}


QString UpdateViewItem::filePath()
{
    UpdateDirItem *diritem = static_cast<UpdateDirItem*>(parent());
    return diritem->dirPath() + m_filename;
}                      


void UpdateViewItem::setStatus(UpdateView::Status newstatus, UpdateView::Filter filter)
{
    if (newstatus != m_status)
        {
            m_status = newstatus;
            applyFilter(filter);
            if (isVisible())
                repaint();
        }
    m_undefined = false;
}


void UpdateViewItem::applyFilter(UpdateView::Filter filter)
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


void UpdateViewItem::setRevTag(QString rev, QString tag)
{
    m_revision = rev;

    if (tag.length() == 20 && tag[0] == 'D' && tag[5] == '.'
        && tag[8] == '.' && tag[11] == '.' && tag[14] == '.'
        && tag[17] == '.')
        {
            m_tag = tag.mid(1, 4);
            m_tag += "/";
            m_tag += tag.mid(6, 2);
            m_tag += "/";
            m_tag += tag.mid(9, 2);
            m_tag += " ";
            m_tag += tag.mid(12, 2);
            m_tag += ":";
            m_tag += tag.mid(15, 2);
            m_tag += ":";
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

void UpdateViewItem::setTimestamp(time_t timestamp)
{
    m_timestamp = timestamp;
}

void UpdateViewItem::markUpdated(bool laststage, bool success, UpdateView::Filter filter)
{
    UpdateView::Status newstatus = m_status;
    
    if (laststage)
        {
            if (undefinedState() && m_status != UpdateView::NotInCVS)
                newstatus = success? UpdateView::UpToDate : UpdateView::Unknown;
            setStatus(newstatus, filter);
        }
    else
        setUndefinedState(true);
}


int UpdateViewItem::statusClass() const
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


int UpdateViewItem::compare(QListViewItem* i, int col, bool bAscending) const
{
    // UpdateDirItems are always lesser than UpdateViewItems
    if (UpdateView::isDirItem(i))
        return bAscending ? 1 : -1;

    UpdateViewItem const* pItem = static_cast<UpdateViewItem*>(i);

    int iResult(0);
    switch (col)
    {
    case File:
        iResult = m_filename.localeAwareCompare(pItem->m_filename);
        break;
    case Status:
        if ((iResult = ::compare(statusClass(), pItem->statusClass())) == 0)
            iResult = m_filename.localeAwareCompare(pItem->m_filename);
        break;
    case Revision:
        iResult = ::compareRevisions(m_revision, pItem->m_revision);
        break;
    case TagOrDate:
        iResult = m_tag.localeAwareCompare(pItem->m_tag);
        break;
    case Timestamp:
        iResult = ::compare(m_timestamp, pItem->m_timestamp);
        break;
    }

    return iResult;
}


QString UpdateViewItem::text(int col) const
{
    switch (col)
	{
	case File:
	    return m_filename;
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
                QDateTime timestamp;
                timestamp.setTime_t(m_timestamp);
                return KGlobal::locale()->formatDateTime(timestamp);
	    }
	default:
	    return QString::null;
	}
}       



void UpdateViewItem::paintCell(QPainter *p, const QColorGroup &cg,
			       int col, int width, int align)
{
    QColor color =
	(m_status == UpdateView::Conflict)? conflictColor
	: (m_status == UpdateView::LocallyModified ||
	   m_status == UpdateView::LocallyAdded  ||
	   m_status == UpdateView::LocallyRemoved)? localChangeColor
	: (m_status == UpdateView::Patched ||
	   m_status == UpdateView::Updated ||
           m_status == UpdateView::Removed ||
	   m_status == UpdateView::NeedsPatch ||
	   m_status == UpdateView::NeedsUpdate)? remoteChangeColor
	: cg.base();
    QColorGroup mycg(cg);
    mycg.setBrush(QColorGroup::Base, color);

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
    
    setColumnWidth(UpdateViewItem::Status, width+5);
    setPreferredColumn(UpdateViewItem::File);
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
    return item->text(UpdateViewItem::Status).isEmpty();
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


bool UpdateView::hasSingleSelection()
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


void UpdateView::getSingleSelection(QString *filename, QString *revision)
{
    QPtrStack<QListViewItem> s;

    for ( QListViewItem *item = firstChild(); item;
	  item = item->nextSibling()? item->nextSibling() : s.pop() )
	{
	    if (item->firstChild())
		s.push(item->firstChild());
	    else if (item->isSelected())
                {
                    UpdateViewItem *viewitem = static_cast<UpdateViewItem*>(item);
                    *filename = viewitem->filePath();
                    if (revision)
                        *revision = viewitem->revision();
                }
	}
}


QStringList UpdateView::multipleSelection()
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
                                dirpath = ".";
                            res.append(dirpath);
                        }
                    else
                        res.append( static_cast<UpdateViewItem*>(item)->filePath() );
                }
        }
    return res;
}


QStringList UpdateView::fileSelection()
{
    QStringList res;
    QPtrStack<QListViewItem> s;

    for ( QListViewItem *item = firstChild(); item;
          item = item->nextSibling()? item->nextSibling() : s.pop() )
        {
            if (item->firstChild())
                s.push(item->firstChild());

            if (item->isSelected() && !isDirItem(item))
                res.append( static_cast<UpdateViewItem*>(item)->filePath() );
        }
    return res;
}


void UpdateView::unfoldTree()
{
    QApplication::setOverrideCursor(waitCursor);
    
    QPtrStack<QListViewItem> s;
    for ( QListViewItem *item = firstChild(); item;
	  item = item->nextSibling()? item->nextSibling() : s.pop() )
	{
	    if (isDirItem(item))
                item->setOpen(true);
            if (item->firstChild())
		s.push(item->firstChild());
            qApp->processEvents();
	}

    triggerUpdate();    
    QApplication::restoreOverrideCursor();
}


void UpdateView::foldTree()
{
    QPtrStack<QListViewItem> s;
    for ( QListViewItem *item = firstChild(); item;
	  item = item->nextSibling()? item->nextSibling() : s.pop() )
	{
	    if (isDirItem(item) && item != firstChild())
                item->setOpen(false);
            if (item->firstChild())
		s.push(item->firstChild());
	}

    triggerUpdate();    
}


/**
 * Clear the tree view and insert the directory dirname
 * into it as the new root item
 */
void UpdateView::openDirectory(QString dirname)
{
    clear();
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
void UpdateView::finishJob(bool success)
{
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
                            UpdateViewItem *viewitem = static_cast<UpdateViewItem*>(item);
                            viewitem->markUpdated(laststage, success, filter());
                        }
            }
        else
            {
                UpdateViewItem *viewitem = static_cast<UpdateViewItem*>(it.current());
                viewitem->markUpdated(laststage, success, filter());
            }
}


/**
 * Remember the selection, see prepareJob()
 */
void UpdateView::rememberSelection(bool recursive)
{
    // Collect all selected dir and file items into relevantSelection

    QPtrList<QListViewItem> shallowItems, deepItems;

    QPtrStack<QListViewItem> s;
    for ( QListViewItem *item = firstChild(); item;
	  item = item->nextSibling()? item->nextSibling() : s.pop() )
	{
            if (item->firstChild())
                s.push(item->firstChild());
            if (isSelected(item))
                shallowItems.append(static_cast<QListViewItem*>(item));
	}

    // In the recursive case, we add all directories from the hierarchies
    // under the selected directories.
    
    if (recursive)
        {
            QPtrListIterator<QListViewItem> it(shallowItems);
            for ( ; it.current(); ++it)
                if (isDirItem(it.current()))
                    for ( QListViewItem *item = it.current()->firstChild(); item;
                          item = item->nextSibling()? item->nextSibling() : s.pop() )
                        {
                            if (item->firstChild())
                                s.push(item->firstChild());
                            if (isDirItem(item))
                                deepItems.append(static_cast<QListViewItem*>(item));
                        }
        }

#if 0
    DEBUGOUT("Deep:");
    QPtrListIterator<QListViewItem> it42(deepItems);
    for (; it42.current(); ++it42)
        DEBUGOUT("  " << (*it42)->text(UpdateViewItem::File));
    DEBUGOUT("Shallow:");
    QPtrListIterator<QListViewItem> it43(shallowItems);
    for (; it43.current(); ++it43)
        DEBUGOUT("  " << (*it43)->text(UpdateViewItem::File));
#endif
    
    // Collect everything together, and avoid duplicates:
    
    relevantSelection.clear();
    QPtrListIterator<QListViewItem> it1(shallowItems);
    for ( ; it1.current(); ++it1)
        if (!relevantSelection.contains(it1.current()))
            relevantSelection.append(it1.current());
    QPtrListIterator<QListViewItem> it2(deepItems);
    for ( ; it2.current(); ++it2)
        if (!relevantSelection.contains(it2.current()))
            relevantSelection.append(it2.current());

#if 0
    DEBUGOUT("Relevant:");
    QPtrListIterator<QListViewItem> it44(relevantSelection);
    for (; it44.current(); ++it44)
        DEBUGOUT("  " << (*it44)->text(UpdateViewItem::File));
    DEBUGOUT("End");
#endif
}


/**
 * Use the remembered selection to resynchronize
 * with the actual directory and Entries content.
 */
void UpdateView::syncSelection()
{
    QPtrList<UpdateDirItem> dirs;
    
    QPtrListIterator<QListViewItem> it1(relevantSelection);
    for ( ; it1.current(); ++it1)
	{
            UpdateDirItem *diritem = 0;
            if (isDirItem(it1.current()))
                diritem = static_cast<UpdateDirItem*>(it1.current());
            else if (it1.current()->parent())
                diritem = static_cast<UpdateDirItem*>(it1.current()->parent());
            if (diritem && !dirs.contains(diritem))
                dirs.append(diritem);
        }

    QApplication::setOverrideCursor(waitCursor);
    
    QPtrListIterator<UpdateDirItem> it2(dirs);
    for ( ; it2.current(); ++it2)
	{
            it2.current()->syncWithDirectory();
            it2.current()->syncWithEntries();
            qApp->processEvents();
        }
    
    QApplication::restoreOverrideCursor();
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
            QChar statuschar = str[0];
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
    
    QFileInfo fi(name);
    QString dirpath(fi.dirPath());
    QString fileName(fi.fileName());

    if (dirpath == ".") 
        dirpath = QString::null;
    else
	dirpath += '/';

    UpdateDirItem *longestmatch = 0;
    QPtrStack<QListViewItem> s;
    for ( QListViewItem *item = firstChild(); item;
	  item = item->nextSibling()? item->nextSibling() : s.pop() )
	{
	    if (UpdateView::isDirItem(item))
		{
		    UpdateDirItem *diritem = static_cast<UpdateDirItem*>(item);
		    if (diritem->dirPath() == dirpath)
			{
			    diritem->updateChildItem(fileName, status, isdir);
			    return;
			}
                    else if (!diritem->dirPath().isEmpty() && dirpath.startsWith(diritem->dirPath())
                             && (!longestmatch || diritem->dirPath().length() > longestmatch->dirPath().length()))
                        longestmatch = diritem;
                    
                    if (item->firstChild())
                        s.push(item->firstChild());
		}
	}

    if (!longestmatch)
        {
            kdDebug() << "no match: " << dirname << endl;
            return;
        }
    // Item doesn't belong any existing directory in the tree, so we have to create
    // the missing leaves. longestmatch is the directory item where we have to attach
    kdDebug() << "longest match: " << longestmatch->dirPath() << endl;
    kdDebug() << "leaves: " <<  dirpath.mid(longestmatch->dirPath().length()) << endl;
    QStringList leaves = QStringList::split('/', dirpath.mid(longestmatch->dirPath().length()));
    for (int i=0; i < (int)leaves.count(); ++i)
        {
            QString newFileName = longestmatch->dirPath();
            for (int j=0; j < i; ++j)
                {
                    newFileName += leaves[j];
                    newFileName += '/';
                }
            newFileName += leaves[i];
            kdDebug() << "add missing " << newFileName << endl;
            updateItem(newFileName, Unknown, true);
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
        emit fileOpened(static_cast<UpdateViewItem*>(item)->filePath());
}

#include "updateview.moc"


// Local Variables:
// c-basic-offset: 4
// End:
