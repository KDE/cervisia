/*
 * Copyright (C) 1999-2002 Bernd Gehrmann <bernd@mail.berlios.de>
 * Copyright (c) 2003-2004 André Wöbbeking <Woebbeking@web.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef ANNOTATEVIEW_H
#define ANNOTATEVIEW_H


#include <qlistview.h>


class KConfig;


namespace Cervisia
{
struct LogInfo;
}


class AnnotateView : public QListView
{
    Q_OBJECT

public:

    explicit AnnotateView( KConfig &cfg, QWidget *parent=0, const char *name=0 );

    void addLine(const Cervisia::LogInfo& logInfo, const QString& content,
                 bool odd);

    virtual QSize sizeHint() const;

private slots:

    void slotQueryToolTip(const QPoint&, QRect&, QString&);
};


#endif
