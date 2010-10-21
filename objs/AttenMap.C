//==========================================================//
// Copyright (C) 2010, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_attenmap_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>
#include "Array.h"
#include "Constants.h"
#include "AttenMap.h"
#include "ETime.h"

AttenMap::AttenMap() : _map(NULL) {
  return;
}

AttenMap::~AttenMap() {
  _Deallocate();
  _map = NULL;
  return;
}

int AttenMap::_Allocate() {
  _map = (unsigned char***)make_array(sizeof(char), 3, 12, 180, 360 );
  if( _map == NULL ) 
    return(0);
  return(1);
}

int AttenMap::_Deallocate() {
  free_array( (void*)_map, 3, 12, 180, 360 );
  return(1);
}

int AttenMap::ReadWentzAttenMap(const char* filename) {

  if( !_Allocate() ) {
    fprintf(stderr,"Error allocating attenuation map!\n");
    return(0);
  }

  FILE* fid = fopen( filename, "r" );
  if( fid == NULL )
    return(0);
  
  for( int i_mm = 0; i_mm < 12; ++i_mm ) {
    for( int i_lat = 0; i_lat < 180; ++i_lat ) {
      if( fread((void*)*(*(_map+i_mm)+i_lat), sizeof(char), 360, fid ) != 360 ) {
        fclose(fid);
        return(0);
      }
    }
  }
  fclose(fid);
  return(1);
}

// returns the nadir attenuation in dB; must divide by the cosine of the 
// incidence angle to get approximate path-length attenuation in dB.
// Verified by getting slice_lon, slice_lat, atten_from_map in L2A and
// comparing to what this method returns. Rms difference is 0.003 dB, and 
// the atten_from_map table is quantized at 0.01 dB.

// longitude, latitude in radians
// sec_year is seconds since the start of the current year.
float AttenMap::GetNadirAtten( double longitude, double latitude, double sec_year )
{
  if( _map == NULL ) {
    fprintf(stderr,"AttenMap::GetNadirAtten: Error, atten map not loaded!\n");
    return(0.0);
  }

  double lon_deg = longitude * rtd;
  double lat_deg = latitude  * rtd; 
  
  // wrap lon to [0,360) interval
  if( lon_deg < 0 ) lon_deg += 360;
    
  // Check bounds
  if( sec_year < 0   || sec_year >  366 * 86400 ||
      lon_deg  < 0   || lon_deg  >= 360         ||
      lat_deg  < -90 || lat_deg  >  90    ) {
    fprintf(stderr,"AttenMap::GetNadirAtten: Error, inputs out of range!\n");
    return(0.0); // 0 means no attenuation will be applied (in dB).
  }
  
  // Code based on Attenuation_From_Map.F in the QCSAT MGDR source.
  // note that I subtract one from the array indices before using them...
  double  brief = ( sec_year - 1314900) / 2629800;
  int     i1    = floor( 1 + brief );
  int     i2    = i1 + 1;
  double  a1    = i1 - brief;
  double  a2    = 1  - a1;
  
  if( i1 == 0 ) i1 = 12;
  if( i2 == 13) i2 = 1;
  
  brief     = lat_deg + 89.5;
  int    j1 = floor( 1 + brief );
  int    j2 = j1 + 1;
  double b1 = j1 - brief;
  double b2 = 1  - b1;
  
  if( j1 == 0 )   j1 = 1;
  if( j2 == 181 ) j2 = 180;
  
  brief     = lon_deg - 0.5;
  int    k1 = floor( 1 + brief );
  int    k2 = k1 + 1;
  double c1 = k1 - brief;
  double c2 = 1 - c1;
  
  if( k1 == 0 )   k1 = 360;
  if( k2 == 361 ) k2 = 1;
  
  float atten = 0.002 * (
        a1 * b1 * ( c1 * _map[i1-1][j1-1][k1-1] + c2 * _map[i1-1][j1-1][k2-1] ) +
        a1 * b2 * ( c1 * _map[i1-1][j2-1][k1-1] + c2 * _map[i1-1][j2-1][k2-1] ) +
        a2 * b1 * ( c1 * _map[i2-1][j1-1][k1-1] + c2 * _map[i2-1][j1-1][k2-1] ) +
        a2 * b2 * ( c1 * _map[i2-1][j2-1][k1-1] + c2 * _map[i2-1][j2-1][k2-1] ) );
  
  //printf("sec_year, lon, lat, atten: %20.12f %f %f %f\n", sec_year, lon_deg, lat_deg, atten );
  return(atten);
}