/* 
 *  Copyright (C) 1999-2001 Bernd Gehrmann
 *                          bernd@physik.hu-berlin.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "config.h"
#include <qstrlist.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qstack.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kfileview.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include "misc.h"
#include "cvsdir.h"

#include <sys/stat.h>
#include <unistd.h>
#include <locale.h>
#include <time.h>

#include "updateview.h"
#include "updateview.moc"


class UpdateDirItem : public ListViewItem
{
public:
    UpdateDirItem( UpdateView *parent, QString dirname );
    UpdateDirItem( UpdateDirItem *parent, QString dirname );
 
    QString dirPath();
    void syncWithDirectory();
    void syncWithEntries();
    void updateChildItem(QString name, UpdateView::Status status, bool isdir);
    void updateEntriesItem(QString name, UpdateView::Status status, bool isdir,
                           QString rev, QString tagname, time_t timestamp);
    virtual QString key(int col, bool) const;
    virtual QString text(int col) const;
    virtual void setOpen(bool o);
    virtual void setup();
 
    void maybeScanDir(bool recursive);

private:
    UpdateView *updateView() const
        { return static_cast<UpdateView*>(listView()); }
    void scanDirectory();
    
    QString m_dirname;
    bool m_opened;
};


class UpdateViewItem : public ListViewItem
{
public:
    UpdateViewItem( ListViewItem *parent, QString filename );

    QString filePath();
    UpdateView::Status status() const
        { return m_status; }
    QString revision() const
        { return m_revision; }
    bool undefinedState() const
    { return m_undefined; }
    
    virtual QString key(int col, bool) const;
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
    QString m_filename;
    QString m_revision;
    QString m_tag;
    bool m_undefined;
    UpdateView::Status m_status;
    time_t m_timestamp;
};


UpdateDirItem::UpdateDirItem( UpdateDirItem *parent, QString dirname )
    : ListViewItem(parent)
{
    setPixmap(0, SmallIcon("folder"));
    m_dirname = dirname;
    m_opened = false;
}
 
 
UpdateDirItem::UpdateDirItem( UpdateView *parent, QString dirname )
    : ListViewItem(parent)
{
    setPixmap(0, SmallIcon("folder"));
    m_dirname = dirname;
    m_opened = false;
}


QString UpdateDirItem::dirPath()
{
    UpdateDirItem *diritem = static_cast<UpdateDirItem*>(parent());

    return parent()? diritem->dirPath() + m_dirname + "/" : QString("");
}                      


/**
 * Update the status of an item; if it doesn't exist yet, create new one
 */
void UpdateDirItem::updateChildItem(QString name, UpdateView::Status status, bool isdir)
{
    kdDebug() << "Updating " << name << " in " << dirPath() << endl;
    for (ListViewItem *item = myFirstChild(); item;
	 item = item->myNextSibling() )
	{
	    if (item->text(0) == name)
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
                                      QString rev, QString tagname, time_t timestamp)
{
    for (ListViewItem *item = myFirstChild(); item;
	 item = item->myNextSibling() )
	{
	    if (item->text(0) == name)
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
                        }
		    return;
		}
	}

    kdDebug() << "new entries item for " << name << endl;
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


