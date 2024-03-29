//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_spacecraftsim_c[] =
	"@(#) $Id$";

#include <math.h>
#include "Spacecraft.h"
#include "SpacecraftSim.h"
#include "Constants.h"


//-----------------------//
// predigested constants //
//-----------------------//

static const double rm_2 = rm * rm;

//===============//
// SpacecraftSim //
//===============//

SpacecraftSim::SpacecraftSim()
:	rollBias(0.0), pitchBias(0.0), yawBias(0.0), simKprsFlag(1),
    _epoch(0.0), _a(0.0), _e(0.0), _i(0.0), _bigOmega(0.0),
	_littleOmega(0.0), _l(0.0), _period(0.0), _a_3(0.0), _e_2(0.0),
	_ascnodot(0.0), _periasdot(0.0), _ameandot(0.0), _eta(0.0), _pp(0.0),
	_pp_2(0.0), _cosi(0.0), _cosi_2(0.0), _sini_2(0.0), _gama(0.0), _G(0.0),
	_H(0.0), _nextUpdateTime(0.0), _nextEqxTime(0.0)
{
	return;
}

SpacecraftSim::~SpacecraftSim()
{
	return;
}

//---------------------------//
// SpacecraftSim::Initialize //
//---------------------------//

int
SpacecraftSim::Initialize(
	double		start_time)
{
	_nextUpdateTime = start_time;
	_nextEqxTime = FindNextArgOfLatTime(start_time, EQX_ARG_OF_LAT,
		EQX_TIME_TOLERANCE);
	return(1);
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

	//----------------------//
	// calculate the period //
	//----------------------//

	_period = two_pi / (_ameandot + _periasdot);

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
	double			timex,
	Spacecraft*		spacecraft)
{
	// convert time to time since epoch
	double time_since_epoch = timex - _epoch;

	// propagate longitude of ascending node
	double omg1r = _bigOmega + _ascnodot * time_since_epoch;

	// propagate the argument of perigee
	double arperi = _littleOmega + _periasdot * time_since_epoch;

	// propagate the mean anomaly
	double amean = _l + _ameandot * time_since_epoch;

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
	double earthmove = wa * time_since_epoch;


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
	double sinlx = gc_y / xy_dist;
	double coslx = gc_x / xy_dist;
	double satlon = atan2(sinlx, coslx);
	satlon = fmod(satlon + two_pi, two_pi);

	spacecraft->orbitState.time = timex;
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
	double			timex,
	Spacecraft*		spacecraft)
{
  spacecraft->attitude.SetRoll(attCntlDist.roll.GetNumber(timex) + rollBias);
  spacecraft->attitude.SetPitch(attCntlDist.pitch.GetNumber(timex) + pitchBias);
  spacecraft->attitude.SetYaw(attCntlDist.yaw.GetNumber(timex) + yawBias);
  return(1);
}

//-------------------------------//
// SpacecraftSim::ReportAttitude //
//-------------------------------//
// This method reports attitude + knowledge error
// Knowledge error is added *only* if simKprsFlag is non-zero

void
SpacecraftSim::ReportAttitude(
	double			timex,
	Spacecraft*		spacecraft,
	Attitude*		attitude)
{
	float roll = spacecraft->attitude.GetRoll();
	float pitch = spacecraft->attitude.GetPitch();
	float yaw = spacecraft->attitude.GetYaw();

	if (simKprsFlag)
	{
		roll += attKnowDist.roll.GetNumber(timex);
		pitch += attKnowDist.pitch.GetNumber(timex);
		yaw += attKnowDist.yaw.GetNumber(timex);
	}

	unsigned char* order = spacecraft->attitude.GetOrder();
	attitude->Set(roll, pitch, yaw, order[0], order[1], order[2]);

	return;
}

//----------------------------//
// SpacecraftSim::GetArgOfLat //
//----------------------------//

double
SpacecraftSim::GetArgOfLat(
	Spacecraft*		spacecraft)
{
	double alt, lon, gc_lat;
	spacecraft->orbitState.rsat.GetAltLonGCLat(&alt, &lon, &gc_lat);

	double vz = spacecraft->orbitState.vsat.Get(2);
	int asc = (vz > 0.0 ? 1 : 0);

	double gammax;
	if (asc)
	{
		if (gc_lat >= 0.0)
			gammax = asin(sin(gc_lat) / sin(_i));
		else
			gammax = two_pi - fabs(asin(sin(gc_lat) / sin(_i)));
	}
	else
	{
		if (gc_lat >= 0.0)
			gammax = pi - asin(sin(gc_lat) / sin(_i));
		else
			gammax = pi + fabs(asin(sin(gc_lat) / sin(_i)));
	}

	return(gammax);
}

