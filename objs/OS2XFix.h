//==========================================================//
// Copyright (C) 2012, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_os2xfix_h[] =
	"@(#) $Id:";

#ifndef OS2XFIX_H
#define OS2XFIX_H

//======================================================================
// CLASSES
//    OS2Fix
//======================================================================


//======================================================================
// CLASS
//    OS2Fix
//
// DESCRIPTION
//    Applies various corrections to the X-Factor from OSCAT L1B data
//======================================================================

#include <stdlib.h>
#include <vector>

class OS2XFix
{
public:
    //--------------//
    // construction //
    //--------------//
  OS2XFix();
  ~OS2XFix();

	//--------------//
	// input/output //
	//--------------//
  
  int ReadTable(const char* filename);
  
	//--------//
	// access //
	//--------//
  
  int FixIt( int i_pol,  int i_slice, int i_scan, int i_frame, 
             double* xf, double* s0 );
      
protected:

private:
    //-----------//
    // variables //
    //-----------//
  int _n_pol;
  int _n_slices[2];
  int _n_scans[2];
  int _n_frames[2];

  std::vector< std::vector< std::vector< std::vector< float > > > > _dx_table;
  int _table_read;
};

#endif

