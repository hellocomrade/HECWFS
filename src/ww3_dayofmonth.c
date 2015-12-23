/***************************************************************************
 *   Copyright (C) *
 *   *
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
#include "ww3_dayofmonth.h"

int aiDofM[]={0,31,28,31,30,31,30,31,31,30,31,30,31};
int aiLeapDofM[]={0,31,29,31,30,31,30,31,31,30,31,30,31};
char *pszFmtS="0%d";
char *pszFmtD="%d";
char *pszFmtTemp="%%d-%s-%s %s:00:00";

int isLeapYear(int year)
{
  return (year%400==0)||(year%4==0&&year%100!=0);
}

void getTimestamp(int year,int month,int day,int hour,int offset,int isLeap,char* buf,int size)
{
  int *piDofM=NULL;
  if(size<14)return;
  if(hour<0||hour>23)return;
  if(month<1||month>12)return;
  if(isLeap)
  {
    if(aiLeapDofM[month]<day)
      return;
    piDofM=aiLeapDofM;
  }
  else
  {
    if(aiDofM[month]<day)
      return;
    piDofM=aiDofM;
  }
  char *pszMonthFmt;
  char *pszDayFmt;
  char *pszHourFmt;
  char pszFmt[32];
  int y,m,d,h;
  memset(pszFmt,0,32);
  memset(buf,0,size);
  
  h=(hour+offset)%24;
  if(h<0)//handle the minus offset situation
  {
      h=24+h;
      d=day+(hour+offset-23)/24;
  }
  else
      d=day+(hour+offset)/24;
  m=month;
  y=year;
  while(d>piDofM[m])
  {
    d-=piDofM[m++];
    if(m>12)//can not be a value of 14,15...
    {
      m=1;
      ++y;
      isLeap=isLeapYear(y);
      piDofM=isLeap?aiLeapDofM:aiDofM;
    }
  }
  if(m<10)
    pszMonthFmt=pszFmtS;
  else
    pszMonthFmt=pszFmtD;
  if(d<10)
    pszDayFmt=pszFmtS;
  else
    pszDayFmt=pszFmtD;
  if(h<10)
    pszHourFmt=pszFmtS;
  else
    pszHourFmt=pszFmtD;
  //snprintf(pszFmt,20,"%%d-%s-%sT%s:00:00",pszMonthFmt,pszDayFmt,pszHourFmt);
  int len=strlen(pszMonthFmt)+strlen(pszDayFmt)+strlen(pszHourFmt)+strlen(pszFmtTemp)+1;
  snprintf(pszFmt,len,pszFmtTemp,pszMonthFmt,pszDayFmt,pszHourFmt);
  snprintf(buf,size,pszFmt,y,m,d,h);
}
