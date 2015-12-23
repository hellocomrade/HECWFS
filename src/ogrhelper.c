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
#include "ogrhelper.h"

inline static char createField(const char *pszFldName,OGRFieldType type,size_t width,size_t precision,OGRLayerH hLayer)
{
    char result=-1;
    OGRFieldDefnH hFld=NULL;
    if(!pszFldName||!hLayer)
        return result;
    if((hFld=OGR_Fld_Create(pszFldName,type))!=NULL)
    {
        if(width>0)
            OGR_Fld_SetWidth(hFld,width);
        if(precision>0)
            OGR_Fld_SetPrecision(hFld,precision);
        OGR_L_CreateField(hLayer,hFld,FALSE);
        OGR_Fld_Destroy(hFld);
        result=0;
    }
    else
        printError(0,"OGR can not create field %s\n",pszFldName);
    return result;
}
char createGrid(const char *pszOutputFile,float *pfUa,float *pfVa,size_t elem_size,float *pfX,float *pfY,float *pfNv,size_t node_size)
{
    char result=-1;
    if(!pfUa||!pfVa||elem_size<1||!pfX||!pfY||!pfNv||node_size<1)
        return result;
    //const char *pszOutputFile="./hecwfs_grid.shp";
    OGRDataSourceH hDs=NULL;
    OGRSpatialReferenceH hSRS=NULL;
    OGRSFDriverH hDriver=NULL;
    OGRFieldDefnH hFld=NULL;
    OGRLayerH hLayer=NULL;
    OGRFeatureDefnH hFtrDef=NULL;
    OGRFeatureH hFeature=NULL;
    OGRGeometryH hGeom=NULL;
    OGRGeometryH hRing=NULL;
    int i;
    OGRRegisterAll();
    if((hSRS=OSRNewSpatialReference(NULL))==NULL)//epsg:26990
    {
        printError(0,"OGR can not create spatial reference state plane MI south for %s\n",pszOutputFile);
        return result;
    }
    //OSRSetWellKnownGeogCS(hSRS,"NAD83");
    //OSRSetLCC(hSRS,43.666666666666664,42.1,41.5,-84.36666666666666,4000000.0,0.0);
    OSRImportFromEPSG(hSRS,26990);
    if((hDriver=OGRGetDriverByName("ESRI Shapefile"))==NULL)
    {
        printError(0,"OGR can not create driver for %s\n",pszOutputFile);
        return result;
    }
    if((hDs=OGR_Dr_CreateDataSource(hDriver,pszOutputFile,NULL))==NULL)
    {
        printError(0,"OGR can not create data source for %s, file may already exist!\n",pszOutputFile);
        return result;
    }
    if((hLayer=OGR_DS_CreateLayer(hDs,"hecwfs",hSRS,wkbPolygon,NULL))==NULL)
    {
        printError(0,"OGR can not create point layer for %s\n",pszOutputFile);
        return result;
    }
    //if(createField("id",OFTInteger,-1,-1,hLayer)<0)
    //    return result;
    if((hFtrDef=OGR_L_GetLayerDefn(hLayer))==NULL)
    {
        printError(0,"OGR can not retrieve layer defination for %s\n",pszOutputFile);
        return result;
    }
    for(i=0;i<elem_size;++i)
    {
        hFeature=OGR_F_Create(hFtrDef);
        hGeom=OGR_G_CreateGeometry(wkbPolygon);
        hRing=OGR_G_CreateGeometry(wkbLinearRing);
        if(pfNv[i]+pfNv[elem_size+i]+pfNv[elem_size*2+i]>node_size*3)
            break;
        OGR_G_AddPoint_2D(hRing,pfX[(int)(pfNv[i])-1],pfY[(int)(pfNv[i])-1]);
        OGR_G_AddPoint_2D(hRing,pfX[(int)(pfNv[elem_size+i])-1],pfY[(int)(pfNv[elem_size+i])-1]);
        OGR_G_AddPoint_2D(hRing,pfX[(int)(pfNv[elem_size*2+i])-1],pfY[(int)(pfNv[elem_size*2+i])-1]);
        OGR_G_AddPoint_2D(hRing,pfX[(int)(pfNv[i])-1],pfY[(int)(pfNv[i])-1]);
        OGR_G_AddGeometry(hGeom,hRing);
        OGR_F_SetGeometry(hFeature,hGeom);
        //OGR_F_SetFieldInteger(hFeature,0,i+1);
        OGR_L_CreateFeature(hLayer,hFeature);
        OGR_G_DestroyGeometry(hRing);
        OGR_G_DestroyGeometry(hGeom);
        OGR_F_Destroy(hFeature);
    }
    OSRDestroySpatialReference( hSRS );
    OGR_DS_Destroy(hDs);
    return 0;
}
char createPointLayer(const char *pszOutputFile,float *pfUa,float *pfVa,float *pfWl,size_t elem_size, float *pfX, float *pfY,float *pfNv,size_t node_size,char *pszTimestamp)
{
    char result=-1;
    if(!pszOutputFile||!pfUa||!pfVa||elem_size<1||!pfX||!pfY||!pfNv||node_size<1||!pszTimestamp)
        return result;
    OGRDataSourceH hDs=NULL;
    OGRSpatialReferenceH hSRS=NULL;
    OGRSFDriverH hDriver=NULL;
    OGRFieldDefnH hFld=NULL;
    OGRLayerH hLayer=NULL;
    OGRFeatureDefnH hFtrDef=NULL;
    OGRFeatureH hFeature=NULL;
    OGRGeometryH hGeom=NULL;
    int i;
    int degree=0;
    OGRRegisterAll();
    if((hSRS=OSRNewSpatialReference(NULL))==NULL)//epsg:26990
    {
        printError(0,"OGR can not create spatial reference state plane MI south for %s\n",pszOutputFile);
        return result;
    }
    //OSRSetWellKnownGeogCS(hSRS,"NAD83");
    //OSRSetLCC(hSRS,43.666666666666664,42.1,41.5,-84.36666666666666,4000000.0,0.0);
    OSRImportFromEPSG(hSRS,26990);
    if((hDriver=OGRGetDriverByName("ESRI Shapefile"))==NULL)
    {
        printError(0,"OGR can not create driver for %s\n",pszOutputFile);
        return result;
    }
    if((hDs=OGR_Dr_CreateDataSource(hDriver,pszOutputFile,NULL))==NULL)
    {
        printError(0,"OGR can not create data source for %s, file may already exist!\n",pszOutputFile);
        return result;
    }
    if((hLayer=OGR_DS_CreateLayer(hDs,"hecwfs",hSRS,wkbPoint,NULL))==NULL)
    {
        printError(0,"OGR can not create point layer for %s\n",pszOutputFile);
        return result;
    }
    if(createField("ua",OFTReal,32,8,hLayer)<0)
        return result;
    if(createField("va",OFTReal,32,8,hLayer)<0)
        return result;
    if(createField("v",OFTReal,32,8,hLayer)<0)
	return result;
    
    if(createField("a_angle",OFTInteger,3,-1,hLayer)<0)
	return result;
    if(createField("g_angle",OFTInteger,3,-1,hLayer)<0)
	return result;
    if(createField("m_angle",OFTInteger,4,-1,hLayer)<0)//one byte for +/- sign
	return result;
    if(createField("timestamp",OFTString,20,-1,hLayer)<0)
	return result;
    if(createField("zeta",OFTReal,32,8,hLayer)<0)
	return result;
    if((hFtrDef=OGR_L_GetLayerDefn(hLayer))==NULL)
    {
        printError(0,"OGR can not retrieve layer defination for %s\n",pszOutputFile);
        return result;
    }
    
    for(i=0;i<elem_size;++i)
    {
        hFeature=OGR_F_Create(hFtrDef);
        hGeom=OGR_G_CreateGeometry(wkbPoint);
        if(pfNv[i]+pfNv[elem_size+i]+pfNv[elem_size*2+i]>node_size*3)
            break;
        OGR_G_SetPoint(hGeom,0,((pfX[(int)(pfNv[i])-1]+pfX[(int)(pfNv[elem_size+i])-1]+pfX[(int)(pfNv[elem_size*2+i])-1])/3.0),\
                               ((pfY[(int)(pfNv[i])-1]+pfY[(int)(pfNv[elem_size+i])-1]+pfY[(int)(pfNv[elem_size*2+i])-1])/3.0),\
                               0.0);
        OGR_F_SetGeometry(hFeature,hGeom);
        OGR_F_SetFieldDouble(hFeature,0,pfUa[i]);
        OGR_F_SetFieldDouble(hFeature,1,pfVa[i]);
	OGR_F_SetFieldDouble(hFeature,2,sqrt((double)(pow(pfUa[i],2)+pow(pfVa[i],2))));
        
	//if(pfUa[i]<-HECWFS_ZERO&&pfVa[i]>-HECWFS_ZERO)
	//    degree=(int)((PI*5.0/2.0-atan2(pfVa[i],pfUa[i]))*180.0/PI+0.5);
	//else
	//    degree=(int)((PI/2.0-atan2(pfVa[i],pfUa[i]))*180.0/PI+0.5);

        //Result: -PI~PI Inclusive
        degree=(int)(atan2(pfVa[i],pfUa[i])*180.0/PI+0.5);
        if(degree<0)
            degree+=360;
        //Arithmetic Rotation
	OGR_F_SetFieldInteger(hFeature,3,degree);
        //Geographic Rotation
        if(degree>=0&&degree<=90)//1
            degree=90-degree;
        else if(degree>90&&degree<=180)//2
            degree=360-(degree-90);
        else if(degree>180&&degree<=270)//3
            degree=90-(degree-180)+180;
        else if(degree>270&&degree<=360)//4
            degree=180-(degree-270);
        OGR_F_SetFieldInteger(hFeature,4,degree);
        //MapServer Rotation
        degree*=-1;
        OGR_F_SetFieldInteger(hFeature,5,degree);
        OGR_F_SetFieldString(hFeature,6,pszTimestamp);
        OGR_F_SetFieldDouble(hFeature,7,(pfWl[(int)(pfNv[i])-1]+pfWl[(int)(pfNv[elem_size+i])-1]+pfWl[(int)(pfNv[elem_size*2+i])-1])/3.0+176.784);

        OGR_L_CreateFeature(hLayer,hFeature);
        OGR_G_DestroyGeometry(hGeom);
        OGR_F_Destroy(hFeature);
    }
    OSRDestroySpatialReference( hSRS );
    OGR_DS_Destroy(hDs);
    return 0;
}
