/*
 * Copyright (c) 2004-2005 Andr� W�bbeking <Woebbeking@web.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "tooltip.h"

#include <kglobal.h>
#include <kglobalsettings.h>

#include <qsimplerichtext.h>


namespace Cervisia
{


static QString truncateLines(const QString&, const QFontMetrics&, const QSize&);
static QString truncateLines(const QString&, const QFont&, const QPoint&, const QRect&);


ToolTip::ToolTip(QWidget* widget)
    : QObject(widget), QToolTip(widget)
{
}


void ToolTip::maybeTip(const QPoint& pos)
{
    QRect rect;
    QString text;
    emit queryToolTip(pos, rect, text);

    if (rect.isValid() && !text.isEmpty())
    {
        text = truncateLines(text,
                             font(),
                             parentWidget()->mapToGlobal(pos),
                             KGlobalSettings::desktopGeometry(parentWidget()));
        tip(rect, text);
    }
}


// Primtive routine to truncate the text. size.width() is ignored, only
// size.height() is used at the moment to keep it fast. It doesn't work
// correct if text lines have different heights.
QString truncateLines(const QString&      text,
                      const QFontMetrics& fm,
                      const QSize&        size)
{
    const QChar newLine('\n');

    const int lineSpacing(fm.lineSpacing());
    const int numberOfLines(text.contains(newLine) + 1);
    const int maxNumberOfLines(size.height() / lineSpacing);

    if (numberOfLines <= maxNumberOfLines)
        return text;

    const QChar* unicode(text.unicode());
    for (int count(maxNumberOfLines); count; ++unicode)
        if (*unicode == newLine)
            --count;

    return text.left(unicode - text.unicode() - 1);
}


// Truncate the tooltip's text if necessary
QString truncateLines(const QString& text,
                      const QFont&   font,
                      const QPoint&  globalPos,
                      const QRect&   desktopGeometry)
{
    // maximum size of the tooltip, - 10 just to be safe
    const int maxWidth(kMax(desktopGeometry.width() - globalPos.x(), globalPos.x())
                       - desktopGeometry.left() - 10);
    const int maxHeight(kMax(desktopGeometry.height() - globalPos.y(), globalPos.y())
                       - desktopGeometry.top() - 10);

    // calculate the tooltip's size
    const QSimpleRichText layoutedText(text, font);

    // only if the tooltip's size is bigger in x- and y-direction the text must
    // be truncated otherwise the tip is moved to a position where it fits
    return  ((layoutedText.widthUsed() > maxWidth)
             && (layoutedText.height() > maxHeight))
        ? truncateLines(text, QFontMetrics(font), QSize(maxWidth, maxHeight))
        : text;
}


} // namespace Cervisia


#include "tooltip.moc"
