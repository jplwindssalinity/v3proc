//==============================================================//
// Copyright (C) 1997-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef WIND_VECTOR_H
#define WIND_VECTOR_H

static const char rcs_id_wind_vector_h[] =
    "@(#) $Id$";

//======================================================================
// CLASSES
//    WindVector
//======================================================================

//======================================================================
// CLASS
//    WindVector
//
// DESCRIPTION
//    The WindVector object represents a wind vector (speed and
//    direction). The wind direction is counter-clockwise from East.
//======================================================================

class WindVector
{
public:

    //--------------//
    // construction //
    //--------------//

    WindVector();
    ~WindVector();

    //---------//
    // setting //
    //---------//

    int  SetSpdDir(float speed, float direction);
    int  SetUV(float u, float v);
    int  GetUV(float* u, float* v);

    //--------------//
    // manipulation //
    //--------------//

    int  ScaleToSpeed(float scale);

    //-----------//
    // variables //
    //-----------//

    float  spd;
    float  dir;
};

#endif
