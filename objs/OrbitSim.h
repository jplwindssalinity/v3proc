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

	//----------------//
	// initialization //
	//----------------//

	int		Initialize(double sc_lon, double sc_lat);

protected:

	//-------------------//
	// orbit propagation //
	//-------------------//

	int		_Update(double seconds);

	double	_Ecan(double eccen, double mean_anom);

	//-----------//
	// variables //
	//-----------//

	double	_a;				// semi-major axis (km)
	double	_e;				// eccentricity
	double	_i;				// inclination (radians)
	double	_bigOmega;		// longitude of ascending node
	double	_littleOmega;	// argument of perigee
	double	_l;				// mean anomaly

	double	_semilatex;
	double	_gamma;
	double	_rm;
	double	_eta;
	double	_cosi;
	double	_sini;
	double	_asc;		// ??? (double???)
	double	_xmu;
	double	_ameandot;
	double	_periasdot;
	double	_ascnodot;
	double	_bp;
};

#endif
