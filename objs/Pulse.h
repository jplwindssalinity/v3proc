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
#include "Constants.h"

//======================================================================
// CLASSES
//    Pulse
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

    //---------//
    // setting //
    //---------//

    int  Set(int beam_number, double tx_time, double pulse_width,
             double rtt_min, double rtt_max);

    static double  scale;
    static char    units[3];
    static void    Milli();
    static void    Micro();

    //--------//
    // output //
    //--------//

    int  WriteTransmitPulse(FILE* fp);
    int  WriteEcho(FILE* fp);

    //-----------//
    // variables //
    //-----------//

    // all times are in seconds
    int     beamNumber;
    double  pulseWidth;
    double  beamFill;
    double  rttMin;
    double  rttMax;

    double  startTransmit;
    double  endTransmit;
    double  startEcho;
    double  startPeakEcho;
    double  endPeakEcho;
    double  endEcho;
};

//======================================================================
// CLASS
//    PulseList
//
// DESCRIPTION
//    The PulseList object is a list of Pulses.
//======================================================================

class PulseList : public List<Pulse>
{
public:
    PulseList();
    ~PulseList();

    void  FreeContents();

    int   WriteTransmitPulses(FILE* fp);
    int   WriteEchoes(FILE* fp);
};

//======================================================================
// CLASS
//    BeamControl
//
// DESCRIPTION
//    The BeamControl class knows the (mostly) static features of a
//    beam. It is used to generate pulses.
//======================================================================

class BeamControl
{
public:
    BeamControl();
    ~BeamControl();

    //---------//
    // setting //
    //---------//

    void  SetBeamNumber(int beam_number) { _beamNumber = beam_number; };
    void  SetPri(double pri) { _pri = pri; };
    void  SetPriStep(double pri_step) { _priStep = pri_step; };
    void  SetPulseWidth(double pulse_width) { _pulseWidth = pulse_width; };
    void  SetPulseWidthStep(double pulse_width_step)
              { _pulseWidthStep = pulse_width_step; };
    void  SetOffset(double offset) { _offset = offset; };
    void  SetOffsetStep(double offset_step) { _offsetStep = offset_step; };
    void  SetLookAngle(double look_angle) { _lookAngle = look_angle * dtr; };
    void  SetBeamWidth(double beam_width) { _beamWidth = beam_width * dtr; };
    void  SetInFlight(int in_flight) { _inFlight = in_flight; };
    void  SetAngleBuffer(double angle_buffer)
              { _angleBuffer = angle_buffer * dtr; };
    void  SetTimeBuffer(double time_buffer) { _timeBuffer = time_buffer; };

    void  FullBuffer(double altitude);
    void  NoBuffer(double altitude);

    void  SetRanges();
    int   NextCombo();

    int   GeneratePulses(int count, PulseList* pulse_list, int check = 0);

    int     GetInFlight() { return(_inFlight); };
    double  GetDutyFactor() { return(_pulseWidth / _pri); };

    void    Memorize() { _priMem = _pri; _pulseWidthMem = _pulseWidth;
                _offsetMem = _offset; };
    void    Recall() { _pri = _priMem; _pulseWidth = _pulseWidthMem;
                _offset = _offsetMem; };

    void    SetScaleFactor();
    int     WriteData(FILE* ofp);

protected:
    int     _beamNumber;

    double  _pri;             // seconds
    double  _priMin;
    double  _priMax;
    double  _priStep;
    double  _priMem;

    double  _pulseWidth;      // seconds
    double  _pulseWidthMin;
    double  _pulseWidthMax;
    double  _pulseWidthStep;
    double  _pulseWidthMem;

    double  _offset;          // seconds
    double  _offsetMin;
    double  _offsetMax;
    double  _offsetStep;
    double  _offsetMem;

    double  _lookAngle;       // radians
    double  _lookAngleMin;
    double  _lookAngleMax;

    double  _beamWidth;       // radians

    int     _inFlight;

    double  _angleBuffer;     // radians
    double  _timeBuffer;      // seconds

    double  _rttMin;
    double  _rttMax;
};

//======================================================================
// CLASS
//    BeamControlList
//
// DESCRIPTION
//    The BeamControlList object is a list of BeamControls.
//======================================================================

class BeamControlList : public List<BeamControl>
{
public:
    BeamControlList();
    ~BeamControlList();

    void  FreeContents();

    int     WriteData(FILE* ofp);
};

//======================================================================
// CLASS
//    TotalControl
//
// DESCRIPTION
//    The TotalControl object holds a list of BeamControl objects and
//    a list of Pulse objects. TotalControl makes all decisions.
//======================================================================

class TotalControl
{
public:
    TotalControl();
    ~TotalControl();

    void  SetAltitude(double altitude) { _altitude = altitude; };
    void  FullBuffer();
    void  NoBuffer();
    int   Optimize();
    int   WriteMemSample(const char* filename);

    BeamControlList  beamControlList;
    PulseList        pulseList;

protected:

    double  _altitude;
};

//------------------//
// helper functions //
//------------------//

double  slant_range(double gc_height, double look_angle);
double  round_trip_time(double slant_range);

#endif