//-----------------------------------//
// SpacecraftSim::DetermineNextEvent //
//-----------------------------------//

int
SpacecraftSim::DetermineNextEvent(
	SpacecraftEvent*	spacecraft_event)
{
	//----------------------------------------//
	// find minimum time from possible events //
	//----------------------------------------//

	if (_nextEqxTime < _nextUpdateTime)
	{
		//------------------------//
		// equator crossing event //
		//------------------------//

		spacecraft_event->eventId = SpacecraftEvent::EQUATOR_CROSSING;
		spacecraft_event->time = _nextEqxTime;

		//--------------------------------//
		// set next equator crossing time //
		//--------------------------------//

		_nextEqxTime = FindNextArgOfLatTime(_nextEqxTime + _period,
			EQX_ARG_OF_LAT, EQX_TIME_TOLERANCE);
	}
	else
	{
		//--------------------//
		// update state event //
		//--------------------//

		spacecraft_event->eventId = SpacecraftEvent::UPDATE_STATE;
		spacecraft_event->time = _nextUpdateTime;

		//----------------------------//
		// set next update state time //
		//----------------------------//

		int sample_number = (int)(spacecraft_event->time / _ephemerisPeriod
			+ 1.5);
		_nextUpdateTime = (double)sample_number * _ephemerisPeriod;
	}

	return(1);
}

//-------------------------------------//
// SpacecraftSim::FindNextArgOfLatTime //
//-------------------------------------//

double
SpacecraftSim::FindNextArgOfLatTime(
	double	timex,
	double	target_arg_of_lat,
	double	time_tol)
{
	//-------------------------------------//
	// estimate delta argument of latitude //
	//-------------------------------------//

	static Spacecraft spacecraft;		// used just for this
	UpdateOrbit(timex, &spacecraft);
	double current_arg_of_lat = GetArgOfLat(&spacecraft);
	double dif_arg = fmod(target_arg_of_lat + two_pi - current_arg_of_lat,
		two_pi);

	//--------------//
	// esimate time //
	//--------------//

	double target_time = timex + _period * dif_arg / two_pi;

	//------------------//
	// bracket the root //
	//------------------//

	double delta_time = _period / 1000.0;
	double time_1 = target_time - delta_time;
	double time_2 = target_time + delta_time;

	double add_me = two_pi + pi - target_arg_of_lat;

	UpdateOrbit(time_1, &spacecraft);
	double lat_1 = GetArgOfLat(&spacecraft);
	lat_1 = fmod(lat_1 + add_me, two_pi) - pi;
	while (lat_1 > 0.0)
	{
		time_1 -= delta_time;
		UpdateOrbit(time_1, &spacecraft);
		lat_1 = GetArgOfLat(&spacecraft);
		lat_1 = fmod(lat_1 + add_me, two_pi) - pi;
	}

	UpdateOrbit(time_2, &spacecraft);
	double lat_2 = GetArgOfLat(&spacecraft);
	lat_2 = fmod(lat_2 + add_me, two_pi) - pi;
	while (lat_2 < 0.0)
	{
		time_2 += delta_time;
		UpdateOrbit(time_2, &spacecraft);
		lat_2 = GetArgOfLat(&spacecraft);
		lat_2 = fmod(lat_2 + add_me, two_pi) - pi;
	}

	//------------------//
	// bisection search //
	//------------------//

	double time_mid, lat_mid;
	while (time_2 - time_1 > time_tol)
	{
		time_mid = (time_1 + time_2) / 2.0;
		UpdateOrbit(time_mid, &spacecraft);
		lat_mid = GetArgOfLat(&spacecraft);
		lat_mid = fmod(lat_mid + add_me, two_pi) - pi;

		if (lat_mid < 0.0)
			time_1 = time_mid;
		else
			time_2 = time_mid;
	}

	//--------------------//
	// simple calculation //
	//--------------------//

	target_time = (time_1 + time_2) / 2.0;

	return(target_time);
}

//-------------------------------------//
// SpacecraftSim::FindPrevArgOfLatTime //
//-------------------------------------//

double
SpacecraftSim::FindPrevArgOfLatTime(
	double	timex,
	double	target_arg_of_lat,
	double	time_tol)
{
	double prev_time = timex - _period;
	double target_time = FindNextArgOfLatTime(prev_time, target_arg_of_lat,
		time_tol);
	if (target_time > timex)
	{
		// rare, but happens
		target_time = FindNextArgOfLatTime(prev_time - _period / 2.0,
			target_arg_of_lat, time_tol);
	}
	return(target_time);
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
