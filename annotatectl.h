/* 
 *  Copyright (c) 2002 Christian Loose <christian.loose@hamburg.de>
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

    void showDialog(const QString& fileName, const QString& revision = QString::null);

private:
    struct Private;
    Private* d;
};


#endif
