/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef MISC_H
#define MISC_H


class QString;
class QStringList;
class QWidget;
class KConfig;
class CvsService_stub;


QString joinLine(const QStringList &list);
QStringList splitLine(QString, char delim=' ');

QString userName();
QString tempFileName(const QString &suffix);
void cleanupTempFiles();

bool isValidTag(const QString &str);
QString cvsClient(const QString &sRepository, KConfig* config);

const QStringList fetchBranches(CvsService_stub* cvsService, QWidget* parent);
const QStringList fetchTags(CvsService_stub* cvsService, QWidget* parent);

/**
 * Compares two revision numbers.
 *
 * @return -1 / 0 / 1 if rev1 is < / == / > rev2
 */
int compareRevisions(const QString& rev1, const QString& rev2);


/**
 * Generic compare for two objects of the same class. operator<() must
 * be defined for this class.
 *
 * @return -1 / 0 / 1 if lhs is < / == / > rhs
 */
template<class C>
int compare(const C& lhs, const C& rhs)
{
    if (lhs < rhs)
        return -1;
    else if (rhs < lhs)
        return 1;
    else
        return 0;
}


#endif


// Local Variables:
// c-basic-offset: 4
// End:
