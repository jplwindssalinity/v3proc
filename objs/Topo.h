//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef TOPO_H
#define TOPO_H

static const char rcs_id_pod_h[] =
    "@(#) $Id$";

#include <stdio.h>

//======================================================================
// CLASSES
//    Topo, Stable
//======================================================================

//======================================================================
// CLASS
//    Topo
//
// DESCRIPTION
//    The Topo object holds a topgraphic map of the earth.
//======================================================================

class Topo
{
public:

    //--------------//
    // construction //
    //--------------//

    Topo();
    ~Topo();

    //--------------//
    // input/output //
    //--------------//

    int  Read(const char* filename);

    //--------//
    // access //
    //--------//

    int  Height(float longitude, float latitude);

protected:

    int  _Allocate();
    int  _Deallocate();

    //-----------//
    // variables //
    //-----------//

    short**  _map;
};

//======================================================================
// CLASS
//    Stable
//
// DESCRIPTION
//    The Stable object holds the frequency shift topographic
//    correction.
//======================================================================

class Stable
{
public:

    //--------------//
    // construction //
    //--------------//

    Stable();
    ~Stable();

    int  Allocate();

    //--------------//
    // input/output //
    //--------------//

    int  Read(const char* filename);

    //--------//
    // access //
    //--------//

    float  GetValue(int beam_idx, float angle, float orbit_fraction,
               int mode_id);

protected:

    int  _Allocate();
    int  _Deallocate();

    //-----------//
    // variables //
    //-----------//

    float****  _table;
};

//==================//
// helper functions //
//==================//

float  topo_delta_f(Topo* topo, Stable* stable, int beam_idx,
           float orbit_fraction, float antenna_azimuth, int mode_id,
           float longitude, float latitude);

#endif
