//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

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

	double			EncoderToAngle(unsigned int encoder_value);
	unsigned int	AngleToEncoder(double angle);

	int				SetNumberOfEncoderValues(unsigned int number);
	int				SetAzimuthWithEncoder(unsigned int encoder_value);

	Beam*			GetCurrentBeam();
	unsigned int	GetEarlyEncoderValue();
	unsigned int	GetEncoderValue();
	unsigned int	GetEncoderN() { return(_numberOfEncoderValues); };
	double			GetEarlyDeltaAzimuth();
	double			GetAntennaFraction();

	CoordinateSwitch	GetAntPedToScBody()
							{ return(_antPedToScBody); };
	CoordinateSwitch	GetScBodyToAntPed()
							{ return(_scBodyToAntPed); };

	//-----------//
	// variables //
	//-----------//

	int				numberOfBeams;
	double			priPerBeam;		// seconds
	Beam			beam[MAX_NUMBER_OF_BEAMS];
	Attitude		antennaFrame;	// relative to s/c
	double			azimuthAngle;	// antenna azimuth angle

	double			commandedSpinRate;		// rad/second
	double			actualSpinRate;			// rad/second

	unsigned int	encoderAOffsetDn;	// dn
	double			encoderDelay;		// seconds

	int				currentBeamIdx;	// index of current beam

protected:

	//-----------//
	// variables //
	//-----------//

	unsigned int		_numberOfEncoderValues;
	CoordinateSwitch	_antPedToScBody;
	CoordinateSwitch	_scBodyToAntPed;
};

#endif
