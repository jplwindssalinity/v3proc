//==========================================================//
// Copyright (C) 2010, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef ATTENMAP_H
#define ATTENMAP_H

static const char rcs_id_attenmap_h[] =
    "@(#) $Id$";
//======================================================================
// CLASSES
//    AttenMap
//======================================================================


//======================================================================
// CLASS
//    AttenMap
//
// DESCRIPTION
//    The LandMap object contains a lon, lat, monthly map of 
//    nadir attenuation in dB.  There are methods for reading in the 
//    attenuation map and getting an attenuation value.
//======================================================================

#include "LonLat.h"
#include <stdlib.h>

class AttenMap
{
public:

    //--------------//
    // construction //
    //--------------//

    AttenMap();
    ~AttenMap();

	//--------------//
	// input/output //
	//--------------//

	int ReadWentzAttenMap(const char* filename);
	
	//--------//
	// access //
	//--------//
    
    float GetNadirAtten(double longitude, double latitude, double sec_year );
    // overloaded; will use _sec_year as 3rd argument to GetNadirAtten
    float GetNadirAtten(double longitude, double latitude );
    int   SetSecYear( double sec_year );
    
protected:

	//--------------//
	// construction //
	//--------------//

	int		_Allocate();
	int		_Deallocate();
	
    //-----------//
    // variables //
    //-----------//
    
    float            _sec_year;
    unsigned char*** _map;
};

#endif