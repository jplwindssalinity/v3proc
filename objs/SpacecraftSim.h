//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef SPACECRAFTSIM_H
#define SPACECRAFTSIM_H

static const char rcs_id_spacecraftsim_h[] =
	"@(#) $Id$";

#include "Spacecraft.h"


//======================================================================
// CLASSES
//		SpacecraftSim
//======================================================================

//======================================================================
// CLASS
//		SpacecraftSim
//
// DESCRIPTION
//		The SpacecraftSim object contains the information necessary to
//		simulate the orbit by operating on an Spacecraft object.  It is
//		used to set members of the Spacecraft object as if the
//		spacecraft were orbiting.
//======================================================================

#define		ORBIT_UPDATE_PERIOD		60.0

class SpacecraftSim
{
public:

	//--------------//
	// construction //
	//--------------//

	SpacecraftSim();
	~SpacecraftSim();

	//----------------//
	// initialization //
	//----------------//

	int		DefineOrbit(double semi_major_axis, double eccentricity,
				double inclination, double longitude_of_asc_node,
				double argument_of_perigee, double mean_anomaly);

	int		LocationToOrbit(double longitude, double latitude, int asc);

	//-------------------//
	// orbit propagation //
	//-------------------//

	int		UpdateOrbit(double time, Spacecraft* spacecraft);

	//---------------------//
	// setting and getting //
	//---------------------//

	double		GetLongitudeOfAscendingNode() { return (_bigOmega * rtd); };
	double		GetMeanAnomaly() { return (_l * rtd); };

protected:

	//-------------------//
	// orbit propagation //
	//-------------------//

	double	_Ecan(double mean_anom);

	//-----------//
	// variables //
	//-----------//

	double	_a;				// semi-major axis (km)
	double	_e;				// eccentricity
	double	_i;				// inclination (radians)
	double	_bigOmega;		// longitude of ascending node
	double	_littleOmega;	// argument of perigee
	double	_l;				// mean anomaly

	//-----------------------//
	// predigested variables //
	//-----------------------//

	double	_a_3;			// semi-major axis cubed
	double	_e_2;			// eccentricity squared

	double	_ascnodot;		// rate of regression of ascending node
	double	_periasdot;		// rate of precession of perigee
	double	_ameandot;		// perturbation of mean anomaly
	double	_eta;
	double	_pp;
	double	_pp_2;
	double	_cosi;
	double	_cosi_2;
	double	_sini_2;
	double	_gama;

	double	_G;
	double	_H;
};

#endif
