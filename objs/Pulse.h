//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef PULSE_H
#define PULSE_H

static const char rcs_id_pulse_h[] =
    "@(#) $Id$";

#include <stdio.h>
#include "List.h"

//======================================================================
// CLASSES
//    Pulse, BeamTiming
//======================================================================

//======================================================================
// CLASS
//    Pulse
//
// DESCRIPTION
//    The Pulse object holds information about a pulse for timing
//    studies.
//======================================================================

class Pulse
{
public:

    //--------------//
    // construction //
    //--------------//

    Pulse();
    ~Pulse();

    double  GetStartTransmit() { return(_startTransmit); };

    //---------//
    // setting //
    //---------//

    int  Set(double tx_time, double pulse_width, double rtt_min,
             double rtt_max);

    //--------//
    // output //
    //--------//

    int  WriteTransmitPulse(FILE* fp);
    int  WriteEcho(FILE* fp);

protected:

    //-----------//
    // variables //
    //-----------//

    // all times are in seconds
    double  _pulseWidth;
    double  _beamFill;
    double  _rttMin;
    double  _rttMax;

    double  _startTransmit;
    double  _endTransmit;
    double  _startEcho;
    double  _startPeakEcho;
    double  _endPeakEcho;
    double  _endEcho;
};

//======================================================================
// CLASS
//    BeamTiming
//
// DESCRIPTION
//    The BeamTiming object holds a set of pulses and can be asked
//    questions about what's happening at any given time.
//======================================================================

class BeamTiming : public List<Pulse>
{
public:

    //--------------//
    // construction //
    //--------------//

    BeamTiming();
    ~BeamTiming();

    void FreeContents();

//    void InFlight(int in_flight) { _inFlight = in_flight; };

    int  AddPulses(int count, double pri, double pulse_width, double rtt_min,
             double rtt_max, int check = 0);

    //--------//
    // output //
    //--------//

    int WriteTransmitPulses(FILE* fp);
    int WriteEchoes(FILE* fp);

protected:

    //-----------//
    // variables //
    //-----------//

//     int    _inFlight;
};

#endif
