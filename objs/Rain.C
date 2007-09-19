//==============================================================//
// Copyright (C) 2006, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_wind_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>
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
    }
    fclose(fp);
    
  }
  return(1);
}   

// this routine should be called from ConfigRainField
// after lon_min,lon_max and lat_min,lat_max are set
int
RainField::ReadSV3DData(
    char*  filename1,
    char*  filename2)
{

  char* filename[2];
  filename[0]=filename1;
  filename[1]=filename2;

  //-------------------------//
  // open refl and attn file //
  //-------------------------//

  FILE* fp = fopen(filename[0], "r");
  if (fp == NULL)
      return(0);

  int byte_count1, byte_count2, data_count1, data_count2;
  int lonDim, latDim, hgtDim;

  if (fread((void *)&byte_count1, sizeof(int), 1, fp) != 1 ||
      fread((void *)&lonDim, sizeof(int), 1, fp) != 1 ||
      fread((void *)&latDim, sizeof(int), 1, fp) != 1 ||
      fread((void *)&hgtDim, sizeof(int), 1, fp) != 1 ||
      fread((void *)&byte_count2, sizeof(int), 1, fp) != 1 ||
      fread((void *)&data_count1, sizeof(int), 1, fp) != 1)
  {
      fclose(fp);
      return(0);
  }

  num_lats = latDim;
  num_lons = lonDim;
  num_hgts = hgtDim;

  // allocate arrays
  if(!_Allocate()){
    fclose(fp);
    return(0);
  }

  //-------------//
  // read fields //
  //-------------//
    
  for(int kk=0;kk<num_hgts;kk++){
    for(int jj=0;jj<num_lats;jj++){
      if(fread(&(vB3[kk][jj][0]),sizeof(float),num_lons,fp)!=(unsigned)num_lons){
        fclose(fp);
        return(0);
      }
    }
  }

  for(int kk=0;kk<num_hgts;kk++){
    for(int jj=0;jj<num_lats;jj++){
      if(fread(&(A3[kk][jj][0]),sizeof(float),num_lons,fp)!=(unsigned)num_lons){
        fclose(fp);
        return(0);
      }
    }
  }

  if(fread(&(hgtInc[0]),sizeof(float),num_hgts,fp)!=(unsigned)num_hgts){
    fclose(fp);
    return(0);
  }

  //-----------------//
  // set up indices  //
  //-----------------//
  if(_wrap){
    _lon.SpecifyWrappedCenters(lon_min * dtr, lon_max * dtr, num_lons);
  }
  else{
    _lon.SpecifyCenters(lon_min * dtr, lon_max * dtr, num_lons);
  }
  _lat.SpecifyCenters(lat_min * dtr, lat_max * dtr, num_lats);

  fclose(fp);

  //------------------//
  // open splash file //
  //------------------//

  fp = fopen(filename[1], "r");
  if (fp == NULL)
      return(0);

  if (fread((void *)&byte_count1, sizeof(int), 1, fp) != 1 ||
      fread((void *)&lonDim, sizeof(int), 1, fp) != 1 ||
      fread((void *)&latDim, sizeof(int), 1, fp) != 1 ||
      fread((void *)&byte_count2, sizeof(int), 1, fp) != 1 ||
      fread((void *)&data_count1, sizeof(int), 1, fp) != 1)
  {
      fclose(fp);
      return(0);
  }

  if (lonDim != num_lons || latDim != num_lats) {
    fprintf(stderr,"RainField::ReadSV3DData: Mismatch between file sizes\n");  
    fclose(fp);
    return(0);
  }

  // for 54 degree inc angle
  for(int ee=0;ee<2;ee++){ // there are 2 estimates
    for(int jj=0;jj<num_lats;jj++){
      if(fread(&(sB[ee][jj][0]),sizeof(float),num_lons,fp)!=(unsigned)num_lons){
        fclose(fp);
        return(0);
      }
    }
  }

  // for 46 degree inc angle
  for(int ee=0;ee<2;ee++){ // there are 2 estimates
    for(int jj=0;jj<num_lats;jj++){
      if(fread(&(sB[ee+2][jj][0]),sizeof(float),num_lons,fp)!=(unsigned)num_lons){
        fclose(fp);
        return(0);
      }
    }
  }

  return(1);
}   

