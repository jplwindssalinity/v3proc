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

    int     Config(int beam_number, ConfigList* config_list);

    int     GetPulsesInFlight() { return(_pulsesInFlight); };
    int     GetId() { return(_pulserId); };

    static int     SetNadirLookAngle(double nadir_look_angle);
    int     SetAltitude(double altitude);
    int     SetPulsesInFlight(int pulses_in_flight);
    int     SetPri(double pri);
    int     SetPulseWidth(double pulse_width);
    int     SetOffset(double offset);

    void    GotoFirstCombo();
    int     GotoNextCombo();

    Pulse*  NextPulse();
    double  DutyFactor() { return(_pulseWidth / _pri); };
    void    Memorize();
    void    Recall();
    int     WriteTransmitPulses(FILE* ofp);
    int     WriteEchoes(FILE* ofp);
    int     WriteNadirReturns(FILE* ofp);

private:
    int     _pulserId;

    int     _pulsesInFlight;

    double  _priSet;
    double  _priMinSet;
    double  _priMaxSet;
    double  _priStep;
    double  _pri;
    double  _priMin;
    double  _priMax;
    double  _priMem;

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
    double  _angleBuffer;
    double  _timeBuffer;

    // used for operations
    int     _useBuffer;
    int     _pulseCount;
    double  _rttMin;
    double  _rttMax;

    // these aren't pulser dependent
    static double  _nadirLookAngle;
    static double  _rttMinNadir;
    static double  _rttMaxNadir;
};

double Pulser::_nadirLookAngle;
double Pulser::_rttMinNadir;
double Pulser::_rttMaxNadir;

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

    void    SetAltitude(double altitude);
    int     NeededPulseCount();
    double  Optimize();
    void    GotoFirstCombo();
    int     GotoNextCombo();
    double  DutyFactor();
    void    Memorize();
    void    Recall();
    int     WritePulseTiming(const char* filename);

private:
    double    _altitude;
    PulseList  _pulseList;
};

//------------------//
// helper functions //
//------------------//

double  slant_range(double gc_height, double look_angle);
double  round_trip_time(double slant_range);

#endif