static const char *lastModifiedStr(const char *fname)
{
    struct stat st;
    if (lstat(fname, &st) != 0)
	return "";
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

    FILE *f = fopen(QString(dirPath() + QString("CVS/Entries")).latin1(), "r");
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
	    else if (timestamp != lastModifiedStr(QString(dirPath() + name).latin1()))
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
	    updateEntriesItem(name, status, isdir, rev, tagdate, time);
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

    for (ListViewItem *item = myFirstChild(); item;
         item = item->myNextSibling() )
        {
            // Look if file still exists. We never remove directories!
            bool exists = false;
            if (UpdateView::isDirItem(item))
                exists = true;
            else if (files)
                {
                    QFileInfoListIterator it(*files);
                    for ( ; it.current(); ++it)
                        if (it.current()->fileName() == item->text(0))
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
                    viewitem->setRevTag("", "");
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
            for ( ListViewItem *item = myFirstChild(); item;
                  item = item->myNextSibling() )
                if (UpdateView::isDirItem(item))
                    static_cast<UpdateDirItem*>(item)->maybeScanDir(true);
        }
}


void UpdateDirItem::setOpen(bool o)
{
    if ( o )
        maybeScanDir(false);
    
    QListViewItem::setOpen( o );
}
 

QString UpdateDirItem::key(int col, bool) const
{
    static QString tmp;
    switch (col)
	{
	case 0: //return m_dirname;
	case 1: return tmp = QString("0") + m_dirname;
        default:return "";
	}
}


QString UpdateDirItem::text(int col) const
{
    switch (col)
	{
	case 0:  return m_dirname;
	default: return "";
	}
}       


void UpdateDirItem::setup()
{
    setExpandable(true);
    QListViewItem::setup();
}
 

UpdateViewItem::UpdateViewItem( ListViewItem *parent, QString filename )
    : ListViewItem(parent) 
{
    m_status = UpdateView::NotInCVS;
    m_filename = filename;
    m_revision = "";
    m_tag = "";
    m_undefined = false;
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
            if (visible())
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

    if (visible())
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


QString UpdateViewItem::key(int col, bool) const
{
    static QString tmp;
    QString prefix;

    switch (col)
	{
	case 0:
            // Put ordinary files behind all directories
            return tmp = QString("1") + m_filename;
	case 1:
	    // We want to have a kind of priority order when
	    // sorted by 'status'
	    switch (m_status)
		{
		case UpdateView::Conflict:
		    prefix = "1"; break;
		case UpdateView::LocallyAdded:
		    prefix = "2"; break;
		case UpdateView::LocallyRemoved:
		    prefix = "3"; break;
		case UpdateView::LocallyModified:
		    prefix = "4"; break;
		case UpdateView::Updated:
		case UpdateView::NeedsUpdate:
		case UpdateView::Patched:
                case UpdateView::Removed:
		case UpdateView::NeedsPatch:
		case UpdateView::NeedsMerge:
		    prefix = "5"; break;
		case UpdateView::NotInCVS:
		    prefix = "6"; break;
		default:  prefix = "7"; 
		}
	    return tmp = prefix + m_filename;
        case 2:
            return m_revision; // First approximation...
        case 3:
            return m_tag;
        case 4:
    //static QString sortingKey( const QString& value, bool isDir, int sortSpec);
    //static QString sortingKey( KIO::filesize_t value, bool isDir,int sortSpec);
            //return m_timestamp.toString();
	    return KFileView::sortingKey(m_timestamp, false, QDir::Time);
	default:
	    return "";
	}
}


QString UpdateViewItem::text(int col) const
{
    switch (col)
	{
	case 0:
	    return m_filename;
	case 1:
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
	case 2:
	    return m_revision;
        case 3:
            return m_tag;
        case 4:
	    {
		QDateTime timestamp;

		timestamp.setTime_t(m_timestamp);
                return timestamp.toString(Qt::LocalDate);
	    }
	default:
	    return "";
	}
}       



void UpdateViewItem::paintCell(QPainter *p, const QColorGroup &cg,
			       int col, int width, int align)
{
    QColor color =
	(m_status == UpdateView::Conflict)? QColor(255, 100, 100)
	: (m_status == UpdateView::LocallyModified ||
	   m_status == UpdateView::LocallyAdded  ||
	   m_status == UpdateView::LocallyRemoved)? QColor(190, 190, 237)
	: (m_status == UpdateView::Patched ||
	   m_status == UpdateView::Updated ||
           m_status == UpdateView::Removed ||
	   m_status == UpdateView::NeedsPatch ||
	   m_status == UpdateView::NeedsUpdate)? QColor(255, 240, 190)
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

    addColumn(i18n("File name"));
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
    
    setColumnWidth(1, width+5);
    setPreferredColumn(0);
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
    return qstrcmp(item->text(1).latin1(), "") == 0;
}


void UpdateView::setFilter(Filter filter)
{
    filt = filter;
    QStack<ListViewItem> s;
    QPtrList<ListViewItem> l;

    ListViewItem *item = static_cast<ListViewItem*>(firstChild());
    // Hack: Since applyFilter() changes the whole child structure, we have
    // to collect all children of one item in a list first, and then
    // iterator though the list
    while (item)
        {
            for (ListViewItem *childItem1 = item->myFirstChild();
                 childItem1; childItem1 = childItem1->myNextSibling())
                l.append(childItem1);

            for (ListViewItem *childItem2 = l.first();
                 childItem2; childItem2 = l.next())
                {
                    if (childItem2->myFirstChild())
                        s.push(childItem2);
                    
                    if (!isDirItem(childItem2))
                        {
                            static_cast<UpdateViewItem*>(childItem2)->applyFilter(filt);
                        }
                }

            l.clear();
            item = s.pop();
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
    QStack<QListViewItem> s;

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
    QStack<QListViewItem> s;

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
    QStack<QListViewItem> s;

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
    QStack<QListViewItem> s;

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
    
    QStack<QListViewItem> s;
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
    QStack<QListViewItem> s;
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
    QPtrListIterator<ListViewItem> it(relevantSelection);
    for ( ; it.current(); ++it)
        if (isDirItem(it.current()))
            {
                for (ListViewItem *item = it.current()->myFirstChild(); item;
                     item = item->myNextSibling() )
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

    QPtrList<ListViewItem> shallowItems, deepItems;

    QStack<QListViewItem> s;
    for ( QListViewItem *item = firstChild(); item;
	  item = item->nextSibling()? item->nextSibling() : s.pop() )
	{
            if (item->firstChild())
                s.push(item->firstChild());
            if (isSelected(item))
                shallowItems.append(static_cast<ListViewItem*>(item));
	}

    // In the recursive case, we add all directories from the hierarchies
    // under the selected directories.
    
    if (recursive)
        {
            QPtrListIterator<ListViewItem> it(shallowItems);
            for ( ; it.current(); ++it)
                if (isDirItem(it.current()))
                    for ( QListViewItem *item = it.current()->firstChild(); item;
                          item = item->nextSibling()? item->nextSibling() : s.pop() )
                        {
                            if (item->firstChild())
                                s.push(item->firstChild());
                            if (isDirItem(item))
                                deepItems.append(static_cast<ListViewItem*>(item));
                        }
        }

#if 0
    DEBUGOUT("Deep:");
    QPtrListIterator<ListViewItem> it42(deepItems);
    for (; it42.current(); ++it42)
        DEBUGOUT("  " << (*it42)->text(0));
    DEBUGOUT("Shallow:");
    QPtrListIterator<ListViewItem> it43(shallowItems);
    for (; it43.current(); ++it43)
        DEBUGOUT("  " << (*it43)->text(0));
#endif
    
    // Collect everything together, and avoid duplicates:
    
    relevantSelection.clear();
    QPtrListIterator<ListViewItem> it1(shallowItems);
    for ( ; it1.current(); ++it1)
        if (!relevantSelection.contains(it1.current()))
            relevantSelection.append(it1.current());
    QPtrListIterator<ListViewItem> it2(deepItems);
    for ( ; it2.current(); ++it2)
        if (!relevantSelection.contains(it2.current()))
            relevantSelection.append(it2.current());

#if 0
    DEBUGOUT("Relevant:");
    QPtrListIterator<ListViewItem> it44(relevantSelection);
    for (; it44.current(); ++it44)
        DEBUGOUT("  " << (*it44)->text(0));
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
    
    QPtrListIterator<ListViewItem> it1(relevantSelection);
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
    	dirpath = "";
    else
	dirpath += '/';

    UpdateDirItem *longestmatch = 0;
    QStack<QListViewItem> s;
    for ( ListViewItem *item = static_cast<ListViewItem*>(firstChild()); item;
	  item = item->myNextSibling()? item->myNextSibling() : static_cast<ListViewItem*>(s.pop()) )
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
                    
                    if (item->myFirstChild())
                        s.push(item->myFirstChild());
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
    for (int i=0; i < leaves.count(); ++i)
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

// Local Variables:
// c-basic-offset: 4
// End:
