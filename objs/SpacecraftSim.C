//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_spacecraftsim_c[] =
	"@(#) $Id$";

#include <math.h>
#include "Spacecraft.h"
#include "SpacecraftSim.h"
#include "Constants.h"

//-----------//
// constants //
//-----------//

static const double xmu = 3.986032e5;	// earth mass (GM) km3/sec2
static const double rj2 = 1.08260e-3;	// earth gravitational harmonic
static const double rm = 6.3781778e3;	// equatorial radius of earth
static const double wa_deg = 4.1797271e-3;	// earth rotation rate
static const double wa = wa_deg * dtr;	// rot. rate in radians

//-----------------------//
// predigested constants //
//-----------------------//

static const double rm_2 = rm * rm;

//===============//
// SpacecraftSim //
//===============//

SpacecraftSim::SpacecraftSim()
{
	return;
}

SpacecraftSim::~SpacecraftSim()
{
	return;
}

//----------------------------//
// SpacecraftSim::DefineOrbit //
//----------------------------//

int
SpacecraftSim::DefineOrbit(
	double	epoch,
	double	semi_major_axis,
	double	eccentricity,
	double	inclination,
	double	longitude_of_asc_node,
	double	argument_of_perigee,
	double	mean_anomaly_at_epoch)
{
	//---------------------------------------//
	// copy variables and convert to radians //
	//---------------------------------------//

	_epoch = epoch;
	_a = semi_major_axis;
	_e = eccentricity;
	_i = inclination * dtr;
	_bigOmega = longitude_of_asc_node * dtr;
	_littleOmega = argument_of_perigee * dtr;
	_l = mean_anomaly_at_epoch * dtr;

	//-----------//
	// predigest //
	//-----------//

	_a_3 = _a * _a * _a;
	_e_2 = _e * _e;

	//-------------------------------------------------------------//
	// calculate perturbations of mean anomaly, periapse, asc node //
	//-------------------------------------------------------------//
 
	double a = sqrt(xmu / _a_3);	// two-body mean motion
	_pp = _a * (1.0 - _e_2);	// semi-latus rectum (nearly killed 'em!)
	_pp_2 = _pp * _pp;
	_gama = rj2 * rm_2 / _pp_2;
	_eta = sqrt(1.0 - _e_2);
	_cosi = cos(_i);
	_cosi_2 = _cosi * _cosi;
	double sini = sin(_i);
	_sini_2 = sini * sini;
 
	// perturbation of mean anomaly
	double b1 = 1.0 - 1.5 * _sini_2;
	double b = 1.0 + 1.5 * _gama * _eta * b1;
	_ameandot = a * b;		// "anomalistic" mean motion
 
	// rate of precession of perigee
	_periasdot = 1.5 * _gama * _ameandot * (2.0 - 2.5 * _sini_2);
 
	// rate of regression of ascending node
	_ascnodot = -1.5 * _gama * _ameandot * _cosi;

	_G = sqrt(xmu * _pp);
	_H = _G * _cosi;

	return(1);
}

//--------------------------------//
// SpacecraftSim::LocationToOrbit //
//--------------------------------//

int
SpacecraftSim::LocationToOrbit(
	double		longitude,
	double		latitude,
	int			asc)
{
	//--------------------//
	// convert to radians //
	//--------------------//

	double lon = longitude * dtr;
	double lat = latitude * dtr;

	double bpo = asin(sin(lat) / sin(_i));
	if (asc < 0)
		bpo = pi - bpo;
	if (bpo < 0.0)
		bpo += two_pi;
	double beta = bpo - _littleOmega;
	if (beta < 0.0)
		beta += two_pi;

	double delo = asin(tan(lat) / tan(_i));
	double omg1 = lon - delo;
	if (asc < 0)
		omg1 = lon + delo + pi;
	if (omg1 < 0.0)
		omg1 += two_pi;
	if (omg1 > two_pi)
		omg1 -= two_pi;
	_bigOmega = omg1;

	double tangle = beta;
	double cosec = (cos(tangle) + _e) / (1.0 + _e * cos(tangle));
	double sinec = (sqrt(1.0 - _e_2) * sin(tangle)) / (1.0 + _e * cos(tangle));
	double eangle = atan2(sinec, cosec);
	_l = eangle - _e * sin(eangle);

	return(1);
}

//----------------------------//
// SpacecraftSim::UpdateOrbit //
//----------------------------//

