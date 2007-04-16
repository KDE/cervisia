/*
 *  Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
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


#include "watchersdialog.h"

#include <qlayout.h>
#include <q3table.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QBoxLayout>
#include <kconfig.h>
#include <klineedit.h>
#include <klocale.h>
#include <kconfiggroup.h>

#include "misc.h"
#include "cvsserviceinterface.h"
#include "progressdialog.h"


WatchersDialog::WatchersDialog(KConfig& cfg, QWidget* parent, const char* name)
    : KDialog(parent)
    , partConfig(cfg)
{
    setButtons(Close);
    showButtonSeparator(true);

    QFrame* mainWidget = new QFrame(this);
    setMainWidget(mainWidget);

    QBoxLayout *layout = new QVBoxLayout(mainWidget);
    layout->setSpacing(spacingHint());
    layout->setMargin(0);

    table = new Q3Table(mainWidget, "watchersTable");
    table->setNumCols(5);
    table->setSelectionMode(Q3Table::NoSelection);
    table->setColumnMovingEnabled(false);
    table->setRowMovingEnabled(false);
    table->setReadOnly(true);
    table->setDragEnabled(false);
    table->setSorting(true);
    table->verticalHeader()->hide();
    table->setLeftMargin(0);

    Q3Header* header = table->horizontalHeader();
    header->setLabel(0, i18n("File"));
    header->setLabel(1, i18n("Watcher"));
    header->setLabel(2, i18n("Edit"));
    header->setLabel(3, i18n("Unedit"));
    header->setLabel(4, i18n("Commit"));

    layout->addWidget(table, 1);

    setAttribute(Qt::WA_DeleteOnClose, true);

    KConfigGroup cg(&partConfig, "WatchersDialog");
    restoreDialogSize(cg);
}


WatchersDialog::~WatchersDialog()
{
    KConfigGroup cg(&partConfig, "WatchersDialog");
    saveDialogSize(cg);
}


bool WatchersDialog::parseWatchers(OrgKdeCervisiaCvsserviceCvsserviceInterface* cvsService,
                                   const QStringList& files)
{
    setCaption(i18n("CVS Watchers"));

    QDBusReply<QDBusObjectPath> job = cvsService->watchers(files);
    if( !job.isValid() )
        return false;

    ProgressDialog dlg(this, "Watchers",cvsService->service(), job, "watchers", i18n("CVS Watchers"));
    if( !dlg.execute() )
        return false;

    QString line;
    int numRows = 0;
    while( dlg.getLine(line) )
    {
        // parse the output line
        QStringList list = splitLine(line);

        // ignore empty lines and unknown files
        if( list.isEmpty() || list[0] == "?" )
            continue;

        // add a new row to the table
        table->setNumRows(numRows + 1);

        table->setText(numRows, 0, list[0]);
        table->setText(numRows, 1, list[1]);

        Q3CheckTableItem* item = new Q3CheckTableItem(table, "");
        item->setChecked(list.contains("edit"));
        table->setItem(numRows, 2, item);

        item = new Q3CheckTableItem(table, "");
        item->setChecked(list.contains("unedit"));
        table->setItem(numRows, 3, item);

        item = new Q3CheckTableItem(table, "");
        item->setChecked(list.contains("commit"));
        table->setItem(numRows, 4, item);

        ++numRows;
    }

    return true;
}
