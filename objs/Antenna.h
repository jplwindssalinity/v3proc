//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef ANTENNA_H
#define ANTENNA_H

static const char rcs_id_antenna_h[] =
	"@(#) $Id$";

#include "CoordinateSwitch.h"
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

	CoordinateSwitch	GetAntPedToScBody()
							{ return(_antPedToScBody); };
	CoordinateSwitch	GetScBodyToAntPed()
							{ return(_scBodyToAntPed); };
	Beam*	GetCurrentBeam();

	//-------------//
	// conversions //
	//-------------//

	int		GetEncoderValue();
	int		SetAzimuthWithEncoder(int encoder);

	//-----------//
	// variables //
	//-----------//

	int			numberOfBeams;
	double		priPerBeam;		// seconds
	Beam		beam[MAX_NUMBER_OF_BEAMS];
	Attitude	antennaFrame;	// relative to s/c
	double		azimuthAngle;	// antenna azimuth angle
	double		spinRate;		// radians per second

	int			currentBeamIdx;	// index of current beam

protected:

	//-----------//
	// variables //
	//-----------//

	int					_numberOfEncoderBits;
	double				_angularResolution;		// radians
	CoordinateSwitch	_antPedToScBody;
	CoordinateSwitch	_scBodyToAntPed;
};

#endif
