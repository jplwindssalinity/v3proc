//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

#ifndef SPACECRAFTSIM_H
#define SPACECRAFTSIM_H

static const char rcs_id_spacecraftsim_h[] =
	"@(#) $Id$";

#include "Spacecraft.h"
#include "Distributions.h"

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

#define EQX_TIME_TOLERANCE		0.1		// seconds
#define EQX_ARG_OF_LAT			0.0
#define SOUTH_ARG_OF_LAT		(1.5*pi)

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

	int		Initialize(double start_time);

	int		DefineOrbit(double epoch, double semi_major_axis,
				double eccentricity, double inclination,
				double longitude_of_asc_node, double argument_of_perigee,
				double mean_anomaly_at_epoch);

	int		LocationToOrbit(double longitude, double latitude, int asc);

	//-------------------//
	// orbit propagation //
	//-------------------//

	int		UpdateOrbit(double time, Spacecraft* spacecraft);
	int		UpdateAttitude(double time, Spacecraft* spacecraft);

	//---------------------//
	// setting and getting //
	//---------------------//


	void	SetEphemerisPeriod(double period) { _ephemerisPeriod = period; };
	double		GetEphemerisPeriod() { return(_ephemerisPeriod); };
	double		GetLongitudeOfAscendingNode() { return (_bigOmega * rtd); };
	double		GetMeanAnomaly() { return (_l * rtd); };
	double		GetPeriod() { return (_period); };
	double		GetEpoch() { return (_epoch); };
	double		GetArgOfLat(Spacecraft* spacecraft);

	//--------//
	// events //
	//--------//

	int		DetermineNextEvent(SpacecraftEvent* spacecraft_event);
	double	FindNextArgOfLatTime(double time, double target_arg_of_lat,
				double time_tol);
	double	FindPrevArgOfLatTime(double time, double target_arg_of_lat,
				double time_tol);

	//----------------------------//
	// Attitude Reporting Routine //
	//----------------------------//

	void	ReportAttitude(double time, Spacecraft* spacecraft,
				Attitude* attitude);

	//------------------//
	// public variables //
	//------------------//

	AttDist attCntlDist;	// Attitude Control Distribution
	AttDist attKnowDist;	// Attitude Knowledge Distribution

	//-------//
	// flags //
	//-------//

	int		simKprsFlag;	// 0 = no knowledge error, 1 = knowledge error

protected:

	//-------------------//
	// orbit propagation //
	//-------------------//

	double	_Ecan(double mean_anom);

	//-----------//
	// ephemeris //
	//-----------//

	double	_ephemerisPeriod;	// time between orbit state reports

	//-----------//
	// variables //
	//-----------//

	double	_epoch;			// epoch (time for mean anomaly)
	double	_a;				// semi-major axis (km)
	double	_e;				// eccentricity
	double	_i;				// inclination (radians)
	double	_bigOmega;		// longitude of ascending node
	double	_littleOmega;	// argument of perigee
	double	_l;				// mean anomaly

	double	_period;		// the orbit period

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
	// event variables //
	//-----------------//

	double		_nextUpdateTime;		// time of next orbit state update
	double		_nextEqxTime;			// time of next equator crossing
};

#endif
