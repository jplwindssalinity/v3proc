//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L1_H
#define L1_H

static const char rcs_id_l1_h[] =
	"@(#) $Id$";

#include "Product.h"

//======================================================================
// CLASSES
//		L1File, L1FileList, L1
//======================================================================

#define L1_DATA_REC_SIZE	92


//======================================================================
// CLASS
//		L1File
//
// DESCRIPTION
//		The L1File object is used to read and write Level 1 data.
//======================================================================

class L1File : public ProductFile
{
public:

	//--------------//
	// construction //
	//--------------//

	L1File();
	~L1File();
};


//======================================================================
// CLASS
//		L1
//
// DESCRIPTION
//		The L1 object is used to manipulate Level 1 data.
//======================================================================

class L1 : public Product
{
public:

	//-------//
	// enums //
	//-------//

	enum L1BeamE { NONE, SCATTEROMETER_BEAM_A, SCATTEROMETER_BEAM_B };

	//--------------//
	// construction //
	//--------------//

	L1();
	~L1();

	//--------------//
	// input/output //
	//--------------//

	int		WriteDataRec();

	//-----------//
	// insertion //
	//-----------//

	int		InsertAll();

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
	L1BeamE		beam;
};

#endif
