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


class QCString;
class QString;
class QStringList;
class QTextCodec;
class QWidget;
class CvsService_stub;


void chomp(QCString *line);
QString joinLine(const QStringList &list);
QStringList splitLine(QString, char delim=' ');
int findWhiteSpace(QString const& rsString, int iStartIndex = 0);

QString userName();
QString tempFileName(const QString &suffix);
void cleanupTempFiles();
QTextCodec *detectCodec(const QString &fileName);

bool isValidTag(const QString &str);
QString cvsClient(const QString &sRepository);
QStringList const fetchBranches(CvsService_stub* cvsService,
                                QWidget*       pParentWidget);
QStringList const fetchTags(CvsService_stub* cvsService,
                            QWidget*       pParentWidget);


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
