//==============================================================//
// Copyright (C) 1998-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_landmap_c[] =
    "@(#) $Id$";

#include <math.h>
#include <strings.h>
#include <string.h>
#include "Array.h"
#include "Constants.h"
#include "LandMap.h"
#include "Misc.h"

//=========//
// LandMap //
//=========//

LandMap::LandMap()
:   _map(NULL), _usemap(0)
{
    return;
}

LandMap::~LandMap()
{
    if (_map)
        _Deallocate();
    _map=NULL;
    return;
}

//---------------------//
// LandMap::Initialize //
//---------------------//

int
LandMap::Initialize(
    char*  filename,
    int    use_map,
    char* lmtype,
    float lonstart,
    float latstart)
{
    _usemap = use_map;
    _lat_start=latstart;
    _lon_start=lonstart;
    if (_usemap != 0)
    {
      if(lmtype==NULL ||
	 strcasecmp(lmtype,"OLD_STYLE")==0) {
	landmap_type=0;
	_pixelsPerDegree=12;
	_mapLatDim = 180 * _pixelsPerDegree;
	_mapLonDim = 45 * _pixelsPerDegree;
	if (! ReadOld(filename))
	  return(0);
      }
      else if(strcasecmp(lmtype,"SIMPLE")==0 ||
	      strcasecmp(lmtype,"SIMPLELANDMAP")==0 ||
	      strcasecmp(lmtype,"SIMPLE_LANDMAP")==0){
	landmap_type=1;
	if (! ReadSimple(filename))
	  return(0);
      }

      else if(strcasecmp(lmtype,"LANDUSE")==0){
	landmap_type=2;
	_mapLatDim=1200;
        _mapLonDim=1200;
	if (! ReadUSGS(filename))
	  return(0);

      }
      else{
	fprintf(stderr,"LandMap::Initialize Bad Type: %s\n",lmtype);
	return(0);
      }
    }

    return(1);
}

//------------------//
// LandMap::ReadOld //
//------------------//

int
LandMap::ReadOld(
    char*  filename)
{
    FILE* ifp = fopen(filename, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "LandMap File Read Failed for file %s.\n", filename);
        exit(1);
    }
    if (! _Allocate())
        return(0);
    for (int c = 0; c < _mapLatDim; c++)
    {
        if (fread((void*)&_map[c][0], sizeof(unsigned char), _mapLonDim,
            ifp) != (unsigned)_mapLonDim)
        {
            return(0);
        }
    }
    fclose(ifp);
    return(1);
}

int
LandMap::ReadSimple(
    char*  filename)
{
    FILE* ifp = fopen(filename, "r");
    if (ifp == NULL)
        return(0);

    int lon_samples, lat_samples;
    if (fread((void *)&lon_samples, sizeof(int), 1, ifp) != 1 ||
        fread((void *)&lat_samples, sizeof(int), 1, ifp) != 1)
    {
        fclose(ifp);
        return(0);
    }

    _mapLonDim = lon_samples;
    _mapLatDim = lat_samples;

    if (! _Allocate())
    {
        fclose(ifp);
        return(0);
    }

    int size = sizeof(char) * _mapLatDim;
    for (int lon_idx = 0; lon_idx < _mapLonDim; lon_idx++)
    {
        if (fread((void *)*(_map + lon_idx), size, 1, ifp) != 1)
        {
            fclose(ifp);
            return(0);
        }
    }

    _lonResolution = two_pi / _mapLonDim;
    _latResolution = pi / _mapLatDim;

    fclose(ifp);
    return(1);
}


