//==========================================================//
// Copyright (C) 2012, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_os2xfix_c[] =
	"@(#) $Id:";

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>
#include <vector>
#include "OS2XFix.h"
#include "Constants.h"

OS2XFix::OS2XFix() : _table_read(0) {
  return;
}

OS2XFix::~OS2XFix() {
  return;
}

int OS2XFix::ReadTable(const char* filename ) {
  
  FILE* ifp = fopen( filename, "r" );

  _dx_table.resize(2);
  
  // Table has outer beam adjustment first (VV)
  fread( &_n_slices[1], sizeof(int), 1, ifp );
  fread( &_n_scans[1],  sizeof(int), 1, ifp );
  fread( &_n_frames[1], sizeof(int), 1, ifp );
  _dx_table[1].resize(_n_slices[1]);
  for( int i_slice = 0; i_slice < _n_slices[1]; ++i_slice )  {
    _dx_table[1][i_slice].resize(_n_scans[1]);
    for( int i_scan = 0; i_scan < _n_scans[1]; ++i_scan ) {
      _dx_table[1][i_slice][i_scan].resize(_n_frames[1]);
      fread( &(_dx_table[1][i_slice][i_scan][0]), sizeof(float), _n_frames[1], ifp );
    }
  }
  
  // Table has inner beam adjustment second (HH)
  fread( &_n_slices[0], sizeof(int), 1, ifp );
  fread( &_n_scans[0],  sizeof(int), 1, ifp );
  fread( &_n_frames[0], sizeof(int), 1, ifp );
  _dx_table[0].resize(_n_slices[0]);
  for( int i_slice = 0; i_slice < _n_slices[0]; ++i_slice )  {
    _dx_table[0][i_slice].resize(_n_scans[0]);
    for( int i_scan = 0; i_scan < _n_scans[0]; ++i_scan ) {
      _dx_table[0][i_slice][i_scan].resize(_n_frames[0]);
      fread( &(_dx_table[0][i_slice][i_scan][0]), sizeof(float), _n_frames[0], ifp );
    }
  }
  fclose(ifp);
  _table_read = 1;
  return(1);
}

int OS2XFix::FixIt( int i_pol, int i_slice, int i_scan, int i_frame, 
  double* xf, double* s0, double* snr ) {
  if( !_table_read ) {
    fprintf(stderr,"OS2XFix::FixIt: Table not read yet!\n");
    return(0);
  }
  // Check that we are in bounds of lookup table
  if( i_slice < 0 || i_slice > _n_slices[i_pol] ||
      i_scan  < 0 || i_scan  > _n_scans[i_pol]  ||
      i_frame < 0 || i_frame > _n_frames[i_pol] ) {
    return(0); // do nothing if out-of-bounds
  }  
  *xf  += _dx_table[i_pol][i_slice][i_scan][i_frame];
  *s0  -= _dx_table[i_pol][i_slice][i_scan][i_frame];
  *snr -= _dx_table[i_pol][i_slice][i_scan][i_frame];
  return(1);
}  
