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
	//----------------//
	// copy variables //
	//----------------//

	_a = semi_major_axis;
	_e = eccentricity;
	_i = inclination * DTR;
	_littleOmega = argument_of_perigee * DTR;

	//-----------//
	// predigest //
	//-----------//

	_a_3 = _a * _a * _a;
	_e_2 = _e * _e;

	//------------//
	// initialize //
	//------------//

	_bigOmega = 0.0;
	_l = 0.0;

	return;
}

OrbitSim::~OrbitSim()
{
	return;
}

int
OrbitSim::ScState(
	double	time)
{
	//-----------//
	// constants //
	//-----------//

	static const double xmu = 3.986032e5;	// earth mass (GM) km3/sec2
	static const double rj2 = 1.08260e-3;	// earth gravitational harmonic
	static const double rm = 6.3781778e3;	// equatorial radius of earth
	static const double pi = M_PI;			// pi
	static const double wa_deg = 4.1797271e-3;	// earth rotation rate
	static const double wa = wa_deg * DTR;	// rot. rate in radians

	//-----------------------//
	// predigested constants //
	//-----------------------//

	static const double rm_2 = rm * rm;
	static const double two_pi = pi * pi;

	//-------------------------------------------------------------//
	// calculate perturbations of mean anomaly, periapse, asc node //
	//-------------------------------------------------------------//

	double a = sqrt(xmu / _a_3);	// two-body mean motion
	double pp = _a * (1.0 - _e_2);	// semi-latus rectum (nearly killed 'em!)
	double pp_2 = pp * pp;
	double gamma = rj2 * rm_2 / pp_2;
	double eta = sqrt(1.0 - _e_2);
	double cosi = cos(_i);
	double cosi_2 = cosi * cosi;
	double sini = sin(_i);
	double sini_2 = sini * sini;

	// perturbation of mean anomaly
	double b1 = 1.0 - 1.5 * sini_2;
	double b = 1.0 + 1.5 * gamma * eta * b1;
	double ameandot = a * b;		// "anomalistic" mean motion

	// rate of precession of perigee
	double periasdot = 1.5 * gamma * ameandot * (2.0 - 2.5 * sini_2);

	// rate of regression of ascending node
	double ascnodot = -1.5 * gamma * ameandot * cosi;

	// propagate longitude of ascending node
	double omg1r = _bigOmega + ascnodot * time;

	// propagate the argument of perigee
	double arperi = _littleOmega + periasdot * time;

	// propagate the mean anomaly
	double amean = _l + ameandot * time;

	// compute eccentric and true anomalies
	double eangle = _Ecan(amean);
	double cos_eangle = cos(eangle);
	double costrue = (cos_eangle - _e) / (1.0 - _e * cos_eangle);
	double sintrue = (eta * sin(eangle)) / (1.0 - _e * cos_eangle);
	double tangle = atan2(sintrue, costrue);
	double sin_tangle = sin(tangle);
	double dmean = fmod(amean, two_pi);

	//-----------------------------------------//
	// Hill variable S/C ephemeris computation //
	//-----------------------------------------//

	double r = _a * (1.0 - _e * cos_eangle);
	double r_2 = r * r;
	double G = sqrt(xmu * pp);
	double H = G * cosi;
	double rdot = xmu * _e * sin_tangle / G;
	double u = arperi + tangle;
	double two_u = 2.0 * u;
	double sin_two_u = sin(two_u);
	double cos_two_u = cos(two_u);

	// perturbed variables
	a = 8.0 * (1.0 - 3.0 * cosi_2) * (1.0 / (1.0 + eta) + eta * r_2 / pp_2) *
		_e * sin_tangle;
	b = 16.0 * sini_2 * sin_two_u;
	double delrdot = -gamma * G * pp * (a+b) / (32.0 * r_2);

	a = 8.0 * (1.0 - 3.0 * cosi_2) * (1.0 + _e * cos(tangle) /
		(1.0 + eta) + 2.0 * eta * r / pp);
	b = 8.0 * sini_2 * cos_two_u;
	double delr = gamma * pp * (a+b) / 32.0;

	a = 8.0 * sini_2 * (3.0 * cos_two_u + _e * (3.0 * cos(two_u - tangle) +
		cos(two_u + tangle)));
	double delG = gamma * G * a / 32.0;

	// get the minimum separation between true and mean anomalies
	double difangle = tangle - dmean;
	if (difangle > pi)
		difangle -= two_pi;
	if (difangle < -pi)
		difangle += two_pi;

	a = 24.0 * (1.0 - 5.0 * cosi_2) * difangle +
		16.0 * (1.0 - 6.0 * cosi_2 + (1.0 - 3.0 * cosi_2) / (1.0 + eta)) *
		_e * sin_tangle;
	b = 4.0 * (1.0 - 3.0 * cosi_2) * _e_2 * sin(2.0 * tangle) /
		(1.0 + eta) - (4.0 - 28.0 * cosi_2) * sin_two_u;
	double c = -4.0 * (4.0 - 10.0 * cosi_2) * _e * sin(two_u - tangle) +
		8.0 * cosi_2 * _e * sin(two_u + tangle);

	double delu = -gamma * (a+b+c) / 32.0;

	a = 24.0 * difangle + 24.0 * _e * sin_tangle - 12.0 * sin_two_u;
	b = -12.0 * _e * sin(two_u - tangle) - 4.0 * _e * sin(two_u + tangle);

	double delh = -gamma * cosi * (a+b) / 16.0;

	// new values
	double rnew = r + delr;
	double rdotnew = rdot + delrdot;
	double unew = u + delu;
	double Gnew = G + delG;
	double cosinew = H / Gnew;
	double sininew = sqrt(1.0 - cosinew * cosinew);

	// compute S/C geocentric latitude and longitude
	double sinlat = sininew * sin(unew);
	double satlat = asin(sinlat);
	double coslat = cos(satlat);

	a = cosinew * sin(unew);
	b = cos(unew);
	double slong = atan2(a, b);
	double hnew = omg1r + delh;
	double slonginer = (slong + hnew);
	double earthmove = wa * time;
	double omg1p = hnew - earthmove;
	double slongear = slonginer - earthmove;

	double slatdot = Gnew * sininew * cos(unew) / (rnew * rnew * coslat);
	double slongdot = H / (rnew * rnew * coslat * coslat);

	// compute ascending lon longitude at start of current rev
	double ff = -_littleOmega;
	double E_node = atan2(eta * sin(ff), cos(ff) + _e);
	double amean_node = E_node - _e * sin(E_node);
	double dt = (amean_node - _l) / ameandot;

	// get GMT of lat node crossing, the compute long_asc_node
	double pnode = two_pi / (ameandot + periasdot);
	int node_num = (int)((time - dt) / pnode) * pnode;

	// long_asc_node
	double lnode = _bigOmega - (ascnodot + wa) * (node_num + dt);
	lnode = fmod(lnode + two_pi, two_pi);

	// calculate s/c location and velocity vectors
	double c1 = cos(satlat);
	double s1 = sin(satlat);
	double c2 = cos(slonginer);
	double s2 = sin(slonginer);

	// x, y, z to center of earth
	_gc_xyz[0] = rnew * c1 * c2;
	_gc_xyz[1] = rnew * c1 * s2;
	_gc_xyz[2] = rnew * s1;

	// S/C velocity vector
	_vel[0] = rdotnew*c1*c2 - rnew*s1*c2*slatdot - rnew*c1*s2*slongdot;
	_vel[1] = rdotnew*c1*s2 - rnew*s1*s2*slatdot + rnew*c1*c2*slongdot;
	_vel[2] = rdotnew*s1 + rnew*c1*slatdot;

	// rotate the x-y axis by the earth's motion to get final
	// inertial state vector {x, y, z, xdot, ydot, zdot}

	a = cos(earthmove);
	b = sin(earthmove);

	double rss[2];
	rss[0] = _gc_xyz[0] * a + _gc_xyz[1] * b;
	rss[1] = -_gc_xyz[0] * b + _gc_xyz[1] * a;

	double vss[2];
	vss[0] = _vel[0] * a + _vel[1] * b;
	vss[1] = -_vel[0] * b + _vel[1] * a;

	_gc_xyz[0] = rss[0];
	_gc_xyz[1] = rss[1];
	_vel[0] = vss[0];
	_vel[1] = vss[1];

	// satellite radial distance from center of earth
	double rsmag = sqrt(_gc_xyz[0]*_gc_xyz[0] + _gc_xyz[1]*_gc_xyz[1] +
		_gc_xyz[2]*_gc_xyz[2]);

	// satellite latitude
	satlat = asin(_gc_xyz[2] / rsmag);

	// satellite longitude
	double sinl = _gc_xyz[1] /
		sqrt(_gc_xyz[0]*_gc_xyz[0] + _gc_xyz[1]*_gc_xyz[1]);
	double cosl = _gc_xyz[0] /
		sqrt(_gc_xyz[0]*_gc_xyz[0] + _gc_xyz[1]*_gc_xyz[1]);
	double satlon = atan2(sinl, cosl);
	satlon = fmod(satlon + two_pi, two_pi);

	_gc_hll[0] = rsmag;
	_gc_hll[1] = satlon;
	_gc_hll[2] = satlat;

	return(1);
}

