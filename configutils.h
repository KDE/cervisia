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

#ifndef CERVISIA_CONFIGUTILS_H
#define CERVISIA_CONFIGUTILS_H

class QDialog;
class QString;
class QSize;
class KConfig;


namespace Cervisia
{


/**
 * Copy&Paste from KDialogBase for KDE 3.1 support
 *
 * restore the dialogs size from the configuration according to the screen size.
 * If no size is saved for one dimension of the screen, a default size
 * is choosed. The default width is 50 percent of the screen width, the
 * default height is 40 percent of the screen height.
 *
 * @param groupName Name of the group to read from. The old group
 *                  of KGlobal::config is preserved.
 */
QSize configDialogSize(QDialog* dialog, KConfig& config, const QString& groupName);

/**
 * Copy&Paste from KDialogBase for KDE 3.1 support
 *
 * save the dialogs size dependant on the screen dimension either to
 * CervisiaPart config file.
 *
 * @param The group to which the dialogs size is saved. See @ref restoreDialogSize
 * to restore the size.
 */
void saveDialogSize(QDialog* dialog, KConfig& config, const QString& groupName);


} // namespace Cervisia

#endif

