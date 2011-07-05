//==============================================================//
// Copyright (C) 1997-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_ephemeris_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "Ephemeris.h"
#include "List.h"
#include "List.C"
#include "Interpolate.h"
#include "Constants.h"
#include "SeaPac.h"

//============//
// OrbitState //
//============//

//
// Default constructor
//

OrbitState::OrbitState()
{
    return;
}

//
// Destructor
//

OrbitState::~OrbitState()
{
    return;
}

//
// OrbitState::Read()
//
// Read one set of time,position,velocity components from the specified file.
// The file is assumed to be binary with time stored in double precision,
// followed by position and velocity components also stored in double precision.
// Position and velocity components are assumed to be stored in meters
// and are immediately converted to km.
//

int
OrbitState::Read(
    FILE*  inputfile)
{
    if (fread(&time, sizeof(double), 1, inputfile) != 1)
        return(0);
    if (! rsat.Read(inputfile))
        return(0);
    if (! vsat.Read(inputfile))
        return(0);
    rsat /= 1000.0;  // convert to km.
    vsat /= 1000.0;  // convert to km/s.
    return(1);
}

//
// OrbitState::Write()
//
// Write one set of time,position,velocity components to the specified file.
// The file is assumed to be binary with time stored in double precision,
// and position and velocity components also stored in double precision.
// Position and velocities are assumed to be stored in memory with km units
// and are converted to meters for file storage.
//

int
OrbitState::Write(
    FILE*  outputfile)
{
    if (fwrite(&time, sizeof(double), 1, outputfile) != 1)
        return(0);
    rsat *= 1000.0;  // convert to m.
    vsat *= 1000.0;  // convert to m/s.
    if (! rsat.Write(outputfile))
    {
        rsat /= 1000.0;  // convert back to km
        vsat /= 1000.0;  // convert back to km/s
        return(0);
    }
    if (! vsat.Write(outputfile))
    {
        rsat /= 1000.0;  // convert back to km
        vsat /= 1000.0;  // convert back to km/s
        return(0);
    }

    rsat /= 1000.0;  // convert back to km
    vsat /= 1000.0;  // convert back to km/s
    return(1);
}

//
// OrbitState::ReadFloat()
//
// Read one set of time,position,velocity components from the specified file.
// The file is assumed to be binary with time stored in double precision,
// and position and velocity components stored in single precision (float).
//

int
OrbitState::ReadFloat(
    FILE*  inputfile)
{
    float posvel[6];

    if (fread(&time, sizeof(double), 1, inputfile) != 1)
        return(0);
    if (fread(&posvel, sizeof(float), 6, inputfile) != 6)
        return(0);
    rsat.Set(posvel[0], posvel[1], posvel[2]);
    vsat.Set(posvel[3], posvel[4], posvel[5]);
    return(1);
}

//
// OrbitState::WriteFloat()
//
// Write one set of time,position,velocity components to the specified file.
// The file is assumed to be binary with time stored in double precision,
// and position and velocity components stored in single precision (float).
//

int
OrbitState::WriteFloat(FILE *outputfile)

{

double value;
float posvel[6];

rsat.Get(0,&value);
posvel[0] = value;
rsat.Get(1,&value);
posvel[1] = value;
rsat.Get(2,&value);
posvel[2] = value;
vsat.Get(0,&value);
posvel[3] = value;
vsat.Get(1,&value);
posvel[4] = value;
vsat.Get(2,&value);
posvel[5] = value;
if (fwrite(&time,sizeof(double),1,outputfile) != 1) return(0);
if (fwrite(&posvel,sizeof(float),6,outputfile) != 6) return(0);
return(1);
}

//
// OrbitState::WriteAscii()
//
// Write one set of time,position,velocity components to the specified file.
// The file is ASCII.
//

