//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

#ifndef ORBITSIM_H
#define ORBITSIM_H

static const char rcs_id_orbitsim_h[] =
	"@(#) $Id$";

//======================================================================
// CLASSES
//		OrbitSim
//======================================================================

//======================================================================
// CLASS
//		OrbitSim
//
// DESCRIPTION
//		The OrbitSim object contains orbital parameters and an orbital
//		simulator.
//======================================================================

#define DTR		1.745329252e-2
#define TWOPI	6.283185308

#define RJ2		1.08260E-3

class OrbitSim
{
public:

	//--------------//
	// construction //
	//--------------//

	OrbitSim(double semi_major_axis, double eccentricity, double inclination,
		double argument_of_perigee);
	~OrbitSim();

	int		ScState(double time);

	//----------------//
	// initialization //
	//----------------//

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

	double	_gc_xyz[3];		// geocentric x
	double	_gc_hll[3];		// geocentric height, longitude, latitude
	double	_vel[3];		// s/c velocity in rotating Earth frame

	//-----------------------//
	// predigested variables //
	//-----------------------//

	double	_a_3;			// semi-major axis cubed
	double	_e_2;			// eccentricity squared
};

#endif
