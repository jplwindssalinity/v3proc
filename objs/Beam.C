//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_beam_c[] =
	"@(#) $Id$";

#include "Beam.h"

//======//
// Beam //
//======//

Beam::Beam()
:	polarization(NONE), timeOffset(0.0),
	_lookAngle(0.0), _azimuthAngle(0.0)
{
	return;
}

Beam::~Beam()
{
	return;
}

//-----------------------//
// Beam::SetBeamGeometry //
//-----------------------//

int
Beam::SetBeamGeometry(
	double	look_angle,
	double	azimuth_angle)
{
	//------------------------//
	// copy passed parameters //
	//------------------------//

	_lookAngle = look_angle;
	_azimuthAngle = azimuth_angle;

	//----------------------------------------------------//
	// generate forward and reverse coordinate transforms //
	//----------------------------------------------------//

	Attitude beam_frame;
	beam_frame.Set(0.0, _lookAngle, _azimuthAngle, 3, 2, 1);

	_antFrameToBeamFrame.SetRotation(beam_frame);
	_beamFrameToAntFrame = _antFrameToBeamFrame.ReverseDirection();

	return(1);
}