#define USGS_BLOCK_SIZE (10*dtr)
#define USGS_BLOCK_PIXELS 1200
#define USGS_NLATS 21600
#define USGS_NLONS 43200
int
LandMap::ReadUSGS(
    char*  filename)
{   
  sprintf(usgs_dir,"%s",filename);
  _lat_start=floor(_lat_start/USGS_BLOCK_SIZE+0.00001)*USGS_BLOCK_SIZE;
  _lon_start=floor(_lon_start/USGS_BLOCK_SIZE+0.00001)*USGS_BLOCK_SIZE;
  int ilon0,ilon1,ilat0,ilat1;
  //ilon0=int((_lon_start+pi)*USGS_NLONS/two_pi + 0.0001) %USGS_NLONS+1;
  ilon0=int((_lon_start+pi)*USGS_NLONS/two_pi + 0.001) %USGS_NLONS+1;
  ilat0=int((_lat_start+pi/2)*USGS_NLATS/pi + 0.1)+1;
  ilon1=ilon0+USGS_BLOCK_PIXELS-1;
  ilat1=ilat0+USGS_BLOCK_PIXELS-1;
  char fullname[200];
  sprintf(fullname,"%s/%5.5d-%5.5d.%5.5d-%5.5d",usgs_dir,ilon0,ilon1,ilat0,ilat1);
  FILE* ifp = fopen(fullname, "r");
  if (ifp == NULL){
    fprintf(stderr,"Cannot open filename %s\n",fullname);
    return(0);
  }

  if (! _Allocate())
    {
      fclose(ifp);
      return(0);
    }

  int size = sizeof(char) * _mapLonDim;
  for (int lat_idx = 0; lat_idx < _mapLatDim; lat_idx++)
    {
      if (fread((void *)*(_map + lat_idx), size, 1, ifp) != 1)
        {
	  fclose(ifp);
	  return(0);
        }
    }
  
  _lonResolution = 10.0*dtr / _mapLonDim;
  _latResolution = 10.0*dtr / _mapLatDim;

  fclose(ifp);
  return(1);
}

//-----------------//
// LandMap::IsLand //
//-----------------//

int
LandMap::IsLand(
    float  lon,
    float  lat)
{
    if (_usemap == 0)
        return(0);

    while (lon < 0)
        lon+=two_pi;

    if(landmap_type==0){
      return(IsLandOld(lon,lat));
    }
    else if(landmap_type==1){
      return(IsLandSimple(lon,lat));      
    }
    else{
      return(IsLandUSGS(lon,lat));      
    }

}

int
LandMap::IsLandOld(float lon, float lat){
  int lon_idx = int(lon * rtd * _pixelsPerDegree + 0.5);
  int lat_idx = int((90.0 + lat * rtd) * _pixelsPerDegree + 0.5);
  int lon_byte_idx = lon_idx / 8;
  int lon_bit_idx = lon_idx % 8;
  
  lon_bit_idx = 7 - lon_bit_idx;
  if (lat_idx == _mapLatDim)
    return(0);
  
  if (lon_byte_idx == _mapLonDim)
    lon_byte_idx = 0;
  
  unsigned char byte = _map[lat_idx][lon_byte_idx];
  int bit = (int)(byte&(0x1 << lon_bit_idx));
  if (bit != 0)
    bit = 1;
  return(bit);
}


int
LandMap::IsLandSimple(
    float  lon,
    float  lat)
{
    if (_usemap == 0)
        return(0);
    while(lon<0) lon += two_pi;    // to make sure it is in range
    while(lon>=two_pi) lon -= two_pi;    // to make sure it is in range
    int lon_idx = (int)(lon / _lonResolution);
    int lat_idx = (int)((lat + pi_over_two) / _latResolution);
    lon_idx = lon_idx % _mapLonDim;
    if (lat_idx < 0)
        lat_idx = 0;
    if (lat_idx >= _mapLatDim)
        lat_idx = _mapLatDim;

    int flag = (int)*(*(_map + lon_idx) + lat_idx);
    if (flag == 1)
        return(1);
    else
        return(0);
}


