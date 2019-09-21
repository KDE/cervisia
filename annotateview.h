/*
 * Copyright (C) 1999-2002 Bernd Gehrmann <bernd@mail.berlios.de>
 * Copyright (c) 2003-2008 André Wöbbeking <Woebbeking@kde.org>
 * Copyright (c) 2015 Martin Koller <kollix@aon.at>
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


#ifndef ANNOTATEVIEW_H
#define ANNOTATEVIEW_H

#include <QTreeWidget>
#include <QStyledItemDelegate>

namespace Cervisia
{
struct LogInfo;
}


class AnnotateView : public QTreeWidget
{
    Q_OBJECT

public:
    explicit AnnotateView(QWidget *parent);

    void addLine(const Cervisia::LogInfo& logInfo, const QString& content, bool odd);

    QSize sizeHint() const override;

    int currentLine() const;
    int lastLine() const;
    void gotoLine(int line);

public slots:
    void findText(const QString &textToFind, bool up);

private slots:
    void configChanged();

    void slotQueryToolTip(const QPoint&, QRect&, QString&);
};

class AnnotateViewDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit AnnotateViewDelegate(AnnotateView *v) : view(v) { }
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    enum { BORDER = 4 };
    AnnotateView *view;
};

#endif