int
OrbitState::WriteAscii(FILE *fp)
{

double value;
float posvel[6];

rsat.Get(0,&value);
posvel[0] = value;
rsat.Get(1,&value);
posvel[1] = value;
rsat.Get(2,&value);
posvel[2] = value;
vsat.Get(0,&value);
posvel[3] = value;
vsat.Get(1,&value);
posvel[4] = value;
vsat.Get(2,&value);
posvel[5] = value;
fprintf(fp,"#### Spacecraft position and Velocity Info ####\n");
fprintf(fp,"Pos_x: %g Pos_y: %g Pos_z %g\n",posvel[0],posvel[1],posvel[2]);
fprintf(fp,"Vel_x: %g Vel_y: %g Vel_z %g\n",posvel[3],posvel[4],posvel[5]);
return(1);

}

//===========//
// Ephemeris //
//===========//

Ephemeris::Ephemeris()
: _interp_midpoint_time(0),
  _interp_order(-1), _interp_time(NULL), _interp_x(NULL),
  _interp_y(NULL), _interp_z(NULL), _interp_vx(NULL), _interp_vy(NULL),
  _interp_vz(NULL)
{
    return;
}

Ephemeris::Ephemeris(
    const char*        filename,
    unsigned int    max_states)
{
    SetInputFile(filename);
    SetMaxNodes(max_states);
    _interp_midpoint_time = 0;
    _interp_order = -1;
    _interp_time = NULL;
    _interp_x = NULL;
    _interp_y = NULL;
    _interp_z = NULL;
    _interp_vx = NULL;
    _interp_vy = NULL;
    _interp_vz = NULL;
    return;
}

//
// Destructor
//

Ephemeris::~Ephemeris()
{
    CloseInputFile();
    if (_interp_time != NULL) free(_interp_time);
    if (_interp_x != NULL) free(_interp_x);
    if (_interp_y != NULL) free(_interp_y);
    if (_interp_z != NULL) free(_interp_z);
    if (_interp_vx != NULL) free(_interp_vx);
    if (_interp_vy != NULL) free(_interp_vy);
    if (_interp_vz != NULL) free(_interp_vz);
    return;
}

//--------------------------//
// Ephemeris::FindSouthPole //
//--------------------------//

//
// Search the list of OrbitState's for the closest most southern latitude
// and return a pointer to the corresponding OrbitState.
// The current_element in the Ephemeris marks the starting point of the search.
//

OrbitState*
Ephemeris::FindSouthPole()
{

    OrbitState* os1 = GetCurrent();
    OrbitState* os2 = GetNext();

    if ((os1 == NULL) || (os2 == NULL))
    {
        printf("Error: FindSouthPole ran out of ephemeris data\n");
        return(NULL);
    }

    if (os1->rsat.Get(2) > os2->rsat.Get(2))
    {    // z-values are decreasing, so we're going south as desired.
        while (os1->rsat.Get(2) > os2->rsat.Get(2))
        {    // keep going south until the track turns north.
            os1 = os2;
            os2 = GetNext();
            if (os2 == NULL)
            {
                printf("Error: FindSouthPole ran out of ephemeris data\n");
                return(NULL);
            }
        }
        return(os1);
    }
    else
    {    // z-value are increasing, so we're going north - need to turn around.
        os1 = GetCurrent();
        os2 = GetPrev();
        while (os1->rsat.Get(2) > os2->rsat.Get(2))
        {    // keep going south until the track turns north.
            os1 = os2;
            os2 = GetPrev();
            if (os2 == NULL)
            {
                printf("Error: FindSouthPole ran out of ephemeris data\n");
                return(NULL);
            }
        }
        return(os1);
    }

}

//------------------------//
// Ephemeris::GetPosition //
//------------------------//
// Interpolate this OrbitState List (ie., Ephemeris) to the desired time
// and return the position.

int
Ephemeris::GetPosition(
    double            time,
    int                order,
    EarthPosition*    rsat)
{

    OrbitState os;
    if (GetOrbitState(time,order,&os) == 0) return(0);
//    if (GetOrbitState_2pt(time,&os) == 0) return(0);
    *rsat = os.rsat;
    return(1);

}

//--------------------------//
// Ephemeris::GetOrbitState //
//--------------------------//
// Interpolate this OrbitState List (ie., Ephemeris) to the desired time
// and return the interpolated OrbitState
// This method uses N'th order polynomial interpolation.

int
Ephemeris::GetOrbitState(
    double            time,
    int                order,
    OrbitState*        orbit_state)
{
    if (order < 0)
        return(0);

    if (order != _interp_order)
    {
        // need to reform the interpolating arrays.
        free(_interp_time);
        free(_interp_x);
        free(_interp_y);
        free(_interp_z);
        free(_interp_vx);
        free(_interp_vy);
        free(_interp_vz);
        int size = sizeof(double) * (order + 1);
        _interp_time = (double*)malloc(size);
        _interp_x = (double*)malloc(size);
        _interp_y = (double*)malloc(size);
        _interp_z = (double*)malloc(size);
        _interp_vx = (double*)malloc(size);
        _interp_vy = (double*)malloc(size);
        _interp_vz = (double*)malloc(size);
        if ((_interp_x == NULL) || (_interp_y == NULL) ||
            (_interp_z == NULL) || (_interp_vx == NULL) ||
            (_interp_vy == NULL) || (_interp_vz == NULL) ||
            (_interp_time == NULL))
        {
            return(0);
        }
        _interp_order = order;
    }

    OrbitState* os1;
    OrbitState* os2;
    if (_GetBracketingOrbitStates(time, &os1, &os2) == 0)
    {
        printf("Error: Can't find requested time %18.12g in Ephemeris\n",time);
        return(0);
    }

    if ((os1->time != _interp_midpoint_time) || (order != _interp_order))
    {
        // Interpolating points need to be set up.
        _interp_midpoint_time = os1->time;

        for (int i=0; i < (order+1)/2; i++)
        {
            // Position list at start of the set of interpolating points.
            if (GotoPrev() == 0) break;    // Can't back up anymore.
        }
        OrbitState* os = GetCurrent();
        if (os == NULL) os = GetHead();    // Backed off beginning of list.
        for (int i=0; i < order+1; i++)
        {
            // Load the interpolating set.
            if (os == NULL)
                return(0);        // Ephemeris not long enough

            _interp_time[i] = os->time;
            _interp_x[i] = os->rsat.Get(0);
            _interp_y[i] = os->rsat.Get(1);
            _interp_z[i] = os->rsat.Get(2);
            _interp_vx[i] = os->vsat.Get(0);
            _interp_vy[i] = os->vsat.Get(1);
            _interp_vz[i] = os->vsat.Get(2);
            os = GetOrReadNext();
        }
    }

    // Polynomial interpolation of the vector components.
    double x,y,z,vx,vy,vz;
    polint(_interp_time,_interp_x,order+1,time,&x);
    polint(_interp_time,_interp_y,order+1,time,&y);
    polint(_interp_time,_interp_z,order+1,time,&z);
    polint(_interp_time,_interp_vx,order+1,time,&vx);
    polint(_interp_time,_interp_vy,order+1,time,&vy);
    polint(_interp_time,_interp_vz,order+1,time,&vz);

    orbit_state->rsat.Set(x,y,z);
    orbit_state->vsat.Set(vx,vy,vz);
    orbit_state->time = time;

    return(1);
}

//------------------------------//
// Ephemeris::GetOrbitState_2pt //
//------------------------------//
// Interpolate this OrbitState List (ie., Ephemeris) to the desired time
// and return the interpolated OrbitState
// This method uses a simple 2 point linear or circular (depending on which
// is commented out below) interpolation between the bracketing points.