int
LandMap::IsLandUSGS(
    float  lon,
    float  lat)
{
    if (_usemap == 0)
        return(0);
//     while(lon<_lon_start) lon += two_pi;    // to make sure it is in range
//     while(lon>=_lon_start+_mapLonDim*_lonResolution && lon>=pi) lon -= two_pi;    // to make sure it is in range
// 
//     // outside of landmap calls ExpandUSGS
//     if( lon <_lon_start || lat < _lat_start ||
// 	lat >= _lat_start+(_mapLatDim-1)*_latResolution  || 
// 	lon >= _lon_start+(_mapLonDim-1)*_lonResolution ){
//         //printf("expand %f %f %f %f %d %d %g %g\n", _lat_start, _lon_start, lat, lon, _mapLatDim, _mapLonDim, _latResolution, _lonResolution);
//       return(ExpandUSGS(lon,lat));
//     }
    
    if( lat < -pi/2 ) lat = -pi/2;
    if( lat >  pi/2 ) lat =  pi/2;
    while(lon<=-pi) lon+=two_pi;
    while(lon>pi)   lon-=two_pi;
    
    int lon_idx = (int)((lon-_lon_start) / _lonResolution);
    int lat_idx = (int)((lat-_lat_start) / _latResolution);
    
    if( lon_idx < 0 || lon_idx >= _mapLonDim || 
        lat_idx < 0 || lat_idx >= _mapLatDim ) {
      return(ExpandUSGS(lon,lat));
    }
    
    int flag = (int)*(*(_map + lat_idx) + lon_idx);
    if (flag != 16)
        return(1);
    else
        return(0);
}

int LandMap::ExpandUSGS(float lon, float lat){
  double this_lon = lon*rtd;
  double this_lat = lat*rtd;
  
  while( this_lon <= -180 ) this_lon += 360;
  while( this_lon  >  180 ) this_lon -= 360;
  
  if( this_lat < -90 ) this_lat = -90;
  if( this_lat >  90 ) this_lat =  90;
  
  int lon_start = 10*floor(this_lon/10);
  int lat_start = 10*floor(this_lat/10);
  
  _lon_start = lon_start * dtr;
  _lat_start = lat_start * dtr;
  
  int ilon0 = (lon_start+180   )*120+1;
  int ilon1 = (lon_start+180+10)*120;
  int ilat0 = (lat_start+90    )*120+1;
  int ilat1 = (lat_start+90+10 )*120;
  
  char fullname[200];
  sprintf( fullname, "%s/%5.5d-%5.5d.%5.5d-%5.5d", usgs_dir, ilon0, ilon1, ilat0, ilat1 );
  //printf("%f %f %s\n",lon*rtd,lat*rtd,fullname);
  
  FILE* ifp = fopen(fullname, "r");
  if (ifp == NULL){
    fprintf(stderr,"LandMap::ExpandUSGS: Cannot open file %s\n",fullname);
    printf("%d %d %f %f\n",lat_start,lon_start,this_lat,this_lon);
    return(0);
  }
  
  if( _map == NULL && !_Allocate() ) {
    fprintf(stderr,"LandMap::ExpandUSGS: Error allocating _map\n");
    fclose(ifp);
    return(0);
  }
  
  for (int lat_idx = 0; lat_idx < _mapLatDim; lat_idx++) {
    if( fread((void*)*(_map+lat_idx),sizeof(char),_mapLonDim,ifp)!=_mapLonDim) {
      fprintf(stderr,"LandMap::ExpandUSGS: Error reading file %s\n",fullname);
      fclose(ifp);
      return(0);
    }
  }
  fclose(ifp);
  
  _lonResolution = 10.0*dtr / _mapLonDim;
  _latResolution = 10.0*dtr / _mapLatDim;
  
  // output flag value 
  int lon_idx = (int)((lon-_lon_start) / _lonResolution);
  int lat_idx = (int)((lat-_lat_start) / _latResolution);
  
  int flag = (int)*(*(_map + lat_idx) + lon_idx);
  if (flag != 16)
    return(1);
  else
    return(0);
}

