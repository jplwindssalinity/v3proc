//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef SCATTEROMETER_H
#define SCATTEROMETER_H

static const char rcs_id_scatterometer_h[] =
    "@(#) $Id$";

//======================================================================
// CLASSES
//    Scatterometer, ScatRF, ScatDig, ScatAnt
//======================================================================

//======================================================================
// CLASS
//    Scatterometer
//
// DESCRIPTION
//    The Scatterometer class is a base class for scatterometers.
//======================================================================

class Scatterometer
{
public:
    //--------------//
    // construction //
    //--------------//

    Scatterometer();
    ~Scatterometer();
};

//======================================================================
// CLASS
//    ScatRF
//
// DESCRIPTION
//    The ScatRF class is a base class for the radio frequency
//    subsystem for scatterometers.
//======================================================================

class ScatRF
{
public:
    //--------------//
    // construction //
    //--------------//

    ScatRF();
    ~ScatRF();

    //-----------//
    // variables //
    //-----------//

};

//======================================================================
// CLASS
//    ScatDig
//
// DESCRIPTION
//    The ScatDig class is a base class for the digital processor
//    subsystem for scatterometers.
//======================================================================

class ScatDig
{
public:
    //--------------//
    // construction //
    //--------------//

    ScatDig();
    ~ScatDig();

    //-----------//
    // variables //
    //-----------//

};

//======================================================================
// CLASS
//    ScatAnt
//
// DESCRIPTION
//    The ScatAnt class is a base class for the antenna subsystem for
//    scatterometers.
//======================================================================

class ScatAnt
{
public:
    //--------------//
    // construction //
    //--------------//

    ScatAnt();
    ~ScatAnt();

    //-----------//
    // variables //
    //-----------//

};

#endif
