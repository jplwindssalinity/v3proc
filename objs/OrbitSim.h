//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef ORBITSIM_H
#define ORBITSIM_H

static const char rcs_id_orbitsim_h[] =
	"@(#) $Id$";

#include "Orbit.h"


//======================================================================
// CLASSES
//		OrbitSim
//======================================================================

//======================================================================
// CLASS
//		OrbitSim
//
// DESCRIPTION
//		The OrbitSim object contains the information necessary to
//		simulate an orbit by operating on an Orbit object.  It is
//		used to set members of the Orbit object as if the instrument
//		were orbiting.
//======================================================================

class OrbitSim
{
public:

	//--------------//
	// construction //
	//--------------//

	OrbitSim();
	~OrbitSim();

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

	int		UpdateOrbit(double time, Orbit* orbit);

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
