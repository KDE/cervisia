/*
 * Copyright (c) 2004-2005 André Wöbbeking <Woebbeking@web.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef CERVISIA_TOOLTIP_H
#define CERVISIA_TOOLTIP_H


#include <qobject.h>
#include <qtooltip.h>


namespace Cervisia
{


/**
 * This class extends QToolTip:
 * - no more need to subclass just connect to the signal queryToolTip()
 * - truncate too large tooltip texts.
 */
class ToolTip : public QObject, public QToolTip
{
    Q_OBJECT

public:

    /**
     * @param widget The widget you want to add tooltips to. It's also used as
     * parent for the QObject. So you don't have to free an instance of this
     * class yourself.
     */
    explicit ToolTip(QWidget* widget);

signals:

    /**
     * This signal is emitted when a tooltip could be displayed. When a client
     * wants to display anythink it must set a valid tooltip rectangle and a
     * non empty text.
     *
     * @param pos The position of the tooltip in the parent widget's coordinate system.
     *
     * @param rect The rectangle in the parent widget's coordinate system where the
     * tooltip is valid.
     *
     * @param text The tooltip text.
     */
    void queryToolTip(const QPoint& pos, QRect& rect, QString& text);

protected:

    virtual void maybeTip(const QPoint&);
};


} // namespace Cervisia


#endif // CERVISIA_TOOLTIP_H
