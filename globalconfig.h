/*
 *  Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H


/*
 * This classes manages global configuration data of Cervisia. It
 * uses the monostate design pattern.
 */
class GlobalConfig
{
public:
    unsigned timeOut() const { return mTimeOut; }
    void setTimeOut(unsigned timeOut) { mTimeOut = timeOut; }

private:
    static unsigned mTimeOut;
};


#endif