// int LandMap::ExpandUSGS(float lon, float lat){
// 
//   // free old map
//   _Deallocate();
// 
//   
//   // compute new lat lon bounds
//   float minlat=MIN(_lat_start,lat);
//   float maxlat=MAX(_lat_start,lat);
// 
//   //while(lon<_lon_start-pi) lon+=two_pi;
//   //while(lon>_lon_start+pi) lon-=two_pi;
//   float minlon=MIN(_lon_start,lon);
//   float maxlon=MAX(_lon_start,lon);
//   _mapLonDim=int(ceil((maxlon-minlon)/USGS_BLOCK_SIZE))*USGS_BLOCK_PIXELS;
//   _mapLatDim=int(ceil((maxlat-minlat)/USGS_BLOCK_SIZE))*USGS_BLOCK_PIXELS;
//   if(_mapLonDim==0) _mapLonDim=USGS_BLOCK_PIXELS;
//   if(_mapLatDim==0) _mapLatDim=USGS_BLOCK_PIXELS;
//   _lon_start=minlon;
//   _lat_start=minlat;
//   _lat_start=floor(_lat_start/USGS_BLOCK_SIZE+0.00001)*USGS_BLOCK_SIZE;
//   _lon_start=floor(_lon_start/USGS_BLOCK_SIZE+0.00001)*USGS_BLOCK_SIZE;
//   
//   
//   _lat_start=floor(lat/USGS_BLOCK_SIZE+0.00001)*USGS_BLOCK_SIZE;
//   _lon_start=floor(lon/USGS_BLOCK_SIZE+0.00001)*USGS_BLOCK_SIZE;
// 
//   
//   
//   // reAllocate
//   _Allocate();
// 
//   // read in appropriate files
//   int ilon0,ilon1,ilat0,ilat1;
//   //ilon0=int((_lon_start+pi)*USGS_NLONS/two_pi + 0.0001) %USGS_NLONS+1;
//   ilon0=int((_lon_start+pi)*USGS_NLONS/two_pi + 0.001) %USGS_NLONS+1;
//   ilat0=int((_lat_start+pi/2)*USGS_NLATS/pi + 0.1)+1;
//   ilon1=ilon0+USGS_BLOCK_PIXELS-1;
//   ilat1=ilat0+USGS_BLOCK_PIXELS-1;
// 
//   int Nlatblocks=_mapLatDim/USGS_BLOCK_PIXELS;
//   int Nlonblocks=_mapLonDim/USGS_BLOCK_PIXELS;
// 
//   for(int i=0;i<Nlatblocks;i++){
//     for(int j=0;j<Nlonblocks;j++){
//       int ilo0=ilon0+j*USGS_BLOCK_PIXELS;
//       int ilo1=ilon1+j*USGS_BLOCK_PIXELS;
//       int ila0=ilat0+i*USGS_BLOCK_PIXELS;
//       int ila1=ilat1+i*USGS_BLOCK_PIXELS;
//       char fullname[200];
//       sprintf(fullname,"%s/%5.5d-%5.5d.%5.5d-%5.5d",usgs_dir,ilo0,ilo1,ila0,ila1);
//       FILE* ifp = fopen(fullname, "r");
//       if (ifp == NULL){
// 	fprintf(stderr,"Cannot open filename %s\n",fullname);
// 	return(0);
//       }
//       int latoff=i*USGS_BLOCK_PIXELS;
//       int lonoff=j*USGS_BLOCK_PIXELS;
//       int size = sizeof(char) * USGS_BLOCK_PIXELS;
//       for (int lat_idx = 0; lat_idx < USGS_BLOCK_PIXELS; lat_idx++)
// 	{
//           unsigned char* ptr = (*(_map+latoff+lat_idx)+lonoff);
// 	  if (fread((void *)ptr, size, 1, ifp) != 1)
// 	    {
// 	      fclose(ifp);
// 	      return(0);
// 	    }
// 	}
//       fclose(ifp);
//     }
//   }
//   // output flag value 
// 
//   int lon_idx = (int)((lon-_lon_start) / _lonResolution);
//   int lat_idx = (int)((lat-_lat_start) / _latResolution);
//   
//   int flag = (int)*(*(_map + lat_idx) + lon_idx);
//   if (flag != 16)
//     return(1);
//   else
//     return(0);
// 
// }

