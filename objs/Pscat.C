//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_pscat_c[] =
    "@(#) $Id$";

#include "Pscat.h"
#include "PMeas.h"
#include "Misc.h"
#include "Distributions.h"

//============//
// PscatEvent //
//============//

const char* pscat_event_map[] = { "None", "VV", "HH", "VVHV", "HHVH",
    "LOOP", "LOAD" };

PscatEvent::PscatEvent()
:   eventTime(0.0), eventId(NONE), beamIdx(0)
{
    return;
}

PscatEvent::~PscatEvent()
{
    return;
}

//=======//
// Pscat //
//=======//

Pscat::Pscat()
{
    return;
}

Pscat::~Pscat()
{
    return;
}

//------------//
// MakeSlices //
//------------//

// This method creates the measurement list (of PMeas's) for a spot
// and sets the PMeas indices (with one slice per PMeas).

int
Pscat::MakeSlices(
    MeasSpot*  meas_spot)
{
    meas_spot->FreeContents();
    int total_slices = ses.GetTotalSliceCount();

    for (int slice_idx = 0; slice_idx < total_slices; slice_idx++)
    {
        PMeas* pmeas = new PMeas();
        // We assume that the slices are sequential.
        abs_to_rel_idx(slice_idx,total_slices,&(pmeas->startSliceIdx));
        pmeas->numSlices = 1;
        meas_spot->Append(pmeas);
    }

    return(1);
}
