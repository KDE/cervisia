/*
 * Copyright (C) 1999-2002 Bernd Gehrmann <bernd@mail.berlios.de>
 * Copyright (c) 2003-2007 André Wöbbeking <Woebbeking@kde.org>
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


#include "updateview.h"

#include <set>

#include <qapplication.h>
#include <qfileinfo.h>
#include <qptrstack.h>
#include <kconfig.h>
#include <klocale.h>

#include "cervisiasettings.h"
#include "entry.h"
#include "updateview_items.h"
#include "updateview_visitors.h"


using Cervisia::Entry;
using Cervisia::EntryStatus;


UpdateView::UpdateView(KConfig& partConfig, QWidget *parent, const char *name)
    : KListView(parent, name),
      m_partConfig(partConfig),
      m_unfoldingTree(false)
{
    setAllColumnsShowFocus(true);
    setShowSortIndicator(true);
    setSelectionModeExt(Extended);

    addColumn(i18n("File Name"), 280);
    addColumn(i18n("File Type"), 180);
    addColumn(i18n("Status"), 90);
    addColumn(i18n("Revision"), 70);
    addColumn(i18n("Tag/Date"), 90);
    addColumn(i18n("Timestamp"), 120);

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
        ApplyFilterVisitor applyFilterVisitor(filter);
        item->accept(applyFilterVisitor);
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
        tmpRevision = fileItem->entry().m_revision;
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
        if ((*it)->isVisible())
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

        if (isFileItem(item) && item->isVisible())
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


const QColor& UpdateView::notInCvsColor() const
{
    return m_notInCvsColor;
}


bool UpdateView::isUnfoldingTree() const
{
    return m_unfoldingTree;
}


// updates internal data
void UpdateView::replaceItem(QListViewItem* oldItem,
                             QListViewItem* newItem)
{
    const int index(relevantSelection.find(oldItem));
    if (index >= 0)
        relevantSelection.replace(index, newItem);
}


void UpdateView::unfoldSelectedFolders()
{
    QApplication::setOverrideCursor(waitCursor);

    int previousDepth = 0;
    bool isUnfolded = false;

    QStringList selection = multipleSelection();

    // setup name of selected folder
    QString selectedItem = selection.first();
    if( selectedItem.contains('/') )
        selectedItem.remove(0, selectedItem.findRev('/')+1);

    // avoid flicker
    const bool updatesEnabled = isUpdatesEnabled();
    setUpdatesEnabled(false);

    QListViewItemIterator it(this);
    while( QListViewItem* item = it.current() )
    {
        if( isDirItem(item) )
        {
            UpdateDirItem* dirItem = static_cast<UpdateDirItem*>(item);

            // below selected folder?
            if( previousDepth && dirItem->depth() > previousDepth )
            {
                // if this dir wasn't scanned already scan it recursive
                // (this is only a hack to reduce the processEvents() calls,
                // setOpen() would scan the dir too)
                if (dirItem->wasScanned() == false)
                {
                    const bool recursive = true;
                    dirItem->maybeScanDir(recursive);

                    // scanning can take some time so keep the gui alive
                    qApp->processEvents();
                }

                dirItem->setOpen(!isUnfolded);
            }
            // selected folder?
            else if( selectedItem == dirItem->entry().m_name )
            {
                previousDepth = dirItem->depth();
                isUnfolded = dirItem->isOpen();

                // if this dir wasn't scanned already scan it recursive
                // (this is only a hack to reduce the processEvents() calls,
                // setOpen() would scan the dir too)
                if (dirItem->wasScanned() == false)
                {
                    const bool recursive = true;
                    dirItem->maybeScanDir(recursive);

                    // scanning can take some time so keep the gui alive
                    qApp->processEvents();
                }

                dirItem->setOpen(!isUnfolded);
            }
            // back to the level of the selected folder or above?
            else if( previousDepth && dirItem->depth() >= previousDepth )
            {
                previousDepth = 0;
            }
        }

        ++it;
    }

    // maybe some UpdateDirItem was opened the first time so check the whole tree
    setFilter(filter());

    setUpdatesEnabled(updatesEnabled);
    triggerUpdate();

    QApplication::restoreOverrideCursor();
}


void UpdateView::unfoldTree()
{
    QApplication::setOverrideCursor(waitCursor);

    m_unfoldingTree = true;

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
    setFilter(filter());

    setUpdatesEnabled(updatesEnabled);

    triggerUpdate();

    m_unfoldingTree = false;

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
void UpdateView::openDirectory(const QString& dirName)
{
    clear();

    // do this each time as the configuration could be changed
    updateColors();

    Entry entry;
    entry.m_name = dirName;
    entry.m_type = Entry::Dir;

    UpdateDirItem *item = new UpdateDirItem(this, entry);
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

    // maybe some new items were created or
    // visibility of items changed so check the whole tree
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

    m_notInCvsColor = CervisiaSettings::notInCvsColor();
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
        EntryStatus status(Cervisia::Unknown);
        switch (str[0].latin1())
        {
        case 'C':
            status = Cervisia::Conflict;
            break;
        case 'A':
            status = Cervisia::LocallyAdded;
            break;
        case 'R':
            status = Cervisia::LocallyRemoved;
            break;
        case 'M':
            status = Cervisia::LocallyModified;
            break;
        case 'U':
            status = (act == UpdateNoAct) ? Cervisia::NeedsUpdate : Cervisia::Updated;
            break;
        case 'P':
            status = (act == UpdateNoAct) ? Cervisia::NeedsPatch : Cervisia::Patched;
            break;
        case '?':
            status = Cervisia::NotInCVS;
            break;
        default:
            return;
        }
        updateItem(str.mid(2), status, false);
    }

    const QString removedFileStart(QString::fromLatin1("cvs server: "));
    const QString removedFileEnd(QString::fromLatin1(" is no longer in the repository"));
    if (str.startsWith(removedFileStart) && str.endsWith(removedFileEnd))
    {
    }

#if 0
    else if (str.left(21) == "cvs server: Updating " ||
             str.left(21) == "cvs update: Updating ")
        updateItem(str.right(str.length()-21), Unknown, true);
#endif
}


void UpdateView::updateItem(const QString& filePath, EntryStatus status, bool isdir)
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


#include "updateview.moc"


// Local Variables:
// c-basic-offset: 4
// End:
