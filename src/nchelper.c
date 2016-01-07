/***************************************************************************
 *   Copyright (C)  by  *
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
#include "nchelper.h"

struct _nc_info
{
    int ncid;
};
struct _var_Info
{
    char name[HECWFS_NC_NAME];
    int id;
    NC_DimInfo *pDims;
    int dim_count;
    size_t *start;
    size_t *count;
};
/*
*Adapter functions between nc error and UNIX errno
*
*@params:
*          errno:  netcdf error number, NOT POSIX errno
*          msg:    user defined error message
*/
inline static void print_nc_error(int err,const char *msg)
{
    printError(err!=0?1:0,"NETCDF Error %d: %s\n",err,msg);
}
inline static void print_nc_error1(int err,const char *msg,const char *msg1)
{
    printError(err!=0?1:0,"NETCDF Error %d: %s %s\n",err,msg,msg1);
}
/*
*Open a netcdf file
*
*@params:
*           file:  file path and name of the netcdf file
*@return:
*           a _nc_info struct pointer that is redefined as NC_Handler to public
*/
NC_Handler NC_Open4Read(const char *file)
{
    struct _nc_info *nc=NULL;
    if(file)
    {
        int tmp=-1;
        int err1=NC_NOERR;
        err1=nc_open(file,0,&tmp);
        if(err1!=NC_NOERR)
            print_nc_error(err1,"Can not open the file!");
        else
        {
            nc=(struct _nc_info*)malloc(sizeof(struct _nc_info));
            if(!nc)
                printError(1,"NETCDF Error: %s","Can not allocate memory in NC_Open4Read, NULL returned from malloc!");
            else
                nc->ncid=tmp;
        }
    }
    return nc;
}
/*
*Close a netcdf file
*
*@params:
*           pNcInfo:  the handler representing the pointer of _nc_info struct
*@return:
*           none
*/
void NC_Close(NC_Handler pNcInfo)
{
    if(pNcInfo)
    {
        int err=NC_NOERR;
        err=nc_close(pNcInfo->ncid);
        if(err!=NC_NOERR)
            print_nc_error(err,"Can not close the file!");
        free(pNcInfo);
    }
}
/*
*Setter for a dimension's name. Initialize the memebers of NC_DimInfo struct
*including name,start, and count
*
*@params:
*          pDimInfo:  pointer to a NC_DimInfo struct
*
*/
void NC_SetDimName(NC_DimInfo *pDimInfo,const char *pszName)
{
    if(pDimInfo&&pszName)
    {
        memset(pDimInfo->name,0,HECWFS_NC_NAME);
        strncpy(pDimInfo->name,pszName,HECWFS_NC_NAME-1);
        pDimInfo->start=0;
        pDimInfo->count=0;
    }
}
/*
*Read a dimension info by its name from a nectcdf file. Even an error occurs, it
*won't stop execute but returning -1 stands for error
*
*@params:
*          pDimInfo:  a pointer to an initialized NC_DimInfo struct
*          pNcInfo:   a NC_Handler (the pointer of _nc_info)
*                     representing a netcdf file
*
*@return:
*          0:  no error
*          -1: error occurs
*/
char NC_ReadDimInfo(NC_DimInfo *pDimInfo,NC_Handler pNcInfo)
{
    char result=-1;
    if(pDimInfo&&pNcInfo)
    {
        int err=NC_NOERR;
        err=nc_inq_dimid(pNcInfo->ncid,pDimInfo->name,&pDimInfo->id);
        if(NC_NOERR!=err)
            print_nc_error1(err,"Can not query dimension id for",pDimInfo->name);
        else
        {
           err=nc_inq_dimlen(pNcInfo->ncid,pDimInfo->id,&pDimInfo->size);
           if(NC_NOERR!=err)
               print_nc_error1(err,"Can not query dimension length for",pDimInfo->name);
           else 
               result=0;
        }
    }
    return result;
}
/*
*Populate a variable's nid by its name. Initilize the members of _var_Info struct
*link all dimension infos to _var_Info 
*
*@params:
*          pszName:  variable name
*          pNcInfo:  NC_Handler (the pointer of _nc_info)
*          pDimInfo: a NC_DimInfo struct pointer to an array of dimensions metatdata
*          dim_len:  the length of dimensions
*
*@return:
*          a pointer to _var_Info struct that is redefined as NC_Variable to public
*/
NC_Variable NC_DefineVariable(const char *pszName,NC_Handler pNcInfo,NC_DimInfo *pDimInfo,size_t dim_len)
{
    struct _var_Info *var=NULL;
    int err=NC_NOERR;
    int temp;
    if(pNcInfo&&pszName&&pDimInfo&&dim_len>0)
    {
        err=nc_inq_varid(pNcInfo->ncid,pszName,&temp);
        if(NC_NOERR!=err)
            print_nc_error1(err,"Can not query variable id for",pszName);
        else
        {
            var=(struct _var_Info*)malloc(sizeof(struct _var_Info));
            if(!var)
                printError(1,"NETCDF Error: %s","Can not allocate memory in NC_DefineVariable, NULL returned from malloc!\n");
            else
            {
                memset(var->name,0,HECWFS_NC_NAME);
                strncpy(var->name,pszName,HECWFS_NC_NAME-1);
                var->id=temp;
                var->pDims=pDimInfo;
                var->dim_count=dim_len;
                var->start=NULL;
                var->count=NULL;
           }
        }
    }
    return var;
}
/*
*Destroy variable info struct
*
*@params:
*          pVar: a handler to NC_Variable that is a pointer to _var_Info struct
*/
void NC_DestroyVariable(NC_Variable pVar)
{
    if(pVar)
    {
        if(pVar->start)
            free(pVar->start);
        if(pVar->count)
            free(pVar->count);
        free(pVar);
    }
}
/*
*Populate real data values to an array
*
*@params:
*          pNcInfo:  the handler representing the pointer of _nc_info struct
*          pVarInfo:  a handler to NC_Variable that is a pointer to _var_Info struct
*          pfVars:  a float array as a container for data values
*
*@return:
*          0: no error
*         -1: error occurs
*/
char NC_ReadFloatArray(NC_Handler pNcInfo,NC_Variable pVarInfo,float *pfVars)
{
    char result=-1;
    if(pNcInfo&&pVarInfo&&pfVars)
    {
        int i;
        if(!pVarInfo->start)
        {
            pVarInfo->start=(size_t*)malloc(sizeof(size_t)*(pVarInfo->dim_count));
            if(!pVarInfo->start)
            {
                printError(1,"NETCDF Error: %s","Can not allocate memory in NC_ReadFloatArray, NULL returned from malloc!\n");
                return result;
            }
        }
        if(!pVarInfo->count)
        {
            pVarInfo->count=(size_t*)malloc(sizeof(size_t)*(pVarInfo->dim_count));
            if(!pVarInfo->count)
            {
                printError(1,"NETCDF Error: %s","Can not allocate memory in NC_ReadFloatArray, NULL returned from malloc!\n");
                return result;
            }
        }
        for(i=0;i<pVarInfo->dim_count;++i)
        {
            pVarInfo->start[i]=pVarInfo->pDims[i].start;
            pVarInfo->count[i]=pVarInfo->pDims[i].count;
        }
        i=NC_NOERR;
        i=nc_get_vara_float(pNcInfo->ncid,pVarInfo->id,pVarInfo->start,pVarInfo->count,pfVars);
        if(NC_NOERR!=i)
            print_nc_error1(i,"Can not read values for variable(float)",pVarInfo->name);
        else
            result=0;
    }
    return result;
}
