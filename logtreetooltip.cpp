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


#include "logtreetooltip.h"

#include <qpoint.h>
#include <qtable.h>

using namespace Cervisia;


LogTreeToolTip::LogTreeToolTip(QTable* logTreeView)
    : QToolTip(logTreeView->viewport())
    , m_logTreeView(logTreeView)
{
}


void LogTreeToolTip::maybeTip(const QPoint& p)
{
    QPoint cp = m_logTreeView->viewportToContents(p);
    int row = m_logTreeView->rowAt(cp.y());
    int col = m_logTreeView->columnAt(cp.x());
    
    QString tipText = m_logTreeView->text(row, col);
    
    if( tipText.isNull() )
        return;
    
    QRect cr = m_logTreeView->cellGeometry(row, col);
    
    cr.moveTopLeft(m_logTreeView->contentsToViewport(cr.topLeft()));
    tip(cr, tipText); 
}
