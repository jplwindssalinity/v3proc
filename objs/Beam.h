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
//		antenna.  The beam antenna pattern is also held by this object.
//		The beam pattern has to be loaded from an external file.
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
	int		SetBeamPattern(int Nx, int Ny, int ix_zero, int iy_zero,
				double x_spacing, double y_spacing, float **power_gain);

	int		ReadBeamPattern(char* filename,
			 double out_of_range_value);
	int		WriteBeamPattern(char* filename);

	//---------//
	// getting //
	//---------//

	CoordinateSwitch	GetAntFrameToBeamFrame()
							{ return(_antFrameToBeamFrame); };
	CoordinateSwitch	GetBeamFrameToAntFrame()
							{ return(_beamFrameToAntFrame); };
	double	GetPowerGain(double unitx, double unity);

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

	// Beam pattern info
	int		_Nx;
	int		_Ny;
	int		_ix_zero;
	int		_iy_zero;
	double	_x_spacing;
	double	_y_spacing;
	double	_out_of_range_value;
	float**	_power_gain;

	CoordinateSwitch	_antFrameToBeamFrame;
	CoordinateSwitch	_beamFrameToAntFrame;

};

#endif
