//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef PSCAT_H
#define PSCAT_H

static const char rcs_id_pscat_h[] =
    "@(#) $Id$";

#include "Qscat.h"
#include "PMeas.h"

//======================================================================
// CLASSES
//    PscatEvent, Pscat
//======================================================================

//======================================================================
// CLASS
//      PscatEvent
//
// DESCRIPTION
//      The PscatEvent object contains a PSCAT event time and ID.
//======================================================================

extern const char* pscat_event_map[];

class PscatEvent
{
public:

    //-------//
    // enums //
    //-------//

    enum PscatEventE { NONE, VV_SCAT_EVENT, HH_SCAT_EVENT, VV_HV_SCAT_EVENT,
        HH_VH_SCAT_EVENT, LOOPBACK_EVENT, LOAD_EVENT };

    //--------------//
    // construction //
    //--------------//

    PscatEvent();
    ~PscatEvent();

    //-----------//
    // variables //
    //-----------//

    double       eventTime;
    PscatEventE  eventId;
    int          beamIdx;
};

//======================================================================
// CLASS
//    Pscat
//
// DESCRIPTION
//    Pscat is the top level object for the PSCAT polarimetric
//    scatterometer.
//======================================================================

class Pscat : public Qscat
{
public:
    //--------------//
    // construction //
    //--------------//

    Pscat();
    ~Pscat();

    int MakeSlices(MeasSpot* meas_spot);
    int PMeasToEsn(PMeas* meas, PMeas* meas1, PMeas* meas2,
                  float XK, float sigma0,
                  int sim_kpc_flag, float* Esn, float* Es, float* En,
                  float* var_Esn);

};

#endif
