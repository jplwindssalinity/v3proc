//==============================================================//
// Copyright (C) 2006, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_wind_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Rain.h"
#include "Array.h"
#include "Constants.h"

//--------------------//
// WindField::ReadSV  //
// windfield from     //
// Svetla Veleva      //
//--------------------//

RainField::RainField()
:  lat_min(0),
   lat_max(0),
   lon_min(0),
   lon_max(0),
   num_lats(0),
   num_lons(0),
   _wrap(1),
   A(NULL),
   vB(NULL),
   sB(NULL),
   flag(NULL)
{
  // HACK ALERT for now rainfield only works for 46 H pol, 54 V pol case
  // this threshold is use to select between the two versions
  inc_thresh=50*dtr; 
}

RainField::~RainField()
{
  if(flag!=NULL) _Deallocate();
}


// this routine should be called from ConfigRainField
// after lon_min,lon_max and lat_min,lat_max are set
int
RainField::ReadSVBinary(
    char*  filename1,
    char*  filename2)
{

  char* filename[2];
  filename[0]=filename1;
  filename[1]=filename2;

  for(int i=0;i<2;i++){
    //-----------//
    // open file //
    //-----------//

    FILE* fp = fopen(filename[i], "r");
    if (fp == NULL)
        return(0);

    if(i==0){
      if ( fread((void *)&num_lons, sizeof(int), 1, fp) != 1 ||
	   fread((void *)&num_lats, sizeof(int), 1, fp) != 1)
	{
	  fclose(fp);
	  return(0);
	}
    }

    else{
      int n1,n2;
      if ( fread((void *)&n1, sizeof(int), 1, fp) != 1 ||
	   fread((void *)&n2, sizeof(int), 1, fp) != 1)
	{
	  fclose(fp);
	  return(0);
	}
      if(n1!=num_lons || n2!=num_lats){
	fprintf(stderr,"RainField::ReadSVBinary: Mismatch between file sizes\n");  
	fclose(fp);
        return(0);
      }
    }

    if(i==0){
      // allocate arrays
      if(!_Allocate()){
	fclose(fp);
	return(0);
      }
    }

    //------------//
    // read fields //
    //------------//
    
    for(int j=0;j<num_lons;j++){
      if(fread(&(A[i][j][0]),sizeof(float),num_lats,fp)!=(unsigned)num_lats){
	fclose(fp);
        return(0);
      }
    }
    for(int j=0;j<num_lons;j++){
      if(fread(&(vB[i][j][0]),sizeof(float),num_lats,fp)!=(unsigned)num_lats){
	fclose(fp);
        return(0);
      }
    }
    for(int j=0;j<num_lons;j++){
      if(fread(&(sB[i][j][0]),sizeof(float),num_lats,fp)!=(unsigned)num_lats){
	fclose(fp);
        return(0);
      }
    }

    for(int j=0;j<num_lons;j++){
      if(fread(&(flag[i][j][0]),sizeof(int),num_lats,fp)!=(unsigned)num_lats){
	fclose(fp);
        return(0);
      }
    }

    //-----------------//
    // set up indices  //
    //-----------------//
    if(i==0){
      if(_wrap){
	_lon.SpecifyWrappedCenters(lon_min * dtr, lon_max * dtr, num_lons);
      }
      else{
	_lon.SpecifyCenters(lon_min * dtr, lon_max * dtr, num_lons);
      }
      _lat.SpecifyCenters(lat_min * dtr, lat_max * dtr, num_lats);
    }
    fclose(fp);
    
  }
  return(1);
}   

int RainField::_Allocate(){
  A=(float***) make_array(sizeof(float),3,2,num_lons,num_lats);
  vB=(float***) make_array(sizeof(float),3,2,num_lons,num_lats);
  sB=(float***) make_array(sizeof(float),3,2,num_lons,num_lats);
  flag=(int***) make_array(sizeof(int),3,2,num_lons,num_lats);
  if(A==NULL || vB==NULL || sB==NULL || flag == NULL) return(0);
  return(1);
}


int RainField::_Deallocate(){
  free_array((void*)A,3,2,num_lats,num_lons);
  free_array((void*)vB,3,2,num_lats,num_lons);
  free_array((void*)sB,3,2,num_lats,num_lons);
  free_array((void*)flag,3,2,num_lats,num_lons);
  A=NULL;
  vB=NULL;
  sB=NULL;
  flag=NULL;
  return(1);
}

