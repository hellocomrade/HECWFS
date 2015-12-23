/***************************************************************************
 *   Copyright (C)  *
 *     *
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
#ifndef HECWFS_OGRHELPER_H
#define HECWFS_OGRHELPER_H

#include <math.h>
#include "ogr_api.h"
#include "ogr_srs_api.h"

#define HECWFS_ZERO (9e-9)
#ifndef PI
#define PI (3.141592653589793)
#endif

#ifdef __cplusplus
extern "C"{
#endif

char createGrid(const char *pszOutputFile,float *pfUa,float *pfVa,size_t elem_size,float *pfX,float *pfY,float *pfNv,size_t node_size);
char createPointLayer(const char *pszOutputFile,float *pfUa,float *pfVa,float * pfWl,size_t elem_size, float *pfX, float *pfY,float *pfNv,size_t node_size,char *pszTimestamp);

#ifdef __cplusplus
}
#endif

#endif
