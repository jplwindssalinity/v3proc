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
//		OrbitState
//		OrbitSim
//======================================================================

//======================================================================
// CLASS
//		OrbitState
//
// DESCRIPTION
//		The OrbitState object contains the orbit state.
//======================================================================

class OrbitState
{
public:

	//--------------//
	// construction //
	//--------------//

	OrbitState();
	~OrbitState();

	//-----------//
	// variables //
	//-----------//

	double	gc_altitude;
	double	gc_longitude;
	double	gc_latitude;
	double	gc_vector[3];
    double  velocity_vector[3];
};

//======================================================================
// CLASS
//		OrbitSim
//
// DESCRIPTION
//		The OrbitSim object contains orbital parameters and an orbital
//		simulator.
//======================================================================

#define DTR		1.745329252e-2
#define RTD		5.729577951e1

class OrbitSim
{
public:

	//--------------//
	// construction //
	//--------------//

	OrbitSim(double semi_major_axis, double eccentricity, double inclination,
		double longitude_of_asc_node, double argument_of_perigee,
		double mean_anomaly);
	~OrbitSim();

	int			LocationToOrbit(double longitude, double latitude, int asc);
	OrbitState	GetOrbitState(double time);

	//---------------------//
	// setting and getting //
	//---------------------//

	double		GetLongitudeOfAscendingNode() { return (_bigOmega * RTD); };
	double		GetMeanAnomaly() { return (_l * RTD); };

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

	//-----------------//
	// the orbit state //
	//-----------------//

	OrbitState	_orbitState;
};

#endif
