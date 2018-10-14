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


#include "updateview.h"

#include <set>

#include <qapplication.h>
#include <QHeaderView>
#include <qfileinfo.h>
#include <qstack.h>
#include <kconfiggroup.h>
#include <KLocalizedString>

#include "cervisiasettings.h"
#include "updateview_items.h"
#include "updateview_visitors.h"


using Cervisia::Entry;
using Cervisia::EntryStatus;


UpdateView::UpdateView(KConfig& partConfig, QWidget *parent)
    : QTreeWidget(parent),
      m_partConfig(partConfig),
      m_unfoldingTree(false)
{
    setAllColumnsShowFocus(true);
    setUniformRowHeights(true);
    setRootIsDecorated(false);
    header()->setSortIndicatorShown(true);
    setSortingEnabled(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    setHeaderLabels(QStringList() << i18n("File Name") << i18n("Status") << i18n("Revision")
                                  << i18n("Tag/Date") << i18n("Timestamp"));

    header()->resizeSection(0, 280);
    header()->resizeSection(1, 90);
    header()->resizeSection(2, 70);
    header()->resizeSection(3, 90);
    header()->resizeSection(4, 120);

    setFilter(NoFilter);

    connect( this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
             this, SLOT(itemExecuted(QTreeWidgetItem*,int)) );

    connect( this, SIGNAL(itemExpanded(QTreeWidgetItem*)),
             this, SLOT(itemExpandedSlot(QTreeWidgetItem*)) );

    KConfigGroup cg(&m_partConfig, "UpdateView");
    QByteArray state = cg.readEntry<QByteArray>("Columns", QByteArray());
    header()->restoreState(state);
}


UpdateView::~UpdateView()
{
    KConfigGroup cg(&m_partConfig, "UpdateView");
    cg.writeEntry("Columns", header()->saveState());
}


void UpdateView::setFilter(Filter filter)
{
    filt = filter;

    if (UpdateDirItem* item = static_cast<UpdateDirItem*>(topLevelItem(0)))
    {
        ApplyFilterVisitor applyFilterVisitor(filter);
        item->accept(applyFilterVisitor);
    }
}


UpdateView::Filter UpdateView::filter() const
{
    return filt;
}


// returns true iff exactly one UpdateFileItem is selected
bool UpdateView::hasSingleSelection() const
{
    const QList<QTreeWidgetItem *>& listSelectedItems(selectedItems());

    return (listSelectedItems.count() == 1) && isFileItem(listSelectedItems.first());
}


void UpdateView::getSingleSelection(QString *filename, QString *revision) const
{
    const QList<QTreeWidgetItem *>& listSelectedItems(selectedItems());

    QString tmpFileName;
    QString tmpRevision;
    if ((listSelectedItems.count() == 1) && isFileItem(listSelectedItems.first()))
    {
        UpdateFileItem* fileItem(static_cast<UpdateFileItem*>(listSelectedItems.first()));
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

    const QList<QTreeWidgetItem *>& listSelectedItems(selectedItems());
    foreach (QTreeWidgetItem *item, listSelectedItems)
    {
        if (!item->isHidden())
            res.append(static_cast<UpdateItem*>(item)->filePath());
    }

    return res;
}


QStringList UpdateView::fileSelection() const
{
    QStringList res;

    const QList<QTreeWidgetItem *>& listSelectedItems(selectedItems());
    foreach (QTreeWidgetItem *item, listSelectedItems)
    {
        if (isFileItem(item) && !item->isHidden())
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
void UpdateView::replaceItem(QTreeWidgetItem *oldItem,
                             QTreeWidgetItem *newItem)
{
    const int index(relevantSelection.indexOf(oldItem));
    if (index >= 0)
        relevantSelection.replace(index, newItem);
}


void UpdateView::unfoldSelectedFolders()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    int previousDepth = 0;
    bool isUnfolded = false;

    QStringList selection = multipleSelection();

    // setup name of selected folder
    QString selectedItem = selection.first();
    if( selectedItem.contains('/') )
        selectedItem.remove(0, selectedItem.lastIndexOf('/')+1);

    // avoid flicker
    const bool _updatesEnabled = updatesEnabled();
    setUpdatesEnabled(false);

    QTreeWidgetItemIterator it(this);
    while ( QTreeWidgetItem *item = (*it) )
    {
        if ( isDirItem(item) )
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
                isUnfolded = dirItem->isExpanded();

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

    setUpdatesEnabled(_updatesEnabled);
    viewport()->update();

    QApplication::restoreOverrideCursor();
}


void UpdateView::unfoldTree()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_unfoldingTree = true;

    const bool _updatesEnabled = updatesEnabled();

    setUpdatesEnabled(false);

    QTreeWidgetItemIterator it(this);
    while (QTreeWidgetItem *item = (*it) )
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

    setUpdatesEnabled(_updatesEnabled);

    viewport()->update();

    m_unfoldingTree = false;

    QApplication::restoreOverrideCursor();
}


void UpdateView::foldTree()
{
    QTreeWidgetItemIterator it(this);
    while (QTreeWidgetItem* item = (*it) )
    {
        // don't close the top level directory
        if (isDirItem(item) && item->parent())
            item->setExpanded(false);

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
    item->setExpanded(true);
    setCurrentItem(item);
    item->setSelected(true);
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
        static_cast<UpdateDirItem*>(topLevelItem(0))->maybeScanDir(true);

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
    // ... which is not correct (e.g. server not reachable also returns 1)
    const bool success(normalExit && (exitStatus == 0));

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
    foreach (QTreeWidgetItem* it, relevantSelection)
    {
        if (isDirItem(it))
        {
            for (int i = 0; i < it->childCount(); i++)
            {
                QTreeWidgetItem *item = it->child(i);
                if (isFileItem(item))
                {
                    UpdateFileItem* fileItem = static_cast<UpdateFileItem*>(item);
                    fileItem->markUpdated(laststage, success);
                }
            }
        }
        else
        {
            UpdateFileItem* fileItem = static_cast<UpdateFileItem*>(it);
            fileItem->markUpdated(laststage, success);
        }
    }
}


/**
 * Remember the selection, see prepareJob()
 */
void UpdateView::rememberSelection(bool recursive)
{
    std::set<QTreeWidgetItem*> setItems;
    for (QTreeWidgetItemIterator it(this); *it; ++it)
    {
        QTreeWidgetItem* item(*it);

        // if this item is selected and if it was not inserted already
        // and if we work recursive and if it is a dir item then insert
        // all sub dirs
        // DON'T CHANGE TESTING ORDER
        if (item->isSelected()
            && setItems.insert(item).second
            && recursive
            && isDirItem(item))
        {
            QStack<QTreeWidgetItem *> s;
            int childNum = 0;
            QTreeWidgetItem *startItem = item;
            QTreeWidgetItem *childItem = startItem->child(childNum);
            while ( childItem )
            {
                // if this item is a dir item and if it was not
                // inserted already then insert all sub dirs
                // DON'T CHANGE TESTING ORDER
                if (isDirItem(childItem) && setItems.insert(childItem).second)
                {
                    if (QTreeWidgetItem *childChildItem = childItem->child(0))
                        s.push(childChildItem);
                }

                if ( ++childNum < startItem->childCount() )
                    childItem = startItem->child(childNum);
                else
                {
                    if ( s.isEmpty() )
                        break;
                    else
                    {
                        childItem = s.pop();
                        startItem = childItem->parent();
                        childNum = 0;
                    }
                }
            }
        }
    }

    // Copy the set to the list
    relevantSelection.clear();
    std::set<QTreeWidgetItem *>::const_iterator const itItemEnd = setItems.end();
    for (std::set<QTreeWidgetItem *>::const_iterator itItem = setItems.begin();
         itItem != itItemEnd; ++itItem)
        relevantSelection.append(*itItem);

#if 0
    qDebug() << "Relevant:";
    foreach (QTreeWidgetItem * item, relevantSelection)
        qDebug() << "  " << item->text(0);
    qDebug() << "End";
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
    foreach (QTreeWidgetItem* item, relevantSelection)
    {
        UpdateDirItem *dirItem(0);
        if (isDirItem(item))
            dirItem = static_cast<UpdateDirItem*>(item);
        else if (QTreeWidgetItem *parentItem = item->parent())
            dirItem = static_cast<UpdateDirItem*>(parentItem);

        if (dirItem)
            setDirItems.insert(dirItem);
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

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
    KConfigGroup cs(&m_partConfig, "Colors");

    m_conflictColor = cs.readEntry("Conflict", QColor(255, 130, 130) );
    m_localChangeColor = cs.readEntry("LocalChange", QColor(130, 130, 255));
    m_remoteChangeColor = cs.readEntry("RemoteChange", QColor(70, 210, 70) );

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
        switch (str[0].toLatin1())
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

    const QString removedFileStart(QLatin1String("cvs server: "));
    const QString removedFileEnd(QLatin1String(" is no longer in the repository"));
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
    if (isdir && filePath == QLatin1String("."))
        return;

    const QFileInfo fileInfo(filePath);

    UpdateDirItem* rootItem = static_cast<UpdateDirItem*>(topLevelItem(0));
    UpdateDirItem* dirItem = findOrCreateDirItem(fileInfo.path(), rootItem);

    dirItem->updateChildItem(fileInfo.fileName(), status, isdir);
}


void UpdateView::itemExecuted(QTreeWidgetItem *item, int)
{
    if (isFileItem(item))
        emit fileOpened(static_cast<UpdateFileItem*>(item)->filePath());
}

void UpdateView::itemExpandedSlot(QTreeWidgetItem *item)
{
    static_cast<UpdateItem *>(item)->setOpen(true);
}


// Local Variables:
// c-basic-offset: 4
// End:
