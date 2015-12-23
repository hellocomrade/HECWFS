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
#ifndef HECWFS_NCHELPER_H
#define HECWFS_NCHELPER_H

#include <string.h>
#include <stdlib.h>
#include "netcdf.h"

#define HECWFS_NC_NAME (16)
#ifdef __cplusplus
extern "C"{
#endif
struct _var_Info;
struct _nc_info;
typedef struct _dimInfo
{
    char name[HECWFS_NC_NAME];
    size_t size;
    int id;
    int start;
    int count;
}NC_DimInfo;
typedef struct _var_Info* NC_Variable;

typedef struct _nc_info* NC_Handler;

NC_Handler NC_Open4Read(const char *file);
void NC_Close(NC_Handler pNcInfo);
void NC_SetDimName(NC_DimInfo *pDimInfo,const char *pszName);
char NC_ReadDimInfo(NC_DimInfo *pDimInfo,NC_Handler pNcInfo);
NC_Variable NC_DefineVariable(const char *pszName,NC_Handler pNcInfo,NC_DimInfo *pDimInfo,size_t dim_len);
void NC_DestroyVariable(NC_Variable pVar);
char NC_ReadFloatArray(NC_Handler pNcInfo,NC_Variable pVarInfo,float *pfVars);
#ifdef __cplusplus
}
#endif
#endif
