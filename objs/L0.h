//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L0_H
#define L0_H

static const char rcs_id_l0_h[] =
	"@(#) $Id$";

#include "Product.h"

//======================================================================
// CLASSES
//		L0File, L0FileList, L0
//======================================================================

#define L0_DATA_REC_SIZE	92


//======================================================================
// CLASS
//		L0File
//
// DESCRIPTION
//		The L0File object is used to read and write Level 0 data.
//======================================================================

class L0File : public ProductFile
{
public:

	//--------------//
	// construction //
	//--------------//

	L0File();
	~L0File();
};


//======================================================================
// CLASS
//		L0
//
// DESCRIPTION
//		The L0 object is used to manipulate Level 0 data.
//======================================================================

class L0 : public Product
{
public:

	//-------//
	// enums //
	//-------//

	enum L0BeamE { NONE, SCATTEROMETER_BEAM_A, SCATTEROMETER_BEAM_B };

	//--------------//
	// construction //
	//--------------//

	L0();
	~L0();

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
	L0BeamE		beam;
};

#endif