int LandMap::IsCoastal( float lon, float lat, float thresh ){
  if( landmap_type != 2 ) {
    fprintf(stderr,"Don't call LandMap::IsCoastal unless landmap_type==2!\n");
    return(0);
  }
  
  double dlat  = thresh;            // search size
  double dlon  = thresh / cos(lat); // scale by 1/cosine(lat)
  double delta = 10*dtr / 1200;     // pixel spacing of USGS landuse files
  
  int center_mask = IsLandUSGS( lon, lat );
  
  double this_lon = lon - dlon;
  double this_lat = lat - dlat;
  double wrapped_lon;
  if( this_lat < -pi ) this_lat = -pi;
  
  while( this_lat < lat + dlat && this_lat < pi ) {
    while( this_lon < lon + dlon ) {
      // Don't overwrite this_lon but make sure inputs to IsLand
      // are wrapped in range.
      wrapped_lon = this_lon;
      if( wrapped_lon < 0       ) wrapped_lon += two_pi;
      if( wrapped_lon >= two_pi ) wrapped_lon -= two_pi;
      // Quit as soon as we find something not the same.
      if( IsLandUSGS(wrapped_lon,this_lat) != center_mask )
        return(1);
      this_lon += delta*2;
    }
    this_lat += delta*2;
  }
  return(0);
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

//--------------------//
// LandMap::_Allocate //
//--------------------//

int
LandMap::_Allocate()
{
  if(landmap_type!=1)
    _map = (unsigned char**)make_array(sizeof(char), 2, _mapLatDim, _mapLonDim);
  else
    _map = (unsigned char**)make_array(sizeof(char), 2, _mapLonDim, _mapLatDim);
  if (! _map)
    return(0);
  return(1);
}

//----------------------//
// LandMap::_Deallocate //
//----------------------//

int
LandMap::_Deallocate()
{
  if(landmap_type!=1)
    free_array( (void*)_map, 2, _mapLatDim, _mapLonDim );
  else
    free_array( (void*)_map, 2, _mapLonDim, _mapLatDim );
  return(1);
}

//===============//
// SimpleLandMap //
//===============//

SimpleLandMap::SimpleLandMap()
  :   _map(NULL), _usemap(0), _lonSamples(0), _latSamples(0), _lonResolution(0.0),
    _latResolution(0.0)
{
    return;
}

SimpleLandMap::~SimpleLandMap()
{
    if (_map)
        _Deallocate();
    return;
}
//----------------------------//
// SimpleLandMap::Initialize  //
//----------------------------//

int
SimpleLandMap::Initialize(
    char*  filename,
    int    use_map)
{
    _usemap = use_map;
    if (_usemap != 0)
    {
        if (! Read(filename))
            return(0);
    }
    return(1);
}

//---------------------//
// SimpleLandMap::Read //
//---------------------//

int
SimpleLandMap::Read(
    const char*  filename)
{
    FILE* ifp = fopen(filename, "r");
    if (ifp == NULL)
        return(0);

    int lon_samples, lat_samples;
    if (fread((void *)&lon_samples, sizeof(int), 1, ifp) != 1 ||
        fread((void *)&lat_samples, sizeof(int), 1, ifp) != 1)
    {
        fclose(ifp);
        return(0);
    }

    _lonSamples = lon_samples;
    _latSamples = lat_samples;

    if (! _Allocate())
    {
        fclose(ifp);
        return(0);
    }

    int size = sizeof(char) * _latSamples;
    for (int lon_idx = 0; lon_idx < _lonSamples; lon_idx++)
    {
        if (fread((void *)*(_map + lon_idx), size, 1, ifp) != 1)
        {
            fclose(ifp);
            return(0);
        }
    }

    _lonResolution = two_pi / _lonSamples;
    _latResolution = pi / _latSamples;

    fclose(ifp);
    return(1);
}

//----------------------//
// SimpleLandMap::Write //
//----------------------//

int
SimpleLandMap::Write(
    const char*  filename)
{
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    if (fwrite((void *)&_lonSamples, sizeof(int), 1, ofp) != 1 ||
        fwrite((void *)&_latSamples, sizeof(int), 1, ofp) != 1)
    {
        fclose(ofp);
        return(0);
    }

    int size = sizeof(char) * _latSamples;
    for (int lon_idx = 0; lon_idx < _lonSamples; lon_idx++)
    {
        if (fwrite((void *)*(_map + lon_idx), size, 1, ofp) != 1)
        {
            fclose(ofp);
            return(0);
        }
    }

    fclose(ofp);
    return(1);
}

//------------------------//
// SimpleLandMap::GetType //
//------------------------//
// lon must be between 0 and +two_pi radians
// lat must be between -pi/2 and +pi/2 radians
// 0 is ocean
// 1 is land
// 2 is mixed

int
SimpleLandMap::GetType(
    float  lon,
    float  lat)
{
    while(lon<0) lon += two_pi;    // to make sure it is in range
    int lon_idx = (int)(lon / _lonResolution);
    int lat_idx = (int)((lat + pi_over_two) / _latResolution);
    lon_idx = lon_idx % _lonSamples;
    if (lat_idx < 0)
        lat_idx = 0;
    if (lat_idx >= _latSamples)
        lat_idx = _latSamples;

    int flag = *(*(_map + lon_idx) + lat_idx);
    return(flag);
}

//-----------------------//
// SimpleLandMap::IsLand //
//-----------------------//

int
SimpleLandMap::IsLand(
    float  lon,
    float  lat)
{
    if (_usemap == 0)
        return(0);

    int flag = GetType(lon, lat);
    if (flag == 1)
        return(1);
    else
        return(0);
}

//-------------------------//
// SimpleLandMap::Allocate //
//-------------------------//

int
SimpleLandMap::Allocate(
    int  lon_samples,
    int  lat_samples)
{
    _lonSamples = lon_samples;
    _latSamples = lat_samples;

    _lonResolution = two_pi / _lonSamples;
    _latResolution = pi / _latSamples;

    return(_Allocate());
}

//---------------------//
// SimpleLandMap::Fill //
//---------------------//

int
SimpleLandMap::Fill(
    char  value)
{
    for (int i = 0; i < _lonSamples; i++)
    {
        for (int j = 0; j < _latSamples; j++)
        {
            *(*(_map + i) + j) = value;
        }
    }
    return(1);
}

//--------------------------//
// SimpleLandMap::_Allocate //
//--------------------------//

int
SimpleLandMap::_Allocate()
{
    _map = (char**)make_array(sizeof(char), 2, _lonSamples,
        _latSamples);
    if (_map == NULL)
        return(0);
    return(1);
}

//----------------------------//
// SimpleLandMap::_Deallocate //
//----------------------------//

int
SimpleLandMap::_Deallocate()
{
    free_array((void*)_map, 2, _lonSamples, _latSamples);
    _map = NULL;
    return(1);
}

//----------------------------//
//  QSLandMap::QSLandMap()    //
//----------------------------//

 QSLandMap::QSLandMap() : _map(NULL) {
   return;
}

//-----------------------------//
//  QSLandMap::~QSLandMap()    //
//-----------------------------//

QSLandMap::~QSLandMap() {
  if( _map )
    _Deallocate();
  _map = NULL;
  return;
}

//-----------------------------//
//  QSLandMap::_Allocate()     //
//-----------------------------//

int QSLandMap::_Allocate() {
  _map = (unsigned char**)make_array(sizeof(char), 2, 5400, 10800 );
  if( _map == NULL ) 
    return(0);
  return(1);
}

//------------------------------//
//  QSLandMap::_Deallocate()    //
//------------------------------//

int QSLandMap::_Deallocate() {
  free_array( (void*)_map, 2, 5400, 10800 );
  return(1);
}

//-----------------------//
//  QSLandMap::Read()    //
//-----------------------//

int QSLandMap::Read( const char* filename) {
  if( !_Allocate() ) {
    fprintf(stderr,"QSLandMap::Read, Error allocating land map!\n");
    return(0);
  }

  FILE* ifp = fopen(filename, "r");
  if (ifp == NULL)
    return(0);
  
  char num_bytes_be[4], num_bytes_le[4];
  int  num_bytes;
  
  fread( &num_bytes_be, sizeof(char), 4, ifp );
  for( int ii = 0; ii < 4; ++ii ) num_bytes_le[ii] = num_bytes_be[3-ii];
  
  memcpy( &num_bytes, &num_bytes_le[0], 4 );
  
  if( num_bytes != 5400 * 10800 ) {
    fprintf(stderr,"In QSLandMap::Read: Error, unexpected size of land map %d\n",num_bytes);
    fclose(ifp);
    return(0);
  }
  
  for( int i_lat = 0; i_lat < 5400; ++i_lat ) {
    if( fread((void*)*(_map+i_lat), sizeof(char), 10800, ifp ) != 10800 ) {
      fclose(ifp);
      return(0);
    }
  }
  fclose(ifp);
  return(1);
}

//-------------------------//
//  QSLandMap::IsLand()    //
//-------------------------//

int QSLandMap::IsLand( float lon,      // radians
                       float lat,      // radians
                       int   flagging_mode ) {  

  // Check that _map is allocated
  if( _map == NULL )
    return(0);
  
  // Convert to degrees
  float lon_deg = lon * rtd;
  float lat_deg = lat * rtd;
  
  // Check that inputs are in range
  if( lon_deg       <   0 || lon_deg       >= 360 ||
      lat_deg       < -90 || lat_deg       >   90 ||
      flagging_mode <  -1 || flagging_mode >    2 ) {
    fprintf(stderr,"In QSLandMap::IsLand: Error inputs out of range!\n");
    return(0);
  }
  
  // indexing logic from offical processor
  int i_lat = floor( (90+lat_deg)*30  ); // 90 + lat_deg always >= 0 
  int i_lon = floor(     lon_deg *30  );
  
  // Wrap lon index.
  if( i_lon == 10800 ) i_lon = 0;
  // keep i_lat in range
  if( i_lat == 5400  ) i_lat = 5400-1;
  
  
  unsigned char bits;  
  if( flagging_mode == 0 )       // bit 6 used in L2B 25 km product
    bits = (unsigned char)0x40;
  else if ( flagging_mode == 1 ) // bit 4 used in L2B 12.5 km product
    bits = (unsigned char)0x10;
  else if ( flagging_mode == 2 ) // Use most restrictive bit
    bits = (unsigned char)0x80;
  else if ( flagging_mode!=-1){
    fprintf(stderr,"In QSLandMap::IsLand: Unknown value of flagging_mode: %d\n",
           flagging_mode);
    return(0);
  }
  
  if( flagging_mode==-1) // return whole byte to caller
    return((int)_map[i_lat][i_lon]);
  else {
    if( _map[i_lat][i_lon] & bits )
      return(1);
    else
      return(0);
  }
}

//--------------------------//
//  QSIceMap::QSIceMap()    //
//--------------------------//

QSIceMap::QSIceMap() : _map(NULL) {
   return;
} 

//---------------------------//
//  QSIceMap::~QSIceMap()    //
//---------------------------//

QSIceMap::~QSIceMap() {
  if( _map )
    _Deallocate();
  _map = NULL;
  return;
}

//-----------------------------//
//  QSIceMap::_Allocate()     //
//-----------------------------//

int QSIceMap::_Allocate() {
  _map = (unsigned char**)make_array(sizeof(char), 2, 360, 720 );
  if( _map == NULL ) 
    return(0);
  return(1);
}

//------------------------------//
//  QSIceMap::_Deallocate()     //
//------------------------------//

int QSIceMap::_Deallocate() {
  free_array( (void*)_map, 2, 360, 720 );
  return(1);
}

//-----------------------//
//  QSIceMap::Read()     //
//-----------------------//

int QSIceMap::Read( const char* filename) {
  if( !_Allocate() ) {
    fprintf(stderr,"QSIceMap::Read, Error allocating ice map!\n");
    return(0);
  }

  FILE* ifp = fopen(filename, "r");
  if (ifp == NULL)
    return(0);
  
  char num_bytes_be[4], num_bytes_le[4];
  int  num_bytes;
  
  fread( &num_bytes_be, sizeof(char), 4, ifp );
  for( int ii = 0; ii < 4; ++ii ) num_bytes_le[ii] = num_bytes_be[3-ii];
  
  memcpy( &num_bytes, &num_bytes_le[0], 4 );
  
  if( num_bytes != 360 * 720 ) {
    fprintf(stderr,"In QSIceMap::Read: Error, unexpected size of ice map %d\n",num_bytes);
    fclose(ifp);
    return(0);
  }
  
  for( int i_lat = 0; i_lat < 360; ++i_lat ) {
    if( fread((void*)*(_map+i_lat), sizeof(char), 720, ifp ) != 720 ) {
      fclose(ifp);
      return(0);
    }
  }
  fclose(ifp);
  return(1);
}


//-------------------------//
//  QSIceMap::IsIce()      //
//-------------------------//

int QSIceMap::IsIce( float lon,          // radians
                     float lat,          // radians
                     int   beam_idx ) {  // meas->beamIdx
  // Check that _map is allocated
  if( _map == NULL )
    return(0);
  
  // Convert to degrees
  float lon_deg = lon * rtd;
  float lat_deg = lat * rtd;
  
  // Check that inputs are in range
  if( lon_deg       <   0 || lon_deg       >= 360 ||
      lat_deg       < -90 || lat_deg       >   90 ||
      beam_idx      <   0 || beam_idx      >    1  ) {
    fprintf(stderr,"In QSLandMap::IsLand: Error inputs out of range!\n");
    return(0);
  }
  
  // indexing logic from offical processor
  int i_lat = floor( (90+lat_deg)*2  ); // 90 + lat_deg always >= 0 
  int i_lon = floor(     lon_deg *2  );
  
  // Wrap lon index.
  if( i_lon == 720 ) i_lon = 0;
  // keep i_lat in range
  if( i_lat == 360  ) i_lat = 360-1;
  
  unsigned char bits;
  if( beam_idx = 0 ) 
    bits = (unsigned char)0x1; // bit 0
  else if( beam_idx = 1 )
    bits = (unsigned char)0x2; // bit 1
  else {
    fprintf(stderr,"In QSIceMap::IsIce: Shouldn't be here!\n");
    exit(1);
  }
    
  if( _map[i_lat][i_lon] & bits )
    return(1);
  else
    return(0);
}


// ICECMap is a class for the binary files extracted from GRIB files from
// ftp://polar.ncep.noaa.gov/cdas or
// ftp://ftpprd.ncep.noaa.gov/pub/data/nccf/com/omb/prod/
//
// They are for lon in [0, 360], lat in [-90, 90] with 1/12 deg spacing
ICECMap::ICECMap() : _map(NULL) {
    return;
}

ICECMap::ICECMap(const char* filename) : _map(NULL) {
    Read(filename);
    return;
}

ICECMap::~ICECMap() {
    if(_map)
        _Deallocate();
    _map = NULL;
    return;
}

int ICECMap::_Allocate() {
    _map = (float**)make_array(sizeof(float), 2, 180*12, 360*12);
    if(_map == NULL)
        return(0);
    return(1);
}

int ICECMap::_Deallocate() {
    free_array((void*)_map, 2, 180*12, 360*12);
    return(1);
}

int ICECMap::Read(const char* filename) {
    if(!_Allocate()) {
        fprintf(stderr,"ICECMap::Read, Error allocating ice map!\n");
        return(0);
    }

    FILE* ifp = fopen(filename, "r");
    if (ifp == NULL) {
        fprintf(stderr,"ICECMap::Read, Error opening ice map!\n");
        return(0);
    }
    fseek(ifp, 4, SEEK_SET);
    for(int ilat = 0; ilat < 12*180; ++ilat) {
        if(fread((void*)*(_map+ilat), sizeof(float), 4320, ifp) != 4320) {
            fprintf(
                stderr, "ICECMap::Read, Error reading ice map at ilat: %d!\n",
                ilat);
            fclose(ifp);
            return(0);
        }
    }
    fclose(ifp);
    return(1);
}

int ICECMap::Get(float lon, float lat, float* ice_concentation) {

    // Convert to degrees
    float lon_deg = lon * rtd;
    float lat_deg = lat * rtd;

    // map has lon in [0, 360]
    while(lon_deg<0) lon_deg += 360;
    while(lon_deg>=360) lon_deg -= 360;

    int ilon = floor(lon_deg*12);
    int ilat = floor((lat_deg+90)*12);

    if(ilon == 12*360) ilon = 0;
    if(ilat == 12*180) ilat = 12*180 - 1;

    if(ilat > 0 || ilat < 12*180) {
        *ice_concentation = _map[ilat][ilon];
        return(1);
    } else {
        return(0);
    }
}

