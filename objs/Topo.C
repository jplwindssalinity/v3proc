//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_topo_c[] =
    "@(#) $Id$";

#include <stdlib.h>
#include <math.h>
#include "Topo.h"
#include "Array.h"
#include "Constants.h"
#include "BYUXTable.h"

#define FORTRAN_FILE_BYTES   4
#define TOPO_MAP_LATITUDES   721
#define TOPO_MAP_LONGITUDES  1440

#define GS_NUM_BEAMS        2
#define GS_NUM_AZIMUTHS     36
#define GS_NUM_ORBIT_STEPS  32
#define GS_NUM_MODES        8

//======//
// Topo //
//======//

Topo::Topo()
:   _map(NULL)
{
    return;
}

Topo::~Topo()
{
	_Deallocate();
    return;
}

//------------//
// Topo::Read //
//------------//

int
Topo::Read(
    const char*  filename)
{
    //---------------//
    // open the file //
    //---------------//

    FILE* ifp = fopen(filename, "r");
    if (ifp == NULL)
        return(0);

    //----------------------------------//
    // read in the FORTRAN header bytes //
    //----------------------------------//

    char dummy[4];
    if (fread(dummy, sizeof(char), FORTRAN_FILE_BYTES, ifp) !=
        FORTRAN_FILE_BYTES)
    {
        fclose(ifp);
        return(0);
    }

    //----------------------//
    // allocate for the map //
    //----------------------//

    if (! _Allocate())
    {
        fclose(ifp);
        return(0);
    }

    //--------------//
    // read the map //
    //--------------//

    for (int ilat = 0; ilat < TOPO_MAP_LATITUDES; ilat++)
    {
        if (fread(*(_map + ilat), sizeof(short), TOPO_MAP_LONGITUDES, ifp) !=
            TOPO_MAP_LONGITUDES)
        {
            fclose(ifp);
            return(0);
        }
    }

    //------------//
    // close file //
    //------------//

    fclose(ifp);
    return(1);
}

//--------------//
// Topo::Height //
//--------------//

int
Topo::Height(
    float  longitude,
    float  latitude)
{
    int lon_idx = (int)(TOPO_MAP_LONGITUDES * longitude / two_pi + 0.5);
    int lat_idx = (int)(TOPO_MAP_LATITUDES * (latitude + pi_over_two) / pi +
        0.5);
    int height = *(*(_map + lat_idx) + lon_idx);
    return(height);
}

//-----------------//
// Topo::_Allocate //
//-----------------//

int
Topo::_Allocate()
{
    if (_map != NULL)
        return(0);

    _map = (short **)make_array(sizeof(short), 2, TOPO_MAP_LATITUDES,
        TOPO_MAP_LONGITUDES);

    if (_map == NULL)
        return(0);

    return(1);
}

//-------------------//
// Topo::_Deallocate //
//-------------------//

int
Topo::_Deallocate()
{
    if (_map != NULL)
        return(1);

    free_array((void *)_map, 2, TOPO_MAP_LATITUDES, TOPO_MAP_LONGITUDES);

    _map = NULL;
    return(1);
}

//========//
// Stable //
//========//

Stable::Stable()
:   _table(NULL)
{
    return;
}

Stable::~Stable()
{
	_Deallocate();
    return;
}

//--------------//
// Stable::Read //
//--------------//