int  RainField::NearestABLinear(LonLat lon_lat,float inc,float& atten, float& backscat){

    int beam=(inc>inc_thresh);

    // put longitude in range 
    float lonmin = _lon.GetMin();
    int wrap_factor = (int)ceil((lonmin - lon_lat.longitude) / two_pi);
    float lon = lon_lat.longitude + (float)wrap_factor * two_pi;

    // convert to longitude index
    int lon_idx;
    if (! _lon.GetNearestIndexStrict(lon, &lon_idx))
        return(0);

    // convert to latitude index
    int lat_idx;
    if (! _lat.GetNearestIndexStrict(lon_lat.latitude, &lat_idx))
        return(0);

    atten=A[beam][lon_idx][lat_idx];
    backscat=vB[beam][lon_idx][lat_idx]+sB[beam][lon_idx][lat_idx]/atten;

    return(!flag[beam][lon_idx][lat_idx]);
}

int  RainField::InterpolateABLinear(LonLat lon_lat,float inc,float& atten, float& backscat){

    int beam=(inc>inc_thresh);
    // put longitude in range 
    float lonmin = _lon.GetMin();
    int wrap_factor = (int)ceil((lonmin - lon_lat.longitude) / two_pi);
    float lon = lon_lat.longitude + (float)wrap_factor * two_pi;

    // find longitude indices
    int lon_idx[2];
    float lon_coef[2];
    if (_wrap)
    {
        if (! _lon.GetLinearCoefsWrapped(lon, lon_idx, lon_coef))
            return(0);
    }
    else
    {
        if (! _lon.GetLinearCoefsStrict(lon, lon_idx, lon_coef))
            return(0);
    }

    // find latitude indicies
    int lat_idx[2];
    float lat_coef[2];
    if (! _lat.GetLinearCoefsStrict(lon_lat.latitude, lat_idx, lat_coef))
        return(0);

    float corner_u[2][2];
    float** _field = A[beam];
    corner_u[0][0] = *(*(_field + lon_idx[0]) + lat_idx[0]);
    corner_u[0][1] = *(*(_field + lon_idx[0]) + lat_idx[1]);
    corner_u[1][0] = *(*(_field + lon_idx[1]) + lat_idx[0]);
    corner_u[1][1] = *(*(_field + lon_idx[1]) + lat_idx[1]);
    atten= lon_coef[0] * lat_coef[0] * corner_u[0][0] +
                lon_coef[0] * lat_coef[1] * corner_u[0][1] +
                lon_coef[1] * lat_coef[0] * corner_u[1][0] +
                lon_coef[1] * lat_coef[1] * corner_u[1][1];


    _field = vB[beam];
    corner_u[0][0] = *(*(_field + lon_idx[0]) + lat_idx[0]);
    corner_u[0][1] = *(*(_field + lon_idx[0]) + lat_idx[1]);
    corner_u[1][0] = *(*(_field + lon_idx[1]) + lat_idx[0]);
    corner_u[1][1] = *(*(_field + lon_idx[1]) + lat_idx[1]);
    float vbval = lon_coef[0] * lat_coef[0] * corner_u[0][0] +
                lon_coef[0] * lat_coef[1] * corner_u[0][1] +
                lon_coef[1] * lat_coef[0] * corner_u[1][0] +
                lon_coef[1] * lat_coef[1] * corner_u[1][1];

    _field = sB[beam];
    corner_u[0][0] = *(*(_field + lon_idx[0]) + lat_idx[0]);
    corner_u[0][1] = *(*(_field + lon_idx[0]) + lat_idx[1]);
    corner_u[1][0] = *(*(_field + lon_idx[1]) + lat_idx[0]);
    corner_u[1][1] = *(*(_field + lon_idx[1]) + lat_idx[1]);
    float sbval = lon_coef[0] * lat_coef[0] * corner_u[0][0] +
                lon_coef[0] * lat_coef[1] * corner_u[0][1] +
                lon_coef[1] * lat_coef[0] * corner_u[1][0] +
                lon_coef[1] * lat_coef[1] * corner_u[1][1];

    backscat=vbval+sbval/atten;


    int flags[2][2];
    int** f=flag[beam];
    flags[0][0] = *(*(f + lon_idx[0]) + lat_idx[0]);
    flags[0][1] = *(*(f + lon_idx[0]) + lat_idx[1]);
    flags[1][0] = *(*(f + lon_idx[1]) + lat_idx[0]);
    flags[1][1] = *(*(f + lon_idx[1]) + lat_idx[1]);

    int goodval=!flags[0][0] && !flags[0][1] && !flags[1][0] && !flags[1][1];
    return(goodval);
}
