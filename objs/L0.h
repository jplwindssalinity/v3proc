//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef L0_H
#define L0_H

static const char rcs_id_l0_h[] =
	"@(#) $Id$";

//======================================================================
// CLASS
//		L0
//
// DESCRIPTION
//		The L0 object contains the Level 0 telemetry.
//======================================================================

class L0
{
public:

	//------//
	// enum //
	//------//

	enum BeamE { NONE, SCATTEROMETER_BEAM_A, SCATTEROMETER_BEAM_B };

	//-------------//
	// contruction //
	//-------------//

	L0();
	~L0();

	//--------------//
	// input/output //
	//--------------//

	int		OpenForWriting(const char* output_file);
	int		WriteDataRec();

	//-----------//
	// variables //
	//-----------//

	double			time;

	double			gcAltitude;
	double			gcLongitude;
	double			gcLatitude;
	double			gcX;
	double			gcY;
	double			gcZ;
	double			velX;
	double			velY;
	double			velZ;

	double			antennaPosition;
	unsigned char	beam;

protected:

	//-----------//
	// variables //
	//-----------//

	int		_ofd;
};

#endif
