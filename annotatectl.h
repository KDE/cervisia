/* 
 *  Copyright (c) 2002 Christian Loose <christian.loose@hamburg.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef ANNOTATECTL_H
#define ANNOTATECTL_H

#include <qstring.h>

class AnnotateDialog;
class CvsService_stub;
class QWidget;


class AnnotateController
{
public:
    AnnotateController(AnnotateDialog* dialog, CvsService_stub* cvsService);
    ~AnnotateController();

    void showDialog(const QString& fileName, const QString& revision = "");

private:
    struct Private;
    Private* d;
};


#endif
