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
//
// NAME CONVENTIONS
//		To aid in identifying the reference frame an angle is defined in,
//		the following conventions are followed in here and in Beam.C:
//		look_angle and azimuth_angle pairs refer to standard spherical angles
//		in the antenna frame.
//		elevation or Em and azimuth pairs refer to the modifed spherical
//		angles used to access the beam pattern in the beam reference frame.
//======================================================================

#include "CoordinateSwitch.h"

enum PolE { NONE, V_POL, H_POL };

extern const char* beam_map[];

class Beam
{
public:

	//--------------//
	// construction //
	//--------------//

	Beam();
	~Beam();

	// Get and Set Beams separately.
	int		GetElectricalBoresight(double* look_angle, double* azimuth_angle);
	int		SetElectricalBoresight(double look_angle, double azimuth_angle);

	// Set mechanical reference (beam directions determined by pattern data)
	// Use the same inputs for each beam to have a consistent reference.
	int		SetMechanicalBoresight(double look_angle, double azimuth_angle);

	int		SetBeamPattern(int Nx, int Ny, int ix_zero, int iy_zero,
				double x_spacing, double y_spacing, float **power_gain);

	int		ReadBeamPattern(char* filename);
	int		WriteBeamPattern(char* filename);

	CoordinateSwitch	GetAntFrameToBeamFrame()
							{ return(_antFrameToBeamFrame); };
	CoordinateSwitch	GetBeamFrameToAntFrame()
							{ return(_beamFrameToAntFrame); };

	int	GetPowerGain(double look_angle, double azimuth_angle, float *gain);

	//-----------//
	// variables //
	//-----------//

	PolE	polarization;
	float	pulseWidth;		// pulse width in seconds
	float	timeOffset;		// seconds after prf for beam index 0

protected:

	//-----------//
	// variables //
	//-----------//

	double	_reference_lookAngle;
	double	_reference_azimuthAngle;

	// Beam pattern info
	double _electrical_boresight_Em;
	double _electrical_boresight_azimuth;
	int		_Nx;
	int		_Ny;
	int		_ix_zero;
	int		_iy_zero;
	double	_x_spacing;
	double	_y_spacing;
	float**	_power_gain;

	CoordinateSwitch	_antFrameToBeamFrame;
	CoordinateSwitch	_beamFrameToAntFrame;

};

#endif
