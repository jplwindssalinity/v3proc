//==============================================================//
// Copyright (C) 1998-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_landmap_c[] =
    "@(#) $Id$";

#include <math.h>
#include "Array.h"
#include "Constants.h"
#include "LandMap.h"

//==========//
// LandMap  //
//==========//

LandMap::LandMap()
:   _map(NULL), _pixelsPerDegree(12)
{
    _mapLatDim = 180*_pixelsPerDegree;
    _mapLonDim = 45*_pixelsPerDegree;
    return;
}

LandMap::~LandMap(){
  if(_map) _Deallocate();
  _map=NULL;
  return;
}

//=====================//
// LandMap::Initialize //
//=====================//

int LandMap::Initialize(char* filename, int use_map){
  _usemap=use_map;
  if(_usemap!=0){
    if(!Read(filename)) return(0);
  }
  return(1);
}

//===================//
// LandMap::Read    //
//===================//

int LandMap::Read(char* filename){
  FILE* ifp=fopen(filename,"r");
  if(ifp==NULL){
    fprintf(stderr,"LandMap File Read Failed for file %s.\n",filename);
    exit(1);
  }
  if(!_Allocate()) return(0);
  for(int c=0;c<_mapLatDim;c++){
    if(fread((void*)&_map[c][0],sizeof(unsigned char),_mapLonDim,ifp)!=(unsigned)_mapLonDim)
      return(0);
  }
  fclose(ifp);
  return(1);
}

//=========================//
// LandMap::IsLand        //
//=========================//

int
LandMap::IsLand(
    float  lon,
    float  lat)
{
      if(_usemap==0) return(0);

      while(lon < 0) lon+=two_pi;
      int lon_idx=int(lon*rtd*_pixelsPerDegree+0.5);
      int lat_idx=int((90.0+lat*rtd)*_pixelsPerDegree+0.5);
      int lon_byte_idx=lon_idx/8;
      int lon_bit_idx=lon_idx % 8;
      if(lon_bit_idx==0) lon_bit_idx=8;
      lon_bit_idx=8-lon_bit_idx;
      if(lat_idx==_mapLatDim) return(0);
      if(lon_byte_idx==_mapLonDim){
	lon_byte_idx=0;
      }
      unsigned char byte=_map[lat_idx][lon_byte_idx];
      int bit=(int)(byte&(0x1<<lon_bit_idx));
      if(bit!=0) bit=1;
      return(bit);
}

//-----------------//
// LandMap::IsLand //
//-----------------//

int
LandMap::IsLand(
    LonLat*  lon_lat)
{
    return(IsLand(lon_lat->longitude, lon_lat->latitude));
}

//=========================//
// LandMap::_Allocate     //
//=========================//

int LandMap::_Allocate(){
  _map=(unsigned char**)make_array(sizeof(char),2,_mapLatDim,_mapLonDim);
  if(!_map) return(0);
  return(1);
}

//=========================//
// LandMap::_Deallocate   //
//=========================//

int LandMap::_Deallocate(){
  free_array((void*)_map,2,_mapLatDim,_mapLonDim);
  return(1);
}







