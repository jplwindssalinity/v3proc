//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_pulse_c[] =
    "@(#) $Id$";

#include <stdlib.h>
#include <math.h>
#include "Pulse.h"

//=======//
// Pulse //
//=======//

Pulse::Pulse()
:   _pulseWidth(0.0), _beamFill(0.0), _rttMin(0.0), _rttMax(0.0),
    _startTransmit(0.0), _endTransmit(0.0), _startEcho(0.0),
    _startPeakEcho(0.0), _endPeakEcho(0.0), _endEcho(0.0)
{
    return;
}

Pulse::~Pulse()
{
    return;
}

//------------//
// Pulse::Set //
//------------//

int
Pulse::Set(
    double  tx_time,
    double  pulse_width,
    double  rtt_min,
    double  rtt_max)
{
    _pulseWidth = pulse_width;
    _beamFill = (rtt_max - rtt_min) / 2.0;
    _rttMin = rtt_min;
    _rttMax = rtt_max;

    _startTransmit = tx_time;
    _endTransmit = tx_time + pulse_width;
    _startEcho = tx_time + rtt_min;
    _startPeakEcho = tx_time + rtt_min + pulse_width;
    _endPeakEcho = tx_time + rtt_max;
    _endEcho = tx_time + rtt_max + pulse_width;

    return(1);
}

//---------------------------//
// Pulse::WriteTransmitPulse //
//---------------------------//

int
Pulse::WriteTransmitPulse(
    FILE*  fp)
{
    fprintf(fp, "%g %g\n", _startTransmit, 0.0);
    fprintf(fp, "%g %g\n", _startTransmit, 1.0);
    fprintf(fp, "%g %g\n", _endTransmit, 1.0);
    fprintf(fp, "%g %g\n", _endTransmit, 0.0);
    return(1);
}

//------------------//
// Pulse::WriteEcho //
//------------------//

int
Pulse::WriteEcho(
    FILE*  fp)
{
    fprintf(fp, "%g %g\n", _startEcho, 0.0);
    fprintf(fp, "%g %g\n", _startPeakEcho, 0.5);
    fprintf(fp, "%g %g\n", _endPeakEcho, 0.5);
    fprintf(fp, "%g %g\n", _endEcho, 0.0);
    return(1);
}

//============//
// BeamTiming //
//============//

BeamTiming::BeamTiming()
{
    return;
}

BeamTiming::~BeamTiming()
{
    FreeContents();
    return;
}

//--------------------------//
// BeamTiming::FreeContents //
//--------------------------//

void
BeamTiming::FreeContents()
{
    Pulse* pulse;
    GotoHead();
    while ((pulse = RemoveCurrent()) != NULL)
        delete pulse;
    return;
}

//----------------------//
// BeamTiming::Generate //
//----------------------//

int
BeamTiming::AddPulses(
    int     count,
    double  pri,
    double  pulse_width,
    double  rtt_min,
    double  rtt_max,
    int     check)
{
    //-----------------------------//
    // determine the starting time //
    //-----------------------------//

    double starting_time = 0.0;
    Pulse* tail = GetTail();
    if (tail != NULL)
    {
        starting_time = tail->GetStartTransmit() + pri;
    }

    //--------------------------//
    // create and append pulses //
    //--------------------------//

    for (int pulse_idx = 0; pulse_idx < count; pulse_idx++)
    {
        double tx_time = starting_time + (double)pulse_idx * pri;
        Pulse* pulse = new Pulse();
        if (pulse == NULL)
            return(0);
        pulse->Set(tx_time, pulse_width, rtt_min, rtt_max);
        if (! Append(pulse))
            return(0);
    }

    return(1);
}

//---------------------------------//
// BeamTiming::WriteTransmitPulses //
//---------------------------------//

int
BeamTiming::WriteTransmitPulses(
    FILE* fp)
{
    for (Pulse* pulse = GetHead(); pulse; pulse = GetNext())
    {
        if (! pulse->WriteTransmitPulse(fp))
            return(0);
    }
    return(1);
}

//-------------------------//
// BeamTiming::WriteEchoes //
//-------------------------//

int
BeamTiming::WriteEchoes(
    FILE* fp)
{
    for (Pulse* pulse = GetHead(); pulse; pulse = GetNext())
    {
        if (! pulse->WriteEcho(fp))
            return(0);
    }
    return(1);
}