int
Ephemeris::GetOrbitState_2pt(
    double            time,
    OrbitState*        orbit_state)
{
    OrbitState* os1;
    OrbitState* os2;
    if (_GetBracketingOrbitStates(time, &os1, &os2) == 0)
    {
    //    printf("Error: Can't find requested time %g in Ephemeris\n",time);
        return(0);
    }

    double time1 = os1->time;
    EarthPosition rsat1 = os1->rsat;
    Vector3 vsat1 = os1->vsat;

    double time2 = os2->time;
    EarthPosition rsat2 = os2->rsat;
    Vector3 vsat2 = os2->vsat;

    // Linearly interpolate the position components in time.
    //orbit_state->rsat = (rsat2-rsat1)*((time-time1)/(time2-time1)) + rsat1;

    // Circular interpolation of the position vector.
    double range = rsat1.Magnitude();
    double theta = acos((rsat1 % rsat2)/range/range);
    double theta1 = (time-time1)/(time2-time1) * theta;
    Matrix3 a;
    a.Rowset(rsat1, rsat2, rsat1 & rsat2);
    Vector3 b(range*range*cos(theta1), range*range*cos(theta-theta1), 0);
    if (! a.Inverse())
        return(0);

    orbit_state->rsat = a * b;

    // Linearly interpolate the components of the velocity vector.
    orbit_state->vsat = (vsat2-vsat1)*((time-time1)/(time2-time1)) + vsat1;

    orbit_state->time = time;

    return(1);
}

//------------------------------//
// Ephemeris::GetNextOrbitState //
//------------------------------//

int
Ephemeris::GetNextOrbitState(
    OrbitState*        os)
{
    OrbitState* ptr = GetOrReadNext();
    if (ptr == NULL)
        return(0);

    *os = *ptr;
    return(1);
}

#define EPSILON              1E-30