int
SpacecraftSim::UpdateOrbit(
	double			time,
	Spacecraft*		spacecraft)
{
	// propagate longitude of ascending node
	double omg1r = _bigOmega + _ascnodot * time;

	// propagate the argument of perigee
	double arperi = _littleOmega + _periasdot * time;

	// propagate the mean anomaly
	double amean = _l + _ameandot * time;

	// compute eccentric and true anomalies
	double eangle = _Ecan(amean);
	double cos_eangle = cos(eangle);
	double costrue = (cos_eangle - _e) / (1.0 - _e * cos_eangle);
	double sintrue = (_eta * sin(eangle)) / (1.0 - _e * cos_eangle);
	double tangle = atan2(sintrue, costrue);
	double sin_tangle = sin(tangle);
	double dmean = fmod(amean, two_pi);

	//-----------------------------------------//
	// Hill variable S/C ephemeris computation //
	//-----------------------------------------//

	double r = _a * (1.0 - _e * cos_eangle);
	double r_2 = r * r;
	double rdot = xmu * _e * sin_tangle / _G;
	double u = arperi + tangle;
	double two_u = 2.0 * u;
	double sin_two_u = sin(two_u);
	double cos_two_u = cos(two_u);

	// perturbed variables
	double a = 8.0 * (1.0 - 3.0 * _cosi_2) * (1.0 / (1.0 + _eta) +
		_eta * r_2 / _pp_2) * _e * sin_tangle;
	double b = 16.0 * _sini_2 * sin_two_u;
	double delrdot = -_gama * _G * _pp * (a+b) / (32.0 * r_2);

	a = 8.0 * (1.0 - 3.0 * _cosi_2) * (1.0 + _e * cos(tangle) /
		(1.0 + _eta) + 2.0 * _eta * r / _pp);
	b = 8.0 * _sini_2 * cos_two_u;
	double delr = _gama * _pp * (a+b) / 32.0;

	a = 8.0 * _sini_2 * (3.0 * cos_two_u + _e * (3.0 * cos(two_u - tangle) +
		cos(two_u + tangle)));
	double delG = _gama * _G * a / 32.0;

	// get the minimum separation between true and mean anomalies
	double difangle = tangle - dmean;
	if (difangle > pi)
		difangle -= two_pi;
	if (difangle < -pi)
		difangle += two_pi;

	a = 24.0 * (1.0 - 5.0 * _cosi_2) * difangle +
		16.0 * (1.0 - 6.0 * _cosi_2 + (1.0 - 3.0 * _cosi_2) / (1.0 + _eta)) *
		_e * sin_tangle;
	b = 4.0 * (1.0 - 3.0 * _cosi_2) * _e_2 * sin(2.0 * tangle) /
		(1.0 + _eta) - (4.0 - 28.0 * _cosi_2) * sin_two_u;
	double c = -4.0 * (4.0 - 10.0 * _cosi_2) * _e * sin(two_u - tangle) +
		8.0 * _cosi_2 * _e * sin(two_u + tangle);

	double delu = -_gama * (a+b+c) / 32.0;

	a = 24.0 * difangle + 24.0 * _e * sin_tangle - 12.0 * sin_two_u;
	b = -12.0 * _e * sin(two_u - tangle) - 4.0 * _e * sin(two_u + tangle);

	double delh = -_gama * _cosi * (a+b) / 16.0;

	// new values
	double rnew = r + delr;
	double rdotnew = rdot + delrdot;
	double unew = u + delu;
	double Gnew = _G + delG;
	double cosinew = _H / Gnew;
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


	double slatdot = Gnew * sininew * cos(unew) / (rnew * rnew * coslat);
	double slongdot = _H / (rnew * rnew * coslat * coslat);

	// calculate s/c location and velocity vectors
	double c1 = cos(satlat);
	double s1 = sin(satlat);
	double c2 = cos(slonginer);
	double s2 = sin(slonginer);

	// x, y, z to center of earth
	double gc_x = rnew * c1 * c2;
	double gc_y = rnew * c1 * s2;
	double gc_z = rnew * s1;

	// S/C velocity vector
	double vx = rdotnew*c1*c2 - rnew*s1*c2*slatdot - rnew*c1*s2*slongdot;
	double vy = rdotnew*c1*s2 - rnew*s1*s2*slatdot + rnew*c1*c2*slongdot;
	double vz = rdotnew*s1 + rnew*c1*slatdot;

	// rotate the x-y axis by the earth's motion to get final
	// inertial state vector {x, y, z, xdot, ydot, zdot}

	a = cos(earthmove);
	b = sin(earthmove);

	double rss[2];
	rss[0] = gc_x * a + gc_y * b;
	rss[1] = -gc_x * b + gc_y * a;

	double vss[2];
	vss[0] = vx * a + vy * b;
	vss[1] = -vx * b + vy * a;

	gc_x = rss[0];
	gc_y = rss[1];
	vx = vss[0];
	vy = vss[1];

	// satellite radial distance from center of earth
	double rsmag = sqrt(gc_x*gc_x + gc_y*gc_y + gc_z*gc_z);

	// satellite latitude
	satlat = asin(gc_z / rsmag);

	// satellite longitude
	double xy_dist = sqrt(gc_x*gc_x + gc_y*gc_y);
	double sinl = gc_y / xy_dist;
	double cosl = gc_x / xy_dist;
	double satlon = atan2(sinl, cosl);
	satlon = fmod(satlon + two_pi, two_pi);

	spacecraft->orbitState.time = time;
	Vector3 rsat(gc_x, gc_y, gc_z);
	spacecraft->orbitState.rsat = rsat;
	Vector3 vsat(vx, vy, vz);
	spacecraft->orbitState.vsat = vsat;

	return(1);
}

//-------------------------------//
// SpacecraftSim::UpdateAttitude //
//-------------------------------//

int
SpacecraftSim::UpdateAttitude(
	double			time,
	Spacecraft*		spacecraft)
{
	// eventually, a function should go here
	if (time < 0.0)		// bogus check to keep compiler quiet
		return(1);
	spacecraft->attitude.SetRoll(0.0);
	spacecraft->attitude.SetPitch(0.0);
	spacecraft->attitude.SetYaw(0.0);
	return(1);
}

//-----------------------------------//
// SpacecraftSim::DetermineNextEvent //
//-----------------------------------//

int
SpacecraftSim::DetermineNextEvent(
	SpacecraftEvent*    spacecraft_event)
{
	//------------------------------------//
	// initialize next time of each event //
	//------------------------------------//

	static double update_state_time = _epoch;

	//----------------------------------------//
	// find minimum time from possible events //
	//----------------------------------------//
	// easy, since there is only one

	spacecraft_event->eventId = SpacecraftEvent::UPDATE_STATE;
	spacecraft_event->time = update_state_time;

	int sample_number = (int)(spacecraft_event->time / _ephemerisPeriod + 1.5);
	update_state_time = (double)sample_number * _ephemerisPeriod;

	return(1);
}

//----------------------//
// SpacecraftSim::_Ecan //
//----------------------//

double
SpacecraftSim::_Ecan(
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
