//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   04 Feb 1998 14:15:16   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:29:02  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================
 
#ifndef EXQ_C_INCLUDED
#define EXQ_C_INCLUDED

static const char eqx_c_rcsid[] = "@(#) $Header$";

#include "Eqx.h"
#include "Itime.h"

//=============
// Eqx methods 
//=============

Eqx::Eqx()
:time(INVALID_TIME), path(0)
{
    return;
} //Eqx::Eqx

Eqx::Eqx(
    Itime           new_time,
    unsigned short  new_path)
:   time(new_time), path(new_path)
{
    return;
}

Eqx::Eqx(
    Itime   new_time,
    float   eqx_lon)
:   time(new_time)
{
    // convert eqx_lon to path
    float lon_dif = LON_ASC_NODE_PATH_585 - eqx_lon;
    while (lon_dif < 0)
        lon_dif += 360.0;
    while (lon_dif > 360.0)
        lon_dif -= 360.0;
    int new_path = (int)(lon_dif * ORBITS / 360.0 + 0.5);
    path = (unsigned short)new_path;
    return;
}

Eqx::~Eqx()
{
    return;
} //Eqx::~Eqx

//------
// Read 
//------

Eqx::ReturnE
Eqx::Read(
    FILE*   ifp)
{
    char time_string[80];
    int retval = fscanf(ifp, " %hu %s", &path, time_string);
    switch (retval)
    {
    case EOF:
        return (END_OF_FILE);
        break;
    case 2:
        if (time.CodeAToItime(time_string) == 0)
            return (ERROR_PARSING_TIME);
        else
            return (OK);
        break;
    default:
        return (ERROR_READING_EQX);
        break;
    }
    return (ERROR);
}

//-------
// Write 
//-------

Eqx::ReturnE
Eqx::Write(
    FILE*   ifp)
{
    char time_string[CODEA_TIME_LEN];
    time.ItimeToCodeA(time_string);
    fprintf(ifp, "%03hu %s\n", path, time_string);
    return (OK);
}

//---------------
// EstimateItime 
//---------------

Itime
Eqx::EstimateItime(
    Tpg tpg,
    double          period)
{
    // find approximate number of orbits
    // between the eqx and the date
    Itime time_dif = tpg.time - time;
    int orbit_count = (int)(time_dif.Seconds() / period + 0.5);

    // refine approximate orbit count by calculating the
    // multiple of MAX_PATH to use
    int eqx_orbit_number = PathToOrbit(path);
    int tpg_orbit_number = PathToOrbit(tpg.path);
    int c = (int)((double)(orbit_count - tpg_orbit_number - eqx_orbit_number) /
                (double)MAX_PATH + 0.5);
    orbit_count = c*MAX_PATH + tpg_orbit_number - eqx_orbit_number;

    // use the orbit count, period, and argument of latitude
    // to calculate the time
    double added_seconds = ((double)orbit_count + (double)tpg.gamma / 360.0) *
        period;
    return (time + Itime(added_seconds));
}

//-------------
// EstimateTpg 
//-------------

Tpg
Eqx::EstimateTpg(
    Itime   itime,
    double  period)
{
    // find the integer number of orbits between the eqx and the time
    Itime time_dif = itime - time;
    int orbit_count = (int)(time_dif.Seconds() / period);

    // convert the orbit to a path number
    int eqx_orbit_number = PathToOrbit(path);
    int itime_orbit_number = eqx_orbit_number + orbit_count;
    int itime_path = OrbitToPath(itime_orbit_number%MAX_PATH);

    // calculate gamma
    time_dif = itime - time - Itime(period*orbit_count);
    float itime_gamma = (time_dif.Seconds() / period) * 360.0;

    return(Tpg(itime.StartOfDay(), itime_path, itime_gamma));
}

//=================
// EqxList methods 
//=================

EqxList::EqxList()
{
    return;
}

EqxList::EqxList(
    const char*     filename)
{
    Read(filename);
    return;
}

EqxList::~EqxList()
{
    Eqx* eqx;
    GetHead();
    while ((eqx=RemoveCurrent()) != NULL)
        delete eqx;
    return;
}

//-------
// Write 
//-------

EqxList::StatusE
EqxList::Write(
    const char*     filename)
{
    //-------------------
    // open the Eqx file 
    //-------------------

    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
    {
        _status = ERROR_OPENING_FILE;
        return (_status);
    }

    //-----------------------------
    // write the equator crossings 
    //-----------------------------

    for (Eqx* eqx = GetHead(); eqx; eqx = GetNext())
        eqx->Write(ofp);

    //--------------------
    // close the Eqx file 
    //--------------------

    fclose(ofp);
    return (_status);
}


//------
// Read 
//------

EqxList::StatusE
EqxList::Read(
    const char*     filename)
{
    //-------------------
    // open the Eqx file 
    //-------------------

    FILE* ifp = fopen(filename, "r");
    if (ifp == NULL)
    {
        _status = ERROR_OPENING_FILE;
        return (_status);
    }

    //-------------------------------
    // read in the equator crossings 
    //-------------------------------

    _status = EqxList::OK;  // assume everything will be OK
    int done = 0;
    do
    {
        Eqx* newEqx = new Eqx;
        switch (newEqx->Read(ifp))
        {
        case Eqx::OK:
            Append(newEqx);
            break;
        case Eqx::END_OF_FILE:
            delete newEqx;
            done = 1;
            break;
        default:
            delete newEqx;
            _status = EqxList::ERROR_READING_EQX;
            done = 1;
            break;
        }
    } while (! done);

    //--------------------
    // close the Eqx file 
    //--------------------

    fclose(ifp);
    return (_status);
}

//--------
// AddEqx 
//--------

EqxList::StatusE
EqxList::AddEqx(
    Itime   time,
    float   eqx_lon)
{
    // create an Eqx object
    Eqx* newEqx = new Eqx(time, eqx_lon);
    if (! newEqx)
        return(_status = EqxList::ERROR_ALLOCATING_EQX);

    AddUniqueSorted(newEqx);

    return(_status);
}

//------------
// TpgToItime 
//------------

int
EqxList::TpgToItime(
    const Tpg   tpg,
    Itime*      itime)
{

    Eqx* eqx = FindNearestEqx(tpg);
    if (! eqx)
        return (0);

    *itime = eqx->EstimateItime(tpg, SEC_PER_ORBIT);
    return (1);
}

//------------
// ItimeToTpg 
//------------

int
EqxList::ItimeToTpg(
    const Itime     itime,
    Tpg*    tpg)
{
    Eqx* eqx = FindNearestEqx(itime);
    if (! eqx)
        return (0);

    *tpg = eqx->EstimateTpg(itime, SEC_PER_ORBIT);
    return (0);
}

//----------------
// FindNearestEqx 
//----------------
// returns a pointer to the nearest previous eqx (from Tpg)

Eqx*
EqxList::FindNearestEqx(
    const Tpg       tpg)
{
    for (Eqx* eqx = GetTail(); eqx; eqx = GetPrev())
    {
        Tpg eqx_tpg(eqx->time, eqx->path, 0.0);
        if (eqx_tpg < tpg)
            return (eqx);
    }
    return (0);
}

//----------------
// FindNearestEqx 
//----------------
// returns a pointer to the nearest previous eqx (from time)

Eqx*
EqxList::FindNearestEqx(
    const Itime     itime)
{
    for (Eqx* eqx = GetTail(); eqx; eqx = GetPrev())
    {
        if (eqx->time < itime)
            return (eqx);
    }
    return (0);
}

#endif
