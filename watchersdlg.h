/* 
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


#ifndef WATCHERSDLG_H
#define WATCHERSDLG_H


#include <kdialogbase.h>

class QTable;
class KConfig;
class CvsService_stub;


class WatchersDialog : public KDialogBase
{
public:
    explicit WatchersDialog(KConfig& cfg, QWidget* parent = 0,
                            const char* name = 0);
    virtual ~WatchersDialog();

    bool parseWatchers(CvsService_stub* cvsService, const QStringList& files);

private:
    QTable*  table;
    KConfig& partConfig;
};

#endif
