//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_seapac_c[] =
    "@(#) $Id$";

#include <math.h>
#include "SeaPac.h"
#include "Constants.h"

//-------//
// ijbin //
//-------//

#define EARTH_A              6.3781363E6    // meters
#define EPSILON              1E-30
#define L2A_ATRACK_GRIDS     1624
#define L2A_XTRACK_GRIDS     76
#define L2A_GRID_RESOLUTION  25000.0    // meters
#define JOFFSET              (int)(L2A_XTRACK_GRIDS/2)
#define ATRACK_BIN_CONST     (two_pi/L2A_ATRACK_GRIDS)
#define XTRACK_BIN_CONST     (L2A_GRID_RESOLUTION/EARTH_A)

int
ijbin(
    double  orb_smaj_axis,
    double  orb_eccen,
    double  orb_inclination,
    double  long_asc_node,
    double  arg_lat,
    double  nodal_period,
    double  meas_lon,
    double  meas_lat,
    int*    ati,
    int*    cti)
{
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

    double arat = aa / EARTH_A;   // axis to radius ratio
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
 
    //-----------------------------------------------------//
    // Compute bin coordinates from binning formulae       //
    // Compute modified WVC_I for south polar rev boundary //
    //-----------------------------------------------------//

    double dat = lip / ATRACK_BIN_CONST;    // along-track distance
    // int wvc_i = nint(dat + 0.5);
    int wvc_i = (int)floor(dat + 1.0);    // along-track coordinate

    if (wvc_i > L2A_ATRACK_GRIDS)
        wvc_i -= L2A_ATRACK_GRIDS;

    //------------------------------------------------------------//
    // Compute modified WVC_J for uniform grid                    //
    // JPRIME = cell coordinate with respect to subtrack "cell 0" //
    //------------------------------------------------------------//

    double dct = phipi / XTRACK_BIN_CONST;    // cross-track distance
    // wvc_j = nint(dct - 0.5);
    int wvc_j = (int)floor(dct);

    //-------------------------------------------------------------//
    // Final cross-track coordinate                                //
    // Reverse sign to conform to SeaWinds swath coordinate system //
    //-------------------------------------------------------------//

    wvc_j = -wvc_j + JOFFSET;    // cross-track coordinate
 
    //-------------------------------------------//
    // Make sure that the cross-track coordinate //
    // is within 1,L2A_XTRACK_GRIDS range        //
    //-------------------------------------------//

    if (wvc_j < 1)
    {
        wvc_j = 1;
    }
    else if (wvc_j > L2A_XTRACK_GRIDS)
    {
        wvc_j = L2A_XTRACK_GRIDS;
    }

    *ati = wvc_i;
    *cti = wvc_j;

    return(1);
}

//------------------------//
// compute_orbit_elements //
//------------------------//

#define EARTH_GRAVI_PARAM  3.98600E14

