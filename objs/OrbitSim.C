//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_orbitsim_c[] =
	"@(#) $Id$";

#include <math.h>
#include "OrbitSim.h"

//==========//
// OrbitSim //
//==========//

OrbitSim::OrbitSim(
	double	semi_major_axis,
	double	eccentricity,
	double	inclination,
	double	argument_of_perigee)
{
	_a = semi_major_axis;
	_e = eccentricity;
	_i = inclination * DTR;
	_littleOmega = argument_of_perigee;
	_bigOmega = 0.0;
	_l = 0.0;
	return;
}

OrbitSim::~OrbitSim()
{
	return;
}

int
OrbitSim::Initialize(
	double	sc_lon,
	double	sc_lat)
{
	//---------------------------//
	// convert inputs to radians //
	//---------------------------//

	double lon = sc_lon * DTR;
	double lat = sc_lat * DTR;

	_semilatex = _a * (1.0 - _e * _e);
	_gamma = RJ2 * _rm * _rm / (_semilatex * _semilatex);
	_eta = sqrt(1.0 - _e * _e);
	_cosi = cos(_i);
	_sini = sin(_i);

	double bpo = asin(sin(lat) / _sini);

	if (_asc < 0)
		bpo = M_PI - bpo;
	if (bpo < 0.0)
		bpo = bpo + TWOPI;
	double beta = bpo - _littleOmega;
	if (beta < 0.0)
		beta += TWOPI;
	double delo = asin(tan(lat) / tan(_i));
	_bigOmega = lon - delo;
	if (_asc < 0)
		_bigOmega = lon + delo + M_PI;
	if (_bigOmega < 0.0)
		_bigOmega += TWOPI;
	if (_bigOmega > TWOPI)
		_bigOmega -= TWOPI;

	// calculate inital eccentric and mean anomalies
	double cosec = (cos(beta) + _e) / (1.0 + _e * cos(beta));
	double sinec = (sqrt(1.0 - _e) * sin(beta)) / (1.0 + _e * cos(beta));
	double eangle = atan2(sinec, cosec);
	_l = eangle - _e * sin(eangle);


	// calculate propagation of mean anomaly, periasp, asc node
	double a = sqrt(_xmu) / (_a * sqrt(_a));
	double b = 1.0 - 1.5 * _sini * _sini;
	b = 1.0 + 1.5 * _gamma * _eta * b;

	_ameandot = a * b;
	_periasdot = 1.5 * _gamma * _ameandot * (2.0 - 2.5 * _sini * _sini);
	_ascnodot = -1.5 * _ameandot * _cosi;

	return(1);
}

//---------//
// _Update //
//---------//

int
OrbitSim::_Update(
	double		seconds)
{
	// propagate longitude of ascending node
	double omg1r = _bigOmega + _ascnodot * seconds;

	// propagate the argument of perigee
	double arperi = _littleOmega + _periasdot * seconds;

	// propagate the mean, true and eccentric anomalies
	double amean = _l + _ameandot * seconds;
	double eangle = _Ecan(_e, amean);
	double costrue = (cos(eangle) - _e ) / (1.0 - _e * cos(eangle));
	double sintrue = sqrt(1.0 - _e * _e) * sin(eangle) /
		(1.0 - _e * cos(eangle));
	double tangle = atan2(sintrue, costrue);
	double demean = fmod(amean, TWOPI);

	// recalculate beta_prime that will be used in convert
	// beta_prime = true anomaly + argument of perigee
	_bp = tangle + arperi;

	// r, g, rdot, u

	return(1);
}
