/*
 *  Copyright (c) 2003 André Wöbbeking <Woebbeking@web.de>
 *  Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "configutils.h"

#include <qapplication.h>
#include <qdialog.h>
#include <kconfig.h>



QSize Cervisia::configDialogSize(QDialog* dialog, KConfig& config, const QString& groupName)
{
   int w, h;

   int scnum  = QApplication::desktop()->screenNumber(dialog);
   QRect desk = QApplication::desktop()->screenGeometry(scnum);

   w = QMIN( 530, (int) (desk.width() * 0.5)); // maximum default width = 530
   h = (int) (desk.height() * 0.4);

   KConfigGroupSaver cs(&config, groupName);
   w = config.readNumEntry( QString::fromLatin1("Width %1").arg( desk.width()), w );
   h = config.readNumEntry( QString::fromLatin1("Height %1").arg( desk.height()), h );

   return( QSize( w, h ) );
}



void Cervisia::saveDialogSize(QDialog* dialog, KConfig& config, const QString& groupName)
{
   int scnum  = QApplication::desktop()->screenNumber(dialog);
   QRect desk = QApplication::desktop()->screenGeometry(scnum);

   KConfigGroupSaver cs(&config, groupName);
   QSize sizeToSave = dialog->size();

   config.writeEntry( QString::fromLatin1("Width %1").arg( desk.width()),
                      QString::number( sizeToSave.width()), true);
   config.writeEntry( QString::fromLatin1("Height %1").arg( desk.height()),
                      QString::number( sizeToSave.height()), true);
}
