//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef ANTENNASIM_H
#define ANTENNASIM_H

static const char rcs_id_antennasim_h[] =
	"@(#) $Id$";

//======================================================================
// CLASSES
//		AntennaState
//		AntennaSim
//======================================================================

//======================================================================
// CLASS
//		AntennaState
//
// DESCRIPTION
//		The AntennaState object contains the state of the antenna.
//======================================================================

enum BeamE { BEAM_A, BEAM_B, BEAMS };

class AntennaState
{
public:

	//--------------//
	// construction //
	//--------------//

	AntennaState();
	~AntennaState();

	//-----------//
	// variables //
	//-----------//

	double	lookAngle[BEAMS];
	double	azimuthAngle[BEAMS];
};

//======================================================================
// CLASS
//		AntennaSim
//
// DESCRIPTION
//		The AntennaSim object contains an antenna "simulator".
//======================================================================

class AntennaSim
{
public:

	//--------------//
	// construction //
	//--------------//

	AntennaSim();
	~AntennaSim();

	//----------------//
	// initialization //
	//----------------//

	int		DefineAntenna(double spin_rate, double look_angle_a,
				double look_angle_b, double azimuth_angle_a,
				double azimuth_angle_b);

	//---------------------//
	// antenna information //
	//---------------------//

	AntennaState	GetAntennaState(double time);

protected:

	//-----------//
	// variables //
	//-----------//

	double	_spinRate;			// radians per second
	double	_lookAngle[BEAMS];
	double	_azimuthAngle[BEAMS];

	//-------------------//
	// the antenna state //
	//-------------------//

	AntennaState	_antennaState;
};

#endif
