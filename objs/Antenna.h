//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef ANTENNA_H
#define ANTENNA_H

static const char rcs_id_antenna_h[] =
	"@(#) $Id$";

#include "Beam.h"
#include "Attitude.h"

#define MAX_NUMBER_OF_BEAMS		10


//======================================================================
// CLASSES
//		Antenna
//======================================================================

//======================================================================
// CLASS
//		Antenna
//
// DESCRIPTION
//		The Antenna object contains antenna information.  It represents
//		the actual state of the antenna.
//======================================================================

class Antenna
{
public:

	//--------------//
	// construction //
	//--------------//

	Antenna();
	~Antenna();

	int		SetPedestalAttitude(Attitude* attitude);

	//-----------------//
	// setting/getting //
	//-----------------//

	int		SetNumberOfEncoderBits(int number);

	CoordinateSwitch	GetAntPedToAntFrame()
							{ return(_antPedToAntFrame); };
	CoordinateSwitch	GetAntFrameToAntPed()
							{ return(_antFrameToAntPed); };

	//-------------//
	// conversions //
	//-------------//

	int		GetEncoderValue();
	int		SetAzimuthWithEncoder(int encoder);

	//-----------//
	// variables //
	//-----------//

	int			numberOfBeams;
	double		priPerBeam;
	Beam		beam[MAX_NUMBER_OF_BEAMS];
	Attitude	antennaFrame;	// relative to s/c
	double		azimuthAngle;	// antenna azimuth angle

protected:

	//-----------//
	// variables //
	//-----------//

	int					_numberOfEncoderBits;
	double				_angularResolution;
	CoordinateSwitch	_antPedToAntFrame;
	CoordinateSwitch	_antFrameToAntPed;
};

#endif