int
compute_orbit_elements(
    double   x_pos,
    double   y_pos,
    double   z_pos,
    double   x_vel,
    double   y_vel,
    double   z_vel,
    double*  nodal_period,
    double*  arg_lat,
    double*  long_asc_node,
    double*  orb_inclination,
    double*  orb_smaj_axis,
    double*  orb_eccen,
    double*  arg_of_per,    // optional
    double*  mean_anom)     // optional
{
    double pos[3];
    pos[0] = x_pos;
    pos[1] = y_pos;
    pos[2] = z_pos;

    double vel[3];
    vel[0] = x_vel;
    vel[1] = y_vel;
    vel[2] = z_vel;

    //-----------------------------------//
    // Calculate the vector dot products //
    //-----------------------------------//

    double rr = 0.0;
    double rv = 0.0;
    double vv = 0.0;
 
    for (int i = 0; i < 3; i++)
    {
        rr += pos[i]*pos[i];
        rv += pos[i]*vel[i];
        vv += vel[i]*vel[i];
    }

    //---------------------------------------------//
    // Calculate the magnitude of the state vector //
    //---------------------------------------------//

    double r0 = sqrt(rr);
    if (fabs(r0) < EPSILON)
    {
        fprintf(stderr, "abs_r0_too_small\n");
        return(0);
    }

    //--------------------------------------------------------------//
    // Calculate the values used in the semi-major axis computation //
    //--------------------------------------------------------------//

    double aa = (2.0 / r0) - (vv / EARTH_GRAVI_PARAM);
    if (fabs(aa) < EPSILON)
    {
        fprintf(stderr, "abs_aa_too_small\n");
        return(0);
    }
    if (aa < EPSILON)
    {
        fprintf(stderr, "aa_negative\n");
        return(0);
    }
 
    //-------------------------------------------------//
    // Compute the semi-major axis of the orbital path //
    //-------------------------------------------------//

    double tmp_orb_smaj_axis = 1.0 / aa;
    if (tmp_orb_smaj_axis < EPSILON)
    {
        fprintf(stderr, "orb_smaj_axis_too_small\n");
        return(0);
    }
 
    //----------------------------------------------//
    // Compute the eccentricity of the orbital path //
    //----------------------------------------------//

    double ce = 1.0 - r0 / tmp_orb_smaj_axis;
    double se = rv / sqrt(EARTH_GRAVI_PARAM * tmp_orb_smaj_axis);
    double tmp_orb_eccen = sqrt(se*se + ce*ce);
    double param = tmp_orb_smaj_axis * (1.0 - tmp_orb_eccen*tmp_orb_eccen);
    if (fabs(param) < EPSILON)
    {
        fprintf(stderr, "abs_param_too_small\n");
        return(0);
    }
    if (param < EPSILON)
    {
        fprintf(stderr, "param_negative\n");
        return(0);
    }
  
    //--------------------------------------------//
    // Calculate values that are used in the rest //
    // of the orbital element computations.       //
    //--------------------------------------------//

    double u[3], w[3];
    for (int i = 0; i < 3; i++)
    {
        u[i] = pos[i] / r0;
        w[i] = (rr * vel[i] - rv * pos[i]) /
            (r0 * sqrt(EARTH_GRAVI_PARAM * param));
    }

    //-------------------------------------------//
    // Compute the inclination of the orbit path //
    //-------------------------------------------//
 
    double sini = sqrt(u[2]*u[2] + w[2]*w[2]);
    double sum01 = u[0] + w[1];
    double dif10 = u[1] - w[0];
    double cosi = sqrt(sum01*sum01 + dif10*dif10) - 1.0;
    double tmp_orb_inclination = atan2(sini, cosi);

    //------------------------------------------------------//
    // Compute the longitude of the ascending node crossing //
    //------------------------------------------------------//
 
    double tmp_long_asc_node = atan2((u[1]*w[2] - w[1]*u[2]),
        (u[0]*w[2] - w[0]*u[2]));
    if (tmp_long_asc_node < 0.0)
        tmp_long_asc_node += two_pi;

    //--------------------------------------//
    // Compute the argument of the latitude //
    //--------------------------------------//
 
    double tmp_arg_lat = atan2(u[2], w[2]);
    if (tmp_arg_lat < 0.0)
        tmp_arg_lat += two_pi;

    //--------------------------//
    // Compute the nodal period //
    //--------------------------//

    double ree2 = 1.0 - tmp_orb_eccen*tmp_orb_eccen;
    if (fabs(ree2) < EPSILON)
    {
        fprintf(stderr, "abs_ree2_too_small\n");
        return(0);
    }
    if (ree2 < EPSILON)
    {
        fprintf(stderr, "ree2_negative\n");
        return(0);
    }

    cosi = cos(tmp_orb_inclination);
    double cosi2 = cosi*cosi;
    double sma2 = tmp_orb_smaj_axis * tmp_orb_smaj_axis;
    double sma3 = sma2 * tmp_orb_smaj_axis;
    double tmp_nodal_period = (2.0 * pi * sqrt(sma3 / EARTH_GRAVI_PARAM)) *
        (1.0 + (0.75 * rj2 * r1_earth_2 / sma2) * ((1.0 - 3.0 * cosi2) *
        sqrt(ree2) + (1.0 - 5.0 * cosi2)) / (ree2*ree2));

    //---------------------------------//
    // compute the argument of perigee //
    //---------------------------------//

    double ecosv = param / r0 - 1.0;
    double esinv = sqrt(param / EARTH_GRAVI_PARAM) * (rv / r0);
    double true_anom = atan2(esinv, ecosv);
    double tmp_arg_of_per = tmp_arg_lat - true_anom;
    while (tmp_arg_of_per < 0.0)
        tmp_arg_of_per += two_pi;
    while (tmp_arg_of_per >= two_pi)
        tmp_arg_of_per -= two_pi;

    //--------------------------//
    // compute the mean anomaly //
    //--------------------------//

    double ratio = (1.0 - tmp_orb_eccen) / (1.0 + tmp_orb_eccen);
    double eccen_anom = 2.0 * atan(sqrt(ratio) * tan(0.5 * true_anom));
    double tmp_mean_anom = eccen_anom - tmp_orb_eccen * sin(eccen_anom);
    while (tmp_mean_anom < 0.0)
        tmp_mean_anom += two_pi;

    //-------------//
    // copy values //
    //-------------//

    *orb_smaj_axis = tmp_orb_smaj_axis;
    *orb_eccen = tmp_orb_eccen;
    *orb_inclination = tmp_orb_inclination * rtd;
    *long_asc_node = tmp_long_asc_node * rtd;
    *arg_lat = tmp_arg_lat * rtd;
    *nodal_period = tmp_nodal_period;
    if (arg_of_per != NULL)
        *arg_of_per = tmp_arg_of_per * rtd;
    if (mean_anom != NULL)
        *mean_anom = tmp_mean_anom * rtd;

    return(1);
}

//-------------------//
// read_land_sea_map //
//-------------------//

int
read_land_sea_map(
    const char*    filename,
    unsigned char  land_sea_map[LAND_SEA_LATITUDES][LAND_SEA_LONGITUDES])
{
    FILE* ifp = fopen(filename, "r");
    if (ifp == NULL)
        return(0);

    int in_char;
    for (int i = 0; i < 4; i++)
    {
        in_char = fgetc(ifp); 
        if (in_char == EOF)
        {
            fclose(ifp);
            return(0);
        }
    }

    for (int i = 0; i < LAND_SEA_LATITUDES; i++)
    {
        for (int j = 0; j < LAND_SEA_LONGITUDES; j++)
        {
            in_char = fgetc(ifp);
 
            if (in_char == EOF)
            {
                fclose(ifp);
                return(0);
            }
            land_sea_map[i][j] = (unsigned char)in_char;
        }
    }
    fclose(ifp);
    return(1);
}

//-----------//
// map_value //
//-----------//

#define LAND_SEA_MAP_DEG_RES  12.0

int
map_value(
    float          longitude,
    float          latitude,
    unsigned char  land_sea_map[LAND_SEA_LATITUDES][LAND_SEA_LONGITUDES])
{
    int lon_idx = (int)(longitude * LAND_SEA_MAP_DEG_RES);
    if (lon_idx < 0 || lon_idx >= LAND_SEA_LONGITUDES)
        return(0);
    int lat_idx = (int)((latitude + 90.0) * LAND_SEA_MAP_DEG_RES);
    if (lat_idx < 0 || lat_idx >= LAND_SEA_LATITUDES)
        return(0);

    return((int)land_sea_map[lat_idx][lon_idx]);
}
