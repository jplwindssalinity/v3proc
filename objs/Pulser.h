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
#include "ConfigList.h"

//======================================================================
// CLASSES
//
//======================================================================

//======================================================================
// CLASS
//    Pulse
//
// DESCRIPTION
//    The Pulse object holds information about a single pulse.
//======================================================================

class Pulse
{
public:
    Pulse();
    ~Pulse();

    int     pulserId;

    double  startTransmit;
    double  endTransmit;
    double  startEcho;
    double  startPeakEcho;
    double  endPeakEcho;
    double  endEcho;
    double  startNadir;
    double  endNadir;
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

    int   AddIfSafe(Pulse* new_pulse);

    int   WriteTransmitPulses(int beam_idx, FILE* fp);
    int   WriteEchoes(int beam_idx, FILE* fp);
    int   WriteNadirReturns(int beam_idx, FILE* fp);
};

//======================================================================
// CLASS
//    Pulser
//
// DESCRIPTION
//    The Pulser object holds information about pulsing for a single
//    beam.
//======================================================================

class Pulser
{
public:
    Pulser();
    ~Pulser();

    int     Config(int pulser_id, ConfigList* config_list);

    int     GetId() { return(_pulserId); };
    double  GetRttMin() { return(_rttMin); };
    double  GetRttMax() { return(_rttMax); };

    int     SetRtts(double altitude, double angle_buffer = 0.0,
                double time_buffer = 0.0);
    int     SetPulsesInFlight(int pulses_in_flight);
    int     SetPri(double pri);
    void    SetPulseWidthMax(double pulse_width_max);
    int     SetPulseWidth(double pulse_width);
    void    SetOffsetMax(double offset_max);
    int     SetOffset(double offset);

    void    GotoFirstCombo();
    int     GotoNextCombo();

    Pulse*  NextPulse(double pri);
    double  DutyFactor(double pri) { return(_pulseWidth / pri); };
    void    Memorize();
    void    Recall();
    int     WriteTransmitPulses(FILE* ofp);
    int     WriteEchoes(FILE* ofp);
    int     WriteNadirReturns(FILE* ofp);

private:
    int     _pulserId;

    double  _pulseWidthSet;
    double  _pulseWidthMinSet;
    double  _pulseWidthMaxSet;
    double  _pulseWidthStep;
    double  _pulseWidth;
    double  _pulseWidthMin;
    double  _pulseWidthMax;
    double  _pulseWidthMem;

    double  _offsetSet;
    double  _offsetMinSet;
    double  _offsetMaxSet;
    double  _offsetStep;
    double  _offset;
    double  _offsetMin;
    double  _offsetMax;
    double  _offsetMem;

    double  _lookAngle;
    double  _twoWayBeamWidth;

    // used for operations
    int     _pulseCount;
    double  _rttMin;
    double  _rttMax;
};

//======================================================================
// CLASS
//    PulserCluster
//
// DESCRIPTION
//    The PulserCluster object is a list of Pulsers that also holds
//    global information (such as spacecraft altitude) that applies
//    to all pulsers.
//======================================================================

class PulserCluster : public List<Pulser>
{
public:
    PulserCluster();
    ~PulserCluster();

    void    FreeContents();

    int     Config(ConfigList* config_list);

    void    SetRtts(int use_buffer);
    int     SetPulsesInFlight(int pulses_in_flight);
    int     SetPri(double pri);

    int     NeededPulseCount();
    double  Optimize();
    void    GotoFirstCombo();
    int     GotoNextCombo();
    double  DutyFactor();
    void    Memorize();
    void    Recall();
    int     WritePulseTiming(const char* filename);

private:
    double  _altitude;

    int     _pulsesInFlight;

    double  _priSet;
    double  _priMinSet;
    double  _priMaxSet;
    double  _priStep;
    double  _pri;
    double  _priMin;
    double  _priMax;
    double  _priMem;

    double  _clusterRttMin;
    double  _clusterRttMax;

    double  _rttMinNadir;
    double  _rttMaxNadir;

    double  _angleBuffer;
    double  _timeBuffer;

    PulseList  _pulseList;
};

//------------------//
// helper functions //
//------------------//

double  slant_range(double gc_height, double look_angle);
double  round_trip_time(double slant_range);

#endif
