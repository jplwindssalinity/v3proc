//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L15_H
#define L15_H

static const char rcs_id_l15_h[] =
	"@(#) $Id$";

#include "Product.h"

//======================================================================
// CLASSES
//		L15File, L15FileList, L15
//======================================================================

#define L15_DATA_REC_SIZE	100


//======================================================================
// CLASS
//		L15File
//
// DESCRIPTION
//		The L15File object is used to read and write Level 1.5 data.
//======================================================================

class L15File : public ProductFile
{
public:

	//--------------//
	// construction //
	//--------------//

	L15File();
	~L15File();
};


//======================================================================
// CLASS
//		L15
//
// DESCRIPTION
//		The L15 object is used to manipulate Level 1.5 data.
//======================================================================

class L15 : public Product
{
public:

	//-------//
	// enums //
	//-------//

	enum L15BeamE { NONE, SCATTEROMETER_BEAM_A, SCATTEROMETER_BEAM_B };

	//--------------//
	// construction //
	//--------------//

	L15();
	~L15();

	//-------------------//
	// data manipulation //
	//-------------------//
 
	int		PackFrame();
	int		UnpackFrame();

	//-------------------//
	// product variables //
	//-------------------//

	double		time;

	double		gcAltitude;
	double		gcLongitude;
	double		gcLatitude;
	double		gcX;
	double		gcY;
	double		gcZ;
	double		velX;
	double		velY;
	double		velZ;

	double		antennaPosition;
	L15BeamE	beam;
	double		sigma_0;
};

#endif