int RainField::_Allocate(){
  if (!flag_3d) {
    A=(float***) make_array(sizeof(float),3,2,num_lons,num_lats);
    vB=(float***) make_array(sizeof(float),3,2,num_lons,num_lats);
    sB=(float***) make_array(sizeof(float),3,2,num_lons,num_lats);
    flag=(int***) make_array(sizeof(int),3,2,num_lons,num_lats);
    if(A==NULL || vB==NULL || sB==NULL || flag == NULL) return(0);
  } else if (flag_3d) {
    //vB3=(double***) make_array(sizeof(double),3,num_hgts,num_lats,num_lons);
    //A3=(double***) make_array(sizeof(double),3,num_hgts,num_lats,num_lons);
    vB3=(float***) make_array(sizeof(float),3,num_hgts,num_lats,num_lons);
    A3=(float***) make_array(sizeof(float),3,num_hgts,num_lats,num_lons);
    hgtInc = (float*) make_array(sizeof(float), 1, num_hgts);
    sB=(float***) make_array(sizeof(float),3,4,num_lats,num_lons);
    if(A3==NULL || vB3==NULL || hgtInc==NULL || sB==NULL) return(0);
  }
  return(1);
}


int RainField::_Deallocate(){
  if (!flag_3d) {
    free_array((void*)A,3,2,num_lats,num_lons);
    free_array((void*)vB,3,2,num_lats,num_lons);
    free_array((void*)sB,3,2,num_lats,num_lons);
    free_array((void*)flag,3,2,num_lats,num_lons);
    A=NULL;
    vB=NULL;
    sB=NULL;
    flag=NULL;
  } else if (flag_3d) {
    free_array((void*)vB3,3,num_hgts,num_lats,num_lons);
    free_array((void*)A3,3,num_hgts,num_lats,num_lons);
    free_array((void*)hgtInc,1,num_hgts);
    free_array((void*)sB,3,4,num_lats,num_lons);
    vB3 = NULL;
    A3 = NULL;
    hgtInc = NULL;
    sB = NULL;
  }

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

// this routine is used to find attenuation of a rain cell from SV rainData
int
RainField::GetAttn(
    double alt,
    double lon,
    double lat,
    float *cellAttn)
{
  float lonmin = _lon.GetMin();
  int wrap_factor = (int)ceil((lonmin - lon) / two_pi);
  lon = lon + (float)wrap_factor * two_pi;

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

  //cout << lon*rtd << endl;
  //cout << lon_idx[0] << " " << lon_idx[1] << endl;
  //cout << lon_coef[0] << " " << lon_coef[1] << endl;

  // find latitude indicies
  int lat_idx[2];
  float lat_coef[2];
  if (! _lat.GetLinearCoefsStrict(lat, lat_idx, lat_coef))
    return(0);

  //cout << lat_idx[0] << " " << lat_idx[1] << endl;
  //cout << lat_coef[0] << " " << lat_coef[1] << endl;

  // find layer index
  int hgt_idx;
  hgt_idx = int(alt/DZ_LAYER);
  if (fabs(hgt_idx*DZ_LAYER - alt) > 0.5*DZ_LAYER) {
    hgt_idx++;
  }

  hgt_idx--; // for data indexing start with 0
  if (hgt_idx > N_LAYERS-1) hgt_idx = N_LAYERS-1;

  //cout << hgt_idx << endl;

  float corner_u[2][2];
  corner_u[0][0] = A3[hgt_idx][lat_idx[0]][lon_idx[0]];
  corner_u[0][1] = A3[hgt_idx][lat_idx[1]][lon_idx[0]];
  corner_u[1][0] = A3[hgt_idx][lat_idx[0]][lon_idx[1]];
  corner_u[1][1] = A3[hgt_idx][lat_idx[1]][lon_idx[1]];

  //cout << corner_u[0][0] << endl;
  //cout << corner_u[0][1] << endl;
  //cout << corner_u[1][0] << endl;
  //cout << corner_u[1][1] << endl;

  *cellAttn= lon_coef[0] * lat_coef[0] * corner_u[0][0] +
             lon_coef[0] * lat_coef[1] * corner_u[0][1] +
             lon_coef[1] * lat_coef[0] * corner_u[1][0] +
             lon_coef[1] * lat_coef[1] * corner_u[1][1];

  return 1;
}

// this routine is used to find backscattering of a rain cell from SV rainData
int
RainField::GetRefl(
    double alt,
    double lon,
    double lat,
    float *cellRefl)
{
  float lonmin = _lon.GetMin();
  int wrap_factor = (int)ceil((lonmin - lon) / two_pi);
  lon = lon + (float)wrap_factor * two_pi;

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

  //cout << lon_idx[0] << " " << lon_idx[1] << endl;
  //cout << lon_coef[0] << " " << lon_coef[1] << endl;

  // find latitude indicies
  int lat_idx[2];
  float lat_coef[2];
  if (! _lat.GetLinearCoefsStrict(lat, lat_idx, lat_coef))
    return(0);

  //cout << lat_idx[0] << " " << lat_idx[1] << endl;
  //cout << lat_coef[0] << " " << lat_coef[1] << endl;


  // find layer index
  int hgt_idx;
  hgt_idx = int(alt/DZ_LAYER);
  if (fabs(hgt_idx*DZ_LAYER - alt) > 0.5*DZ_LAYER) {
    hgt_idx++;
  }

  hgt_idx--; // for data indexing start with 0
  if (hgt_idx > N_LAYERS-1) hgt_idx = N_LAYERS-1;

  //cout << hgt_idx << endl;

  float corner_u[2][2];
  corner_u[0][0] = vB3[hgt_idx][lat_idx[0]][lon_idx[0]];
  corner_u[0][1] = vB3[hgt_idx][lat_idx[1]][lon_idx[0]];
  corner_u[1][0] = vB3[hgt_idx][lat_idx[0]][lon_idx[1]];
  corner_u[1][1] = vB3[hgt_idx][lat_idx[1]][lon_idx[1]];

  //cout << corner_u[0][0] << endl;
  //cout << corner_u[0][1] << endl;
  //cout << corner_u[1][0] << endl;
  //cout << corner_u[1][1] << endl;

  *cellRefl= lon_coef[0] * lat_coef[0] * corner_u[0][0] +
             lon_coef[0] * lat_coef[1] * corner_u[0][1] +
             lon_coef[1] * lat_coef[0] * corner_u[1][0] +
             lon_coef[1] * lat_coef[1] * corner_u[1][1];

  return 1;
}

// this routine is used to find splash of rain
int
RainField::GetSplash(
    double lon,
    double lat,
    float inc,
    float *rainSpl)
{
  int beam = (inc<inc_thresh)*2; 

  float lonmin = _lon.GetMin();
  int wrap_factor = (int)ceil((lonmin - lon) / two_pi);
  lon = lon + (float)wrap_factor * two_pi;

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

  //cout << lon*rtd  << " " << lat*rtd << endl;
  //cout << lon_idx[0] << " " << lon_idx[1] << endl;
  //cout << lon_coef[0] << " " << lon_coef[1] << endl;

  // find latitude indicies
  int lat_idx[2];
  float lat_coef[2];
  if (! _lat.GetLinearCoefsStrict(lat, lat_idx, lat_coef))
    return(0);

  //cout << lat_idx[0] << " " << lat_idx[1] << endl;
  //cout << lat_coef[0] << " " << lat_coef[1] << endl;
  float corner_u[2][2];

  corner_u[0][0] = sB[beam][lat_idx[0]][lon_idx[0]];
  corner_u[0][1] = sB[beam][lat_idx[1]][lon_idx[0]];
  corner_u[1][0] = sB[beam][lat_idx[0]][lon_idx[1]];
  corner_u[1][1] = sB[beam][lat_idx[1]][lon_idx[1]];

  //cout << corner_u[0][0] << endl;
  //cout << corner_u[0][1] << endl;
  //cout << corner_u[1][0] << endl;
  //cout << corner_u[1][1] << endl;

  *rainSpl = lon_coef[0] * lat_coef[0] * corner_u[0][0] +
             lon_coef[0] * lat_coef[1] * corner_u[0][1] +
             lon_coef[1] * lat_coef[0] * corner_u[1][0] +
             lon_coef[1] * lat_coef[1] * corner_u[1][1];

  return 1;
}

// this routine is used to find the total attenuation
// from the spacecraft to a target point
// Target can be the centroid of a measurement (meas centroid) or a rain cell
int
RainField::ComputeAttn(
    Spacecraft* spacecraft,
    EarthPosition spot_centroid,
    EarthPosition target,
    float incAngle,
    CoordinateSwitch gc_to_rangeazim,
    float* attn)
{
  *attn = 0.;

  Vector3 target_ra = gc_to_rangeazim.Forward(target-spot_centroid);

  Vector3 target_look = target - spacecraft->orbitState.rsat;
  target_look = gc_to_rangeazim.Forward(target_look);

  double alt, lon, lat;
  double fraction, r_attn, a_attn;
  float hh, cellAttn;
  EarthPosition rainCell;

  for (int nn=1; nn<=N_LAYERS; nn++) { // nn starts from 1 as we ignore ground
  //for (int nn=4; nn<5; nn++) { // nn starts from 1 as we ignore ground

    hh = nn*DZ_LAYER;
    fraction = (target_ra.Get(2) - hh)/target_look.Get(2);
    r_attn = target_ra.Get(0) - fraction*target_look.Get(0);
    a_attn = target_ra.Get(1) - fraction*target_look.Get(1);

    //cout << r_attn << " " << a_attn << endl;
    rainCell.SetPosition(r_attn, a_attn, hh);
    rainCell = gc_to_rangeazim.Backward(rainCell) + spot_centroid;
    //rainCell.Show();

    if (!rainCell.GetAltLonGDLat(&alt, &lon, &lat))
      return (0);

    //cout << alt << " " << lon << " " << lat << endl;

    if(!GetAttn(alt, lon, lat, &cellAttn))
      return (0);

    //cout << cellAttn << endl;
    //cout << hgtInc[nn-1] << endl;

    cellAttn *= (hgtInc[nn-1]/1000./cos(incAngle));
    //cout << cellAttn << endl;

    if (fraction > 0.) *attn += cellAttn;

    //cout << *attn << endl;
  } // layer loop

  return 1;
}

// this routine is used to find the rain cell locations at different height which
// have the same range and doppler from the spacecraft to the centroid
// of a measurement (meas centroid)
int
RainField::ComputeLoc(
    Spacecraft* spacecraft,
    Meas* meas,
    EarthPosition spot_centroid,
    CoordinateSwitch gc_to_rangeazim,
    float** rainRngAz)
{

  // get S/C velocity in Earth rotating frame and transform from gc to rangeazim
  Vector3 vscpos(-w_earth * spacecraft->orbitState.rsat.Get(1),
                  w_earth * spacecraft->orbitState.rsat.Get(0), 0);

  Vector3 vel_rot(spacecraft->orbitState.vsat.Get(0)-vscpos.Get(0),
                  spacecraft->orbitState.vsat.Get(1)-vscpos.Get(1),
                  spacecraft->orbitState.vsat.Get(2)-vscpos.Get(2));
  //cout << "dop 1: " << vel_rot % (meas->centroid-spacecraft->orbitState.rsat) << endl;

  Vector3 vel_rot_ra = gc_to_rangeazim.Forward(vel_rot);

  Vector3 centroid_look = meas->centroid - spacecraft->orbitState.rsat;
  double slant_range = centroid_look.Magnitude();

  Vector3 alpha = gc_to_rangeazim.Forward(spacecraft->orbitState.rsat - spot_centroid);

  Vector3 center_ra=gc_to_rangeazim.Forward(meas->centroid-spot_centroid);

  double v1, v2, v3, a1, a2, a3, b1, b2, b3, AA, BB, CC;

  v1 = vel_rot_ra.Get(0);
  v2 = vel_rot_ra.Get(1);
  v3 = vel_rot_ra.Get(2);
  a1 = alpha.Get(0);
  a2 = alpha.Get(1);
  a3 = alpha.Get(2);
  b1 = center_ra.Get(0);
  b2 = center_ra.Get(1);
  b3 = center_ra.Get(2);

  int rot_flag = 0;

  if (fabs(v2)<1.e-3) { // rot 90 degree of rangeazim coord
    rot_flag = 1;
    double temp;
    temp = v1;
    v1 = v2;
    v2 = -temp;
    temp = a1;
    a1 = a2;
    a2 = -temp;
    temp = b1;
    b1 = b2;
    b2 = -temp;
  }

  float hh;
  double rngA, azA, rngB, azB;

  for (int nn=1; nn<=N_LAYERS; nn++) { // nn starts from 1 as we ignore ground
  //for (int nn=4; nn<5; nn++) { // nn starts from 1 as we ignore ground

    hh = nn*DZ_LAYER;

    AA = 1 + (v1/v2)*(v1/v2);
    BB = a1 + (v1/v2)*((v1/v2)*b1 - (v3/v2)*(hh-b3) + b2 - a2);
    CC = ((v1/v2)*b1 - (v3/v2)*(hh-b3) + b2 - a2)*((v1/v2)*b1 - (v3/v2)*(hh-b3) + b2 - a2)
         + (hh-a3)*(hh-a3) - slant_range*slant_range + a1*a1;

    rngA = (BB+sqrt(BB*BB-AA*CC))/AA;
    azA = b2 - ((v1/v2)*(rngA-b1) + (v3/v2)*(hh-b3));
    rngB = (BB-sqrt(BB*BB-AA*CC))/AA;
    azB = b2 - ((v1/v2)*(rngB-b1) + (v3/v2)*(hh-b3));

    if (rot_flag) { // rot back
      double temp;
      temp = rngA;
      rngA = -azA;
      azA = temp;
      temp = rngB;
      rngB = -azB;
      azB = temp;
    }

    //cout << rngA << " " << azA << endl;
    //cout << rngB << " " << azB << endl;

    double d1, d2;

    // select point which is close to meas centroid
    d1 = (rngA-b1)*(rngA-b1) + (azA-b2)*(azA-b2); // find distance from meas centroid
    d2 = (rngB-b1)*(rngB-b1) + (azB-b2)*(azB-b2);

    if (d1>d2) {
      rngA = rngB;
      azA = azB;
    }

    rainRngAz[nn-1][0] = rngA;
    rainRngAz[nn-1][1] = azA;

    //cout << "r,a,h: " << rngA << " " << azA << " " << hh << endl;

    //Vector3 test(rngA, azA, hh);
    //test = gc_to_rangeazim.Backward(test);
    //cout << "dop 2: " << (test+spot_centroid-spacecraft->orbitState.rsat) % vel_rot << endl;

  } // layer loop

  return 1;
}

// this routine is used to find signal return from ambiguity
// For ambiguity, we use a point instead of summing over ptr grid
int
RainField::ComputeAmbEs(
    Spacecraft* spacecraft,
    EarthPosition spot_centroid,
    EarthPosition target,
    float incAngle,
    CoordinateSwitch gc_to_rangeazim,
    float ambs0,
    float* combs0)
{
  *combs0 = 0.;

  Vector3 target_ra = gc_to_rangeazim.Forward(target-spot_centroid);

  Vector3 target_look = target - spacecraft->orbitState.rsat;
  target_look = gc_to_rangeazim.Forward(target_look);

  double alt, lon, lat;
  double fraction, r_attn, a_attn;
  float hh;
  EarthPosition rainCell;

  float layer_attn[40], total_attn;
  float layer_refl[40];

  total_attn = 0.;

  // find the attn and refl at a point for each layer
  for (int nn=1; nn<=N_LAYERS; nn++) { // nn starts from 1 as we ignore ground

    hh = nn*DZ_LAYER;
    fraction = (target_ra.Get(2) - hh)/target_look.Get(2);
    r_attn = target_ra.Get(0) - fraction*target_look.Get(0);
    a_attn = target_ra.Get(1) - fraction*target_look.Get(1);

    //cout << r_attn << " " << a_attn << endl;
    rainCell.SetPosition(r_attn, a_attn, hh);
    rainCell = gc_to_rangeazim.Backward(rainCell) + spot_centroid;
    //rainCell.Show();

    if (!rainCell.GetAltLonGDLat(&alt, &lon, &lat))
      return (0);

    //cout << alt << " " << lon << " " << lat << endl;

    if(!GetAttn(alt, lon, lat, &layer_attn[nn-1]))
      return (0);
    layer_attn[nn-1] *= DZ_LAYER/cos(incAngle);
    total_attn += layer_attn[nn-1];

    if(!GetRefl(alt, lon, lat, &layer_refl[nn-1]))
      return (0);

    //cout << "layer attn, refl: " << nn << " " << layer_attn[nn-1] << " " << layer_refl[nn-1] << endl;

  } // layer loop

  //cout << "total attn: " << total_attn << endl;

  // sum contributions of all layers
  for (int nn=1; nn<=N_LAYERS; nn++) { // nn starts from 1 as we ignore ground
    // find total attn for this layer
    float lower_attn = 0.;
    for (int ll=1; ll<=nn; ll++) {
      lower_attn += layer_attn[ll-1];
      //cout << "layer, layer_attn, tot_attn: " << ll << " " << layer_attn[ll-1] << " " << total_attn-lower_attn << endl;
    }
    *combs0 += layer_refl[nn-1]*exp(-2.*(total_attn-lower_attn));
    //cout << "layer, tot_attn, refl, Es: " << nn << " " << total_attn-lower_attn << " " << layer_refl[nn-1] << " " << *combs0 << endl;
  }

  *combs0 *= const_ZtoSigma*DZ_LAYER*1000./cos(incAngle); // 1000. for DZ_LAYER from km to m

  //cout << *combs0 << endl;

  *combs0 += ambs0*exp(-2.*total_attn);
  //cout << *combs0 << endl;

  return 1;
}
