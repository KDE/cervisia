/* 
 *  Copyright (c) 2004 Christian Loose <christian.loose@hamburg.de>
 *
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef LOGTREETOOLTIP_H
#define LOGTREETOOLTIP_H

#include <qtooltip.h>

class QPoint;
class QTable;


namespace Cervisia
{

class LogTreeToolTip : public QToolTip
{  
public:
    LogTreeToolTip(QTable* logTreeView);

protected:
    void maybeTip(const QPoint& p);
    
private:
    QTable* m_logTreeView;
};

}


#endif