int
Ephemeris::GetSOMCoordinates(
    EarthPosition    rground,
    double           measurement_time,
    int              grid_starts_north_pole,
    double*          ct_lat,
    double*          at_lon )
{	
	double sc_pos[3];
	double sc_vel[3];
	double nodal_period;
	double arg_lat;
	double long_asc_node;
	double orb_inclination;
	double orb_smaj_axis;
	double orb_eccen;
	double arg_per;
	double mean_anom;
    double obs_alt, obs_lon, obs_lat;
	OrbitState os;
	
	// Interpolate ephem to measurement_time.
	GetOrbitState(measurement_time, EPHEMERIS_INTERP_ORDER, &os);
      
    // Copy state vector from OrbitState object & convert to meters, meters/second 
	sc_pos[0] = os.rsat.GetX()*1000;
	sc_pos[1] = os.rsat.GetY()*1000;
	sc_pos[2] = os.rsat.GetZ()*1000;
	sc_vel[0] = os.vsat.GetX()*1000;
	sc_vel[1] = os.vsat.GetY()*1000;
	sc_vel[2] = os.vsat.GetZ()*1000;
	
	// Compute osculating orbit elements.
	if( !compute_orbit_elements( sc_pos[0], sc_pos[1], sc_pos[2],
	     sc_vel[0], sc_vel[1], sc_vel[2], &nodal_period, &arg_lat, 
	     &long_asc_node, &orb_inclination,&orb_smaj_axis,  &orb_eccen,
	     &arg_per, &mean_anom ) )
	{
	  fprintf( stderr, "Ephemeris::GetSOMCoordinates: ERROR in compute_orbit_elements!\n");
	  return(0);
	}
	
	// Get alt, lon, geodetic lat for this Meas
	rground.GetAltLonGDLat(&obs_alt,&obs_lon,&obs_lat);
	
	double meas_lon = obs_lon * rtd;
	double meas_lat = obs_lat * rtd;
	
	//--All of the following code is from IJBIN; I only modified it to return
	// the SOM coordinates instead of array indices.
     double earth_a = r1_earth * 1000.0; // convert from km to m.

    //--------------------------------------------------------//
    // Set some common conversions of orbit element variables //
    //--------------------------------------------------------//
    
    double aa = orb_smaj_axis;
    double ecc = orb_eccen;
    double inc = orb_inclination * dtr;
    double cosi = cos(inc);
    double sini = sin(inc);
    double lzero = long_asc_node * dtr;
    double arglat = arg_lat * dtr;
    double arglmod = arglat + pi_over_two;
    if (arglmod > two_pi)
        arglmod -= two_pi;
 
    //--------------------------------------------//
    // Compute orbit and nodal precession periods //
    //--------------------------------------------//

    double arat = aa / earth_a;   // axis to radius ratio
    double slr = arat * (1.0 - ecc*ecc);    // normalized orbit ellipse
 
    if (fabs(nodal_period) < EPSILON)
    {
        fprintf(stderr, "Nodal period too small\n");
        return(0);
    }
    if (fabs(slr) < EPSILON)
    {
        fprintf(stderr, "SLR too small (divide by zero)\n");
        return(0);
    } 
    double pnode = -1.5 * two_pi * rj2 * cosi / (nodal_period * (slr * slr));
    
    if (fabs(wa - pnode) < EPSILON)
    {
        fprintf(stderr, "Nodal quantity too small\n");
        return(0);
    }

    double p1 = two_pi / (wa - pnode);    // earth period
    double prat = nodal_period / p1;    // period ratio
 
    //-----------------------------------------------//
    // Compute bin coordinates for each cell in beam //
    //-----------------------------------------------//

    double mlon = meas_lon * dtr;    // cell longitude in radians
    double mlat = meas_lat * dtr;    // cell latitude in radians
    double snlat = sin(mlat);
    double cslat = cos(mlat);

    if (fabs(cslat) < EPSILON)
    {
        fprintf(stderr, "cslat too small\n");
        return(0);
    }

    double tnlat = snlat / cslat;
 
    //------------------------------------------------------------------//
    // Get a trial value of along-track longitude to seed the iteration //
    //------------------------------------------------------------------//

    int ascend = 1;
    double lnode = lzero;    // ascending node longitude
    double lit0 = mlon - lnode;  // nadir node longitude difference
    double cslit0 = cos(lit0);
    double snlit0 = sin(lit0);

    if (fabs(cslit0) < EPSILON)
    {
        fprintf(stderr, "cslit0 too small\n");
        return(0);
    }

    if (cslit0 < 0.0)
        ascend = 0;
 
    // along-track longitude
    double lip = atan((cosi*snlit0 + sini*tnlat) / cslit0);
 
    if (! ascend)
        lip += pi;

    if (lip < 0.0)
        lip += two_pi;

    lip += pi_over_two;

    if (lip > two_pi)
        lip -= two_pi;

    //-----------------------------------------------//
    // Begin iteration to get along-track longitude  //
    // Compute new value for along-track `longitude' //
    //-----------------------------------------------//

    double diff = 1.0;    // set initial value greater than tolerance
    double slt = 0.0;

    while (diff > 1.0e-5)
    {
        diff = lip - arglmod;    // angular difference
        double d = pi - fabs(pi - fabs(diff));
 
        double ddif = 0.0;
        if (fabs(diff) > pi)
        {
            if (diff < -pi)
                ddif = d;    // angular difference minimum
            if (diff > pi)
                ddif = -d;
        }
        else
        {
            ddif = diff;
        }

        lnode = lzero - prat*ddif;    // ascending node longitude
        double lit = mlon - lnode;    // cell node longitude difference
        slt = sin(lit);
        double clt = cos(lit);
        ascend = 1;
 
        if (fabs(clt) < EPSILON)
        {
            fprintf(stderr, "abs_clt too small\n");
            return(0);
        }

        if (clt < 0.0)
            ascend = 0;

        double lip1 = atan((cosi*slt + (1.0 - e2) * sini*tnlat) / clt);
  
        if (! ascend)
            lip1 += pi;

        if (lip1 < 0.0)
            lip1 += two_pi;

        lip1 += pi_over_two;
  
        if (lip1 > two_pi)
            lip1 -= two_pi;
 
        //--------------------------------//
        // Check convergence of iteration //
        //--------------------------------//

        diff = fabs(lip - lip1);    // angular difference
        lip = lip1;    // along-track longitude
    }

    //----------------------------------------------------//
    // Iteration to get along-track longitude is complete //
    // Now compute cross-track `latitude'                 //
    //----------------------------------------------------//
 
    double sphipi = (1.0 - e2)*cosi*snlat - sini*cslat*slt;
    sphipi /= sqrt(1.0 - e2*snlat*snlat);
    double phipi = asin(sphipi);    // cross-track latitude	
	
	// If grid starts at north pole, remove pi from at_lon and wrap in range.
  	if( grid_starts_north_pole ) {
  	  lip -= pi;
	  if( lip < 0 ) lip += two_pi;
	}
	
	*at_lon = lip*rtd;   // along-track longitude
    *ct_lat = phipi*rtd; // cross-track latitude

 	// Un-wrap along-track longitude depending on ascending/decending.	
	if( grid_starts_north_pole ) {
	  // For grids starting at north pole:
	  // If decending (vz<0) and at_lon says at end of orbit, it is probably 
	  // wrapped from observations near the start of orbit.	  
	  if( *at_lon > 340 && sc_vel[2] < 0 ) 
	    *at_lon -= 360;
	    
	  // Similar logic on other boundary.
	  if( *at_lon < 20 && sc_vel[2] > 0 )
	    *at_lon += 360;
	} else {
	  // For grids starting at south pole:
	  // If ascending (vz>0) and at_lon says at end of orbit, it is probably 
	  // wrapped from observations near the start of orbit.	  
	  if( *at_lon > 340 && sc_vel[2] > 0 )
	    *at_lon -= 360;
	  
	  // Similar logic on other boundary.
	  if( *at_lon < 20 && sc_vel[2] < 0 )
	    *at_lon += 360;
	}
	return(1);
}