//-----------------//
// OrbitSim::_Ecan //
//-----------------//

double
OrbitSim::_Ecan(
	double mean_anom)
{
	double a, b, enow;
	enow = mean_anom;
	for (int i = 0; i < 3; i++)
	{
		a = enow - _e * sin(enow) - mean_anom;
		b = 1.0 - _e * cos(enow);
		enow -= (a / b);
	}
	return(enow);
}

/*
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
	double demean = fmod(amean, two_pi);

	// recalculate beta_prime that will be used in convert
	// beta_prime = true anomaly + argument of perigee
	_bp = tangle + arperi;

	// r, g, h, rdot, u
	double r = _a * (1.0 - _e * cos(eangle));
	double g = sqrt(_xmu * _semilatex);
	double h = g * cos(_i);
	double rdot = _xmu * _e * sin(tangle) / g;
	double u = arperi + tangle;

	// perturbed variables
	a = 8.0 * (1.0 - 3.0*_cosi*_cosi) * (1.0 / (1.0+_eta) +
		_eta*r*r/(_semilatex*_semilatex)) * _e * sin(tangle);
	b = 16.0 * _sini * _sini * sin(2.0*u);
	delrdot = -1.0 * _gamma * g * _semilatex * (a+b) / (32.0 * r * r);

	a = 8.0 * (1.0 - 3.0 * _cosi * _cosi) * (1.0 + _e * cos(tangle) /
		(1.0 + _eta) + 2.0 * _eta * r / _semilatex);
	b = 8.0 * _sini * _sini * cos(2.0*u);
	delr = _gamma * _semilatex * (a+b) / 32.0;

	a = 24.0 * _sini * _sini * cos(2.0*u) +
		24.0 * _sini * _sini * _e * cos(2.0 * u - tangle) +
		8.0 * _sini * _sini * _e * cos(2.0 * u + tangle);
	delg = _gamma * g * a/32.0;

	// get the minimum separation between true anomaly and mean anomaly
	difangle = tangle - dmean;
	if (difangle > M_PI)
		difangle -= TWOPI;
	if (difangle < -M_PI)
		difangle += TWOPI;

	a = 24.0 * (1.0 - 5.0 * _cosi * _cosi) * difangle +
		16.0 * (1.0 - 6.0 * _cosi * _cosi) +
		(1.0 - 3.0 *_cosi * _cosi) / (1.0 + _eta)) * _e * sin(tangle);
	b = 4.0 * (1.0 - 3.0 * _cosi * _cosi) * _e * _e * sin(2.0 * tangle) /
		(1.0 + _eta) -
		(4.0 - 28.0 * _cosi * _cosi) * sin(2.0 * u);
	c = -4.0 * (4.0 - 10.0 * _cosi * _cosi) * _e * sin(2.0 * u - tangle) +
		8.0 * _cosi * _cosi * _e * sin(2.0 * u + tangle);
	delu = -_gamma * (a+b+c) / 32.0;

	a = 24.0 * difangle + 24.0 * _e * sin(tangle) - 12.0 * sin(2.0 * u);
	b = -12.0 * _e * sin(2.0 * u - tangle) - 4.0 * _e * sin(2.0 * u + tangle);
	delh = -_gamma * _cosi * (a+b) / 16.0;

	// perturbed r, rdot, lat, lon, latdot, londot
	_rnew = r + delr;
	_rdotnew = rdot + delrdot;
	unew = u + delu;
	gnew = g + delg;
	cosinew = h / gnew;
	sininew = sqrt(1.0 - cosinew * cosinew);

	anginclne = acos(cosinew);

	sinlat = sininew * sin(unew);

	return(1);
}
*/
