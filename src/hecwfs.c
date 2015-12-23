/***************************************************************************
 *   Copyright (C) *
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "gdal.h"

#include "nchelper.h"
#include "ww3_dayofmonth.h"

#define HECWFS_PERIOD 3
#define HECWFS_NC_SHP_LIST "/tmp/hecwfs_shp_lst"

typedef enum boolean
{
  false,
  true
}BOOL;

static void usage()
{
    printf("usage:\n"
           "hecwfs -options\n"
           "[-v]: create velocity shp file.\n"
           "[-o] filepath and name: location of the velocity shp file.\n"
           "[-g] filepath and name: create grid shp file and the location of shp file.\n"
           "[-f] filepath and name: location of the netcdf file.\n"
           "[-y] year:\n"
           "[-m] month:\n"
           "[-d] day:\n"
           "[-h] hour:\n"
           "[-s] start hour index(0-2): hour starts from 0\n"
           "[-e] hours(1-3):\n"
           "[-l]: create a file /tmp/hecwfs_nowcast_shp_lst that lists shp files name.\n"
           "Created shp files list is at /tmp/hecwfs_shp_lst\n");
}
inline static void readDimInfo(NC_Handler hNetcdf,NC_DimInfo *pDimInfo,const char *pszName)
{
    char result=0;
    if(!(hNetcdf&&pDimInfo&&pszName))
        exit(-1);
    NC_SetDimName(pDimInfo,pszName);
    result=NC_ReadDimInfo(pDimInfo,hNetcdf);
    if(result)
    {
      NC_Close(hNetcdf);
      printError(0,"HECWFS: Read NetCDF File on Dimension '%s' Failed!\n",pszName);
      exit(-1);
    }
}
inline static NC_Variable getVariable(const char *pszName,NC_Handler pNcInfo,NC_DimInfo *pDimInfo,size_t dim_len)
{
    NC_Variable var=0;
    if(!(pszName&&pNcInfo&&pDimInfo&&dim_len>0))
        exit(-1);
    var=NC_DefineVariable(pszName,pNcInfo,pDimInfo,dim_len);
    if(!var)
    {
        NC_Close(pNcInfo);
        printError(0,"HECWFS: Read NetCDF File on Variable '%s' Failed!\n",pszName);
        exit(-1);
    }
    return var;
}
inline static void fillFloatArray(NC_Handler pNcInfo,NC_Variable pVarInfo,float *pfVars,const char *pszName)
{
    char result=0;
    if(!(pNcInfo&&pVarInfo&&pfVars))
        exit(-1);
    result=NC_ReadFloatArray(pNcInfo,pVarInfo,pfVars);
    if(result)
    {
        printError(0,"HECWFS: Read NetCDF File on Variable(float) '%s' Values Failed!\n",pszName);
        NC_DestroyVariable(pVarInfo);
        NC_Close(pNcInfo);
        exit(-1);
    }
}
inline static void* allocMem(size_t size)
{
    void *temp=NULL;
    temp=malloc(size);
    if(!temp)
    {
        printError(0,"HECWFS: %s\n","Failed to allocate memory!");
        exit(-1);
    }
    return temp;
}

int main(int argc, char *argv[])
{
  if(argc<5)
  {
      printError(0,"HECWFS: %s\n","not enough parameters!");
      exit(-1);
  }
  int c;
  int shour=0;
  int ehour=1;
  int year,month,day,hour;
  BOOL bV=false;
  BOOL bL=false;
  BOOL bG=false;
  char *pszTimestamp=NULL;
  char *pszFilePath=NULL;
  char *pszOutputFile=NULL;
  char *pszGridFile=NULL;
  NC_Handler hNetcdf=0;
  NC_Variable hUa=0,hVa=0,hNv=0,hX=0,hY=0,hWl=0;
  
  NC_DimInfo ncDims4V[3];
  NC_DimInfo ncDims4nv[2];
  NC_DimInfo ncDims4WL[2];
  NC_DimInfo ncDims4xy;
  
  float *pfUa=NULL;
  float *pfVa=NULL;
  float *pfNv=NULL;
  float *pfX=NULL;
  float *pfY=NULL;
  float *pfWl=NULL;
  int result=0;
  while((c=getopt(argc,argv,"g:vlt:f:o:s:e:y:m:d:h:"))!=-1)
  {
      switch(c)
      {
          case 'g'://Creat grid
              bG=true;
              pszGridFile=optarg;
          break;
          case 's':
              shour=atoi(optarg);
          break;
          case 'e':
              ehour=atoi(optarg);
          break;
          case 'y':
              year=atoi(optarg);
          break;
          case 'm':
              month=atoi(optarg);
          break;
          case 'd':
              day=atoi(optarg);
          break;
          case 'h':
              hour=atoi(optarg);
          break;
          case 'v'://create real data shp file
              bV=true;
          break;
          case 'l'://create shp file list
              bL=true;
          break;
          case 't'://timestamp
              pszTimestamp=optarg;
          break;
          case 'f'://source netcdf file
              pszFilePath=optarg;
          break;
          case 'o'://data output shp file
              pszOutputFile=optarg;
          break;
          case '?':
              usage();
              return 0;
          break;
          default:
              usage();
              return 0;
          break;
      }
  }
  if((bV==false&&bG==false)||!pszOutputFile||!pszFilePath||year<=0||month<=0||day<=0||hour<0)//||!pszTimestamp)
  {
      usage();
      return 0;
  }
  if(bG)
  {
      if(!pszGridFile)
      {
          usage();
          return 0;
      }
  }

  hNetcdf=NC_Open4Read(pszFilePath);
  if(!hNetcdf)
  {
      printError(0,"HECWFS: %s\n","Open NetCDF File Failed!");
      return -1;
  }
  
  //read dimension info: 'three', 'nele' for the variable: 'nv'
  readDimInfo(hNetcdf,&ncDims4nv[0],"three");
  readDimInfo(hNetcdf,&ncDims4nv[1],"nele");
  //read variables info: 'nv'
  hNv=getVariable("nv",hNetcdf,ncDims4nv,sizeof(ncDims4nv)/sizeof(NC_DimInfo));
  //allocate mem for variable: 'nv'
  pfNv=(float*)allocMem(sizeof(float)*(ncDims4nv[0].size)*(ncDims4nv[1].size));
  //define range to read for variables: 'nv'
  ncDims4nv[0].start=0;
  ncDims4nv[0].count=ncDims4nv[0].size;
  ncDims4nv[1].start=0;
  ncDims4nv[1].count=ncDims4nv[1].size;
  //read data for variables: 'nv'
  fillFloatArray(hNetcdf,hNv,pfNv,"nv");
  
  //read dimension info: 'node' for the varialbes: 'x', 'y'
  readDimInfo(hNetcdf,&ncDims4xy,"node");
  //read variables info: 'x', 'y'
  hX=getVariable("x",hNetcdf,&ncDims4xy,1);
  hY=getVariable("y",hNetcdf,&ncDims4xy,1);
  //allocate mem for variable: 'x', 'y'
  pfX=(float*)allocMem(sizeof(float)*ncDims4xy.size);
  pfY=(float*)allocMem(sizeof(float)*ncDims4xy.size);
  //define range to read for variables: 'x', 'y'
  ncDims4xy.start=0;
  ncDims4xy.count=ncDims4xy.size;
  fillFloatArray(hNetcdf,hX,pfX,"x");
  fillFloatArray(hNetcdf,hY,pfY,"y");
  /*printf("begin to sleep\n");
  sleep(10);
  printf("end sleep\n");*/
  
  if(bV)
  {
      //read dimension info: 'time', 'siglay', 'nele' for variables: 'u', 'v'
      readDimInfo(hNetcdf,&ncDims4V[0],"time");
      readDimInfo(hNetcdf,&ncDims4V[1],"siglay");
      readDimInfo(hNetcdf,&ncDims4V[2],"nele");
  
      //read variables info: 'a', 'a'
      hUa=getVariable("u",hNetcdf,ncDims4V,sizeof(ncDims4V)/sizeof(NC_DimInfo));
      hVa=getVariable("v",hNetcdf,ncDims4V,sizeof(ncDims4V)/sizeof(NC_DimInfo));
      
      //read dimension info: 'time', 'node' for variable: 'zeta'(water level)
      readDimInfo(hNetcdf,&ncDims4WL[0],"time");
      readDimInfo(hNetcdf,&ncDims4WL[1],"node");
      //read variable info: 'zeta'
      hWl=getVariable("zeta",hNetcdf,ncDims4WL,sizeof(ncDims4WL)/sizeof(NC_DimInfo));

      //Change to adopt forecast(48 hours)
      if(shour<0||shour>ncDims4V[0].size-1)//HECWFS_PERIOD-1)
          shour=0;
      if(ehour<1||ehour>ncDims4V[0].size)//HECWFS_PERIOD)
          ehour=1;
      
      int i=shour+1;
      int isLeap=isLeapYear(year);
      char acTimestamp[20];
      //20 for actimstamp 4 for ".shp" 1 for '\0' 
      int tLen=strlen(pszOutputFile)+1+20+4;
      char *pszVFile=NULL;
      FILE* fLst=NULL;
      pszVFile=(char*)malloc(tLen);
      if(pszVFile)
      {
	for(;i<=ehour;++i)//i is assigned to start hour before entering the loop
	{
		//hour is -1 of real start hour, eg. hour=12 means the real start hour is 13 and ends at 15
		getTimestamp(year,month,day,hour,i,isLeap,acTimestamp,sizeof(acTimestamp));
		snprintf(pszVFile,tLen,"%s%s.shp",pszOutputFile,acTimestamp);
                int j=0;
                //make sure the file name we created works on both unix and windows
                for(;j<strlen(pszVFile);++j)
                    if(pszVFile[j]==' ' || pszVFile[j]==':')
                        pszVFile[j]='_';
		//allocate mem for variables: 'u', 'v','zeta', 1 is for time dim
		if(pfUa==NULL)
		    pfUa=(float*)allocMem(sizeof(float)*1*(ncDims4V[2].size));
		if(pfVa==NULL)
		    pfVa=(float*)allocMem(sizeof(float)*1*(ncDims4V[2].size));
                if(pfWl==NULL)
                    pfWl=(float*)allocMem(sizeof(float)*1*(ncDims4WL[1].size));
		//define range to read for variables: 'ua', 'va', NC_Variable holds the pointer of NC_DimInfo, so the change will reflect on pfXX when you do the fillFloatArray.
		ncDims4V[0].start=i-1;
		ncDims4V[0].count=1;
                ncDims4V[1].start=0;
		ncDims4V[1].count=1;
		ncDims4V[2].start=0;
		ncDims4V[2].count=ncDims4V[2].size;
		//define range to read for variable: 'zeta'
		//read data for variables: 'ua', 'va'
		ncDims4WL[0].start=i-1;
		ncDims4WL[0].count=1;
		ncDims4WL[1].start=0;
		ncDims4WL[1].count=ncDims4WL[1].size;

		fillFloatArray(hNetcdf,hUa,pfUa,"u");
		fillFloatArray(hNetcdf,hVa,pfVa,"v");
	        fillFloatArray(hNetcdf,hWl,pfWl,"zeta");

		if(createPointLayer(pszVFile,pfUa,pfVa,pfWl,ncDims4V[2].size,pfX,pfY,pfNv,ncDims4xy.size,acTimestamp)<0)
		    printError(0,"HECWFS: %s\n","Create Point shp File Failed!");
                else
                {
                    if(bL)
                    {
		        if(!fLst)
                        {
                            fLst=fopen(HECWFS_NC_SHP_LIST,"w");
                        }
                        fputs(pszVFile,fLst);
                        fputc('\n',fLst);
                    }
                }
	}
        if(fLst)
            fclose(fLst);
        free(pszVFile);
     }
  }
  if(bG)
     if(createGrid(pszGridFile,pfUa,pfVa,ncDims4V[2].size,pfX,pfY,pfNv,ncDims4xy.size)<0)
          printError(0,"HECWFS: %s\n","Create Grid shp File Failed!");

  //not really necessary to free mem if the process does end soon
  free(pfY);
  free(pfX);
  free(pfNv);
  free(pfVa);
  free(pfUa);
  
  NC_DestroyVariable(hY);
  NC_DestroyVariable(hX);
  NC_DestroyVariable(hNv);
  NC_DestroyVariable(hUa);
  NC_DestroyVariable(hVa);
  NC_Close(hNetcdf);
  return EXIT_SUCCESS;
}