//---------------------------------------------------------------------------//
// GetSubtrackCoordinates
//
// Convert a position in geocentric coordinates into subtrack coordinates
// using this ephemeris object to define the subtrack.
// This method locates the ephemeris point closest to the position point
// and then uses the surface distance method to get the cross track distance,
// and the reference point to get the along track distance.
// The measurement time is used to determine which orbit the subtrack
// coordinates are needed for.  Otherwise, the position will have subtrack
// coordinates in every rev.
//
// INPUTS:
//    rground = the position on the earth's surface to be gridded.
//    subtrack_start = position on surface of grid start (0,0 point).
//    start_time = the time when the s/c is directly over subtrack_start.
//    measurement_time = the time at which rground was observed.
//    crosstrack,alongtrack = pointers to return variables. (km)
//---------------------------------------------------------------------------//

int
Ephemeris::GetSubtrackCoordinates(
    EarthPosition    rground,
    EarthPosition    subtrack_start,
    double            start_time,
    double            measurement_time,
    float*            crosstrack,
    float*            alongtrack)
{
    double t1, t2, t3, t;
    double r1, r2, r3, r;

    // default returns
    *crosstrack = 0.0;
    *alongtrack = 0.0;

    // Create an object which supplies the range function to be minimized by
    // standard routines like brent() below.
    RangeFunction rangefunc(this,&rground);

    //
    // Initial guesses start at the measurement time and go out 40 sec's which
    // is about the time it takes to cover 300 km.
    //

    t1 = measurement_time;
    t2 = measurement_time + 40.0;

    //
    // Bracket the minimum range between t1 and t3.
    //

    r1 = rangefunc.Range(t1);
    r2 = rangefunc.Range(t2);
    if ((r1 < 0) || (r2 < 0))
        return(0);        // t1 or t2 out of Ephemeris range

    if (r2 > r1)
    {
        // switch t1 and t2 so that range decreases going from t1 to t2.
        double tmp = t1;
        t1 = t2;
        t2 = tmp;
        tmp = r1;
        r1 = r2;
        r2 = tmp;
    }

    //
    // Initial guess for t3 by stepping downhill a factor of 1.6.
    //

    t3 = t2 + 1.6*(t2 - t1);
    r3 = rangefunc.Range(t3);
    if (r3 < 0)
        return(0);        // t3 out of Ephemeris range

    while (r2 > r3)
    {
        // step downhill until the range increases.
        t = t3 + 1.6*(t3 - t2);
        r = rangefunc.Range(t);
        if (r < 0)
            return(0);        // t out of Ephemeris range
        t1 = t2;
        r1 = r2;
        t2 = t3;
        r2 = r3;
        t3 = t;
        r3 = r;
    }

    //
    // Locate the ephemeris position with minimum range to the EarthPosition
    // using a golden section search and the bracketing points t1,t2,t3.
    //

    double R = 0.61803399;
    double C = 1.0 - R;
    double t0 = t1;

    //
    // Put the smaller interval between t0 and t1 and setup for the search
    //

    if (fabs(t3-t2) > fabs(t2-t1))
    {
        t1 = t2;
        r1 = r2;
        t2 = t2 + C*(t3-t2);    // first golden section step
        r2 = rangefunc.Range(t2);
        if (r2 < 0) return(0);    // t2 out of Ephemeris range
    }
    else
    {
        t1 = t2 - C*(t2-t1);    // first golden section step
        r1 = rangefunc.Range(t1);
        if (r1 < 0) return(0);    // t1 out of Ephemeris range
    }

    //
    // Golden Section search loop
    //

    while (fabs(t3-t0) > RANGE_TIME_TOL)
    {
        if (r2 < r1)
        {
            t0 = t1;
            t1 = t2;
            t2 = R*t1 + C*t3;
            r1 = r2;
            r2 = rangefunc.Range(t2);
            if (r2 < 0) return(0);    // t2 out of Ephemeris range
        }
        else
        {
            t3 = t2;
            t2 = t1;
            t1 = R*t2 + C*t0;
            r2 = r1;
            r1 = rangefunc.Range(t1);
            if (r1 < 0) return(0);    // t1 out of Ephemeris range
        }
    }

    double min_time;
    if (r1 < r2) min_time = t1; else min_time = t2;

    // Compute the s/c position and velocity at the minimum range point.
    OrbitState min_state;
    if (GetOrbitState(min_time,EPHEMERIS_INTERP_ORDER,&min_state) == 0)
    {
        return(0);
    }

    // Compute the corresponding nadir point on the earth's surface.
    EarthPosition subtrack_min = min_state.rsat.Nadir();

    // Compute surface distance in the crosstrack direction (simple arclength).
    *crosstrack = subtrack_min.SurfaceDistance(rground);

    // Determine which side the surface point is on.
    Vector3 vec = subtrack_min & rground;    // cross product
    double test = vec % min_state.vsat;        // dot product
    if (test < 0.0)
    {    // vec is generally opposite to vsat, so rground is on the left.
        *crosstrack = -(*crosstrack);    // left side is defined to be negative
    }

    // The along track distance requires more care to keep it increasing
    // smoothly around one rev, and across rev boundaries.
    // Below, we integrate along the subtrack.

    double delta_time = min_time - start_time;
    double sign;

    if (delta_time < 0.0)
    {    // Need to integrate backwards.
        sign = -1.0;
        delta_time = -delta_time;
    }
    else
    {
        sign = 1.0;
    }

    int N = (int)(delta_time / SUBTRACK_INTEGRATION_STEPSIZE);
    EarthPosition rstep,rsat;
    EarthPosition rprev = subtrack_start;

    for (int i=1; i <= N; i++)
    {
        double time = start_time + sign*i*SUBTRACK_INTEGRATION_STEPSIZE;
        if (GetPosition(time,EPHEMERIS_INTERP_ORDER,&rsat) == 0)
        {
            printf("Error integrating along track\n");
            *alongtrack = 0;
            break;
        }
        rstep = rsat.Nadir();
        float arclen = rprev.SurfaceDistance(rstep);
        *alongtrack += arclen;
        rprev = rstep;
    }

    // Add in the last small piece (less than a stepsize).
    *alongtrack += rprev.SurfaceDistance(subtrack_min);

    // Apply sign to result.
    *alongtrack *= sign;

    return(1);
}

