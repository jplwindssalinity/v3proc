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
//      Scatterometer
//======================================================================

//======================================================================
// CLASS
//      Scatterometer
//
// DESCRIPTION
//      The Scatterometer class is a base class for scatterometers.
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

#endif
