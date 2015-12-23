/***************************************************************************
 *   Copyright (C)  *
 *    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#define MAXERRMSG 256


static void _printErr(const char *fmt,va_list ap,int errNoFlag)
{
    char buf[MAXERRMSG];
    int errno_save=errno;
    vsnprintf(buf,MAXERRMSG,fmt,ap);
    if(errNoFlag)
        snprintf(buf+strlen(buf),MAXERRMSG-strlen(buf),": %s",strerror(errno_save));
    if(strlen(buf)<MAXERRMSG-1)
        strcat(buf,"\n");
    fflush(stdout);
    fputs(buf,stderr);
    fflush(stderr);
}
void printError(int errNoFlag,const char *fmt,...)
{
    va_list ap;
    va_start(ap,fmt);
    _printErr(fmt,ap,errNoFlag);
    va_end(ap);
}