//---------------------------------------------------------------------------//
// GetSubtrackPosition
//
// Convert a position in subtrack coordinates into geocentric coordinates
// using this ephemeris object to define the subtrack.
// This method is the inverse of GetSubtrackCoordinates.
//
// INPUTS:
//  ctd,atd = cross and along track distances to the surface point.
//    subtrack_start = position on surface of grid start (0,0 point).
//    start_time = the time when the s/c is directly over subtrack_start.
//    rground = pointer to the returned position on the earth's surface. (km)
//---------------------------------------------------------------------------//

int
Ephemeris::GetSubtrackPosition(
    double            ctd,
    double            atd,
    double            start_time,
    EarthPosition*    rground)
{

    //----------------------------------------------------------------//
    // Estimate the time to reach the indicated along track position.
    //----------------------------------------------------------------//

    OrbitState start_state;
    if (! GetOrbitState(start_time,EPHEMERIS_INTERP_ORDER,&start_state))
    {
        printf("Error: Ephemeris::GetSubtrackPosition needs ephemeris data\n");
        exit(-1);
    }
    double vground = start_state.vsat.Magnitude() *
        r1_earth/start_state.rsat.Magnitude();

    //-------------------------------------------------------------------//
    // The time is scaled to remove an error which is proportional to the
    // along track distance (determined by trial and error).
    //-------------------------------------------------------------------//

    double measurement_time = start_time + atd/vground/1.00932;

    //----------------------------------------------------------------//
    // Go out perpendicular to the ground track to estimate the
    // cross track position.
    //----------------------------------------------------------------//

    EarthPosition r1,r2;
    if (GetPosition(measurement_time,EPHEMERIS_INTERP_ORDER,&r1) == 0 ||
        GetPosition(measurement_time+1.0,EPHEMERIS_INTERP_ORDER,&r2) == 0)
    {
        printf("Error in Ephemeris::GetSubtrackPosition\n");
        exit(-1);
    }
    Vector3 cross_dir = (r2 - r1) & r1;

    //-------------------------------------------------------------------//
    // The distance is scaled to remove an error which is proportional
    // to the cross track distance (determined by trial and error).
    //-------------------------------------------------------------------//

    cross_dir.Scale(ctd*1.1347);
    EarthPosition search_start = r1 + cross_dir;
    search_start = search_start.Nadir();

    *rground = search_start;
    return(1);
}

