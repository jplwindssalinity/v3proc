//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef MEASUREMENT_H
#define MEASUREMENT_H

static const char rcs_id_measurement_h[] =
	"@(#) $Id$";

//======================================================================
// CLASSES
//		Measurement
//======================================================================

//======================================================================
// CLASS
//		Measurement
//
// DESCRIPTION
//		The Measurement object is a general purpose object for
//		holding and manipulating sigma-0 and brightness temperature
//		data.
//======================================================================

class Measurement
{
public:

	//--------------//
	// construction //
	//--------------//

	Measurement();
	~Measurement();

	//-----------//
	// variables //
	//-----------//

	double		value;			// sigma-0 or temperature measurement...
	double		valuedB;		// ...and in dB
	double		incidenceAngle;

	double		centerLongitude;
	double		centerLatitude;
};

#endif