int
Stable::Read(
    const char*  filename)
{
    //---------------//
    // open the file //
    //---------------//

    FILE* ifp = fopen(filename, "r");
    if (ifp == NULL)
        return(0);

    //----------------------------------//
    // read in the FORTRAN header bytes //
    //----------------------------------//

    char dummy[FORTRAN_FILE_BYTES];
    if (fread(dummy, sizeof(char), FORTRAN_FILE_BYTES, ifp) !=
        FORTRAN_FILE_BYTES)
    {
        fclose(ifp);
        return(0);
    }

    //------------------------//
    // allocate for the table //
    //------------------------//

    if (! _Allocate())
    {
        fclose(ifp);
        return(0);
    }

    //----------------//
    // read the table //
    //----------------//

    for (int beam_idx = 0; beam_idx < GS_NUM_BEAMS; beam_idx++)
    {
        for (int azimuth_idx = 0; azimuth_idx < GS_NUM_AZIMUTHS; azimuth_idx++)
        {
            for (int orbit_step_idx = 0; orbit_step_idx < GS_NUM_ORBIT_STEPS;
                orbit_step_idx++)
            {
                float* ptr = *(*(*(_table + beam_idx) + azimuth_idx) +
                    orbit_step_idx);
                if (fread(ptr, sizeof(float), GS_NUM_MODES, ifp) !=
                    GS_NUM_MODES)
                {
                    fclose(ifp);
                    return(0);
                }
            }
        }
    }

    //------------//
    // close file //
    //------------//

    fclose(ifp);
    return(1);
}

//------------------//
// Stable::GetValue //
//------------------//

float
Stable::GetValue(
    int    beam_idx,
    float  antenna_azimuth,
    float  orbit_fraction,
    int    mode_id)
{
    //----------------//
    // check indicies //
    //----------------//

    if (beam_idx < 0 || beam_idx >= GS_NUM_BEAMS)
        return(0.0);
    if (mode_id < 0 || mode_id >= GS_NUM_MODES)
        return(0.0);

    //--------------------------------------//
    // determine interpolation coefficients //
    //--------------------------------------//

    orbit_fraction = fmod(orbit_fraction , 1.0);
    float float_orbit_step = orbit_fraction * GS_NUM_ORBIT_STEPS;
    int int_orbit_step = (int)(float_orbit_step);
    float a = float_orbit_step - (float)int_orbit_step;
    float b = 1.0 - a;
    int next_orbit_step = (int_orbit_step + 1) % GS_NUM_ORBIT_STEPS;

    antenna_azimuth = fmod(antenna_azimuth, two_pi);
    float angle = (float)GS_NUM_AZIMUTHS * antenna_azimuth / two_pi;
    int int_angle = (int)angle;
    float c = angle - int_angle;
    float d = 1.0 - c;
    int next_angle = (int_angle + 1) % GS_NUM_AZIMUTHS;

    //------------------//
    // get the s factor //
    //------------------//

    float s_factor =
        b * d *
        *(*(*(*(_table + beam_idx) + int_angle) + int_orbit_step) + mode_id) +
        b * c *
        *(*(*(*(_table + beam_idx) + next_angle) + int_orbit_step) + mode_id) +
        a * d *
        *(*(*(*(_table + beam_idx) + int_angle) + next_orbit_step) + mode_id) +
        a * c *
        *(*(*(*(_table + beam_idx) + next_angle) + next_orbit_step) + mode_id);

    return(s_factor);
}

//-------------------//
// Stable::_Allocate //
//-------------------//

int
Stable::_Allocate()
{
    if (_table != NULL)
        return(0);

    _table = (float ****)make_array(sizeof(float), 4, GS_NUM_BEAMS,
        GS_NUM_AZIMUTHS, GS_NUM_ORBIT_STEPS, GS_NUM_MODES);

    if (_table == NULL)
        return(0);

    return(1);
}

//---------------------//
// Stable::_Deallocate //
//---------------------//

int
Stable::_Deallocate()
{
    if (_table != NULL)
        return(1);

    free_array((void *)_table, 4, GS_NUM_BEAMS, GS_NUM_AZIMUTHS,
        GS_NUM_ORBIT_STEPS, GS_NUM_MODES);

    _table = NULL;
    return(1);
}

//------------------//
// helper functions //
//------------------//

//--------------//
// topo_delta_f //
//--------------//

float
topo_delta_f(
    Topo*    topo,
    Stable*  stable,
    int      beam_idx,
    float    orbit_fraction,
    float    antenna_azimuth,
    int      mode_id,
    float    longitude,
    float    latitude)
{
    float height = topo->Height(longitude, latitude);
    float s_factor = stable->GetValue(beam_idx, antenna_azimuth,
        orbit_fraction, mode_id);
    float delta_f = s_factor * height * FFT_BIN_SIZE;
    return(delta_f);
}
