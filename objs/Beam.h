//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef BEAM_H
#define BEAM_H

static const char rcs_id_beam_h[] =
	"@(#) $Id$";

//======================================================================
// CLASSES
//		Beam
//======================================================================

//======================================================================
// CLASS
//		Beam
//
// DESCRIPTION
//		The Beam object contains beam state information and fixed
//		coordinate transforms related to the beam "mounting" on the
//		antenna.
//======================================================================

#include "CoordinateSwitch.h"

enum PolE { NONE, V_POL, H_POL };

class Beam
{
public:

	//--------------//
	// construction //
	//--------------//

	Beam();
	~Beam();

	int		SetBeamGeometry(double look_angle, double azimuth_angle);

	//---------//
	// getting //
	//---------//

	CoordinateSwitch	GetAntFrameToBeamFrame()
							{ return(_antFrameToBeamFrame); };
	CoordinateSwitch	GetBeamFrameToAntFrame()
							{ return(_beamFrameToAntFrame); };

	//-----------//
	// variables //
	//-----------//

	PolE	polarization;
	double	timeOffset;			// seconds after prf for beam index 0

protected:

	//-----------//
	// variables //
	//-----------//

	double	_lookAngle;
	double	_azimuthAngle;

	CoordinateSwitch	_antFrameToBeamFrame;
	CoordinateSwitch	_beamFrameToAntFrame;
};

#endif