//--------------------------------------//
// Ephemeris::_GetBracketingOrbitStates //
//--------------------------------------//
//
// This method locates the two ephemeris points that bracket a
// particular time.  It also positions the current pointer for the
// Ephemeris list at the second of these points (*os2 below).

int
Ephemeris::_GetBracketingOrbitStates(
    double            time,
    OrbitState**    os1,
    OrbitState**    os2)
{
    // make sure there is data in the list (if there is any at all)
    OrbitState* current_state = GetCurrent();
    if (current_state == NULL)
        current_state = GetOrReadNext();

    if (current_state == NULL) return(0);

    // search forward
    while (current_state && current_state->time < time)
        current_state = GetOrReadNext();

    if (current_state == NULL) return(0);

    // search backward
    while (current_state && current_state->time > time)
        current_state = GetPrev();

    if (current_state == NULL) return(0);

    OrbitState* next_state = GetOrReadNext();

    // check
    if (next_state == NULL) return(0);

    // set states
    *os1 = current_state;
    *os2 = next_state;
    return(1);
}


//===============//
// RangeFunction //
//===============//

//
// Initialize with a pointer to the object defining the current ephemeris,
// and with a pointer to the surface EarthPosition object.
//

RangeFunction::RangeFunction(Ephemeris *eptr, EarthPosition *rptr)
{

ephemeris = eptr;
rground = rptr;

}

RangeFunction::RangeFunction()
{
return;
}

RangeFunction::~RangeFunction()
{
return;
}

//
// Compute range between s/c location at indicated time and the surface point.
//

double
RangeFunction::Range(double time)

{

EarthPosition rsat;
if (ephemeris->GetPosition(time,EPHEMERIS_INTERP_ORDER,&rsat) == 0)
    return(-1.0);
EarthPosition rlook = *rground - rsat;
return(rlook.Magnitude());

}
