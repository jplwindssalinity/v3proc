//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_pulse_c[] =
    "@(#) $Id$";

#include <stdlib.h>
#include <math.h>
#include "Pulse.h"
#include "List.h"

#define QUOTE '"'

//=======//
// Pulse //
//=======//

double Pulse::scale = 1.0;
char Pulse::units[3] = "s";

Pulse::Pulse()
:   beamNumber(0), pulseWidth(0.0), beamFill(0.0), rttMin(0.0),
    rttMax(0.0), startTransmit(0.0), endTransmit(0.0), startEcho(0.0),
    startPeakEcho(0.0), endPeakEcho(0.0), endEcho(0.0)
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
    int     beam_number,
    double  tx_time,
    double  pulse_width,
    double  rtt_min,
    double  rtt_max)
{
    beamNumber = beam_number;
    pulseWidth = pulse_width;
    beamFill = (rtt_max - rtt_min) / 2.0;
    rttMin = rtt_min;
    rttMax = rtt_max;

    startTransmit = tx_time;
    endTransmit = tx_time + pulse_width;
    startEcho = tx_time + rtt_min;
    startPeakEcho = tx_time + rtt_min + pulse_width;
    endPeakEcho = tx_time + rtt_max;
    endEcho = tx_time + rtt_max + pulse_width;

    return(1);
}

//---------------------------//
// Pulse::WriteTransmitPulse //
//---------------------------//

int
Pulse::WriteTransmitPulse(
    FILE*  fp)
{
    fprintf(fp, "%g %g\n", startTransmit * scale, 0.0);
    fprintf(fp, "%g %g\n", startTransmit * scale, 1.0);
    fprintf(fp, "%g %g\n", endTransmit * scale, 1.0);
    fprintf(fp, "%g %g\n", endTransmit * scale, 0.0);
    return(1);
}

//------------------//
// Pulse::WriteEcho //
//------------------//

int
Pulse::WriteEcho(
    FILE*  fp)
{
    fprintf(fp, "%g %g\n", startEcho * scale, 0.0);
    fprintf(fp, "%g %g\n", startPeakEcho * scale, 0.5);
    fprintf(fp, "%g %g\n", endPeakEcho * scale, 0.5);
    fprintf(fp, "%g %g\n", endEcho * scale, 0.0);
    return(1);
}

//--------------//
// Pulse::Milli //
//--------------//

void
Pulse::Milli()
{
    scale = 1E3;
    sprintf(units, "ms");
    return;
}

//--------------//
// Pulse::Micro //
//--------------//

void
Pulse::Micro()
{
    scale = 1E6;
    sprintf(units, "us");
    return;
}

//===========//
// PulseList //
//===========//

PulseList::PulseList()
{
    return;
}

PulseList::~PulseList()
{
    FreeContents();
    return;
}

//-------------------------//
// PulseList::FreeContents //
//-------------------------//

void
PulseList::FreeContents()
{
    Pulse* pulse;
    GotoHead();
    while ((pulse = RemoveCurrent()) != NULL)
        delete pulse;
    return;
}

//--------------------------------//
// PulseList::WriteTransmitPulses //
//--------------------------------//

int
PulseList::WriteTransmitPulses(
    FILE* fp)
{
    for (Pulse* pulse = GetHead(); pulse; pulse = GetNext())
    {
        if (! pulse->WriteTransmitPulse(fp))
            return(0);
    }
    return(1);
}

//------------------------//
// PulseList::WriteEchoes //
//------------------------//

int
PulseList::WriteEchoes(
    FILE* fp)
{
    for (Pulse* pulse = GetHead(); pulse; pulse = GetNext())
    {
        if (! pulse->WriteEcho(fp))
            return(0);
    }
    return(1);
}

//=============//
// BeamControl //
//=============//

BeamControl::BeamControl()
:   _beamNumber(0), _pri(0.0), _priMin(0.0), _priMax(0.0), _priStep(0.0),
    _priMem(0.0), _pulseWidth(0.0), _pulseWidthMin(0.0), _pulseWidthMax(0.0),
    _pulseWidthStep(0.0), _pulseWidthMem(0.0), _offset(0.0), _offsetMin(0.0),
    _offsetMax(0.0), _offsetStep(0.0), _offsetMem(0.0), _lookAngle(0.0),
    _lookAngleMin(0.0), _lookAngleMax(0.0), _beamWidth(0.0), _inFlight(0),
    _angleBuffer(0.0), _timeBuffer(0.0), _rttMin(0.0), _rttMax(0.0)
{
    return;
}

BeamControl::~BeamControl()
{
    return;
}

//-------------------------//
// BeamControl::FullBuffer //
//-------------------------//

void
BeamControl::FullBuffer(
    double  altitude)
{
    _lookAngleMin = _lookAngle - _beamWidth / 2.0 - _angleBuffer;
    _lookAngleMax = _lookAngle + _beamWidth / 2.0 + _angleBuffer;
    double gch = r1_earth + altitude;
    double slant_range_min = slant_range(gch, _lookAngleMin);
    double slant_range_max = slant_range(gch, _lookAngleMax);
    _rttMin = round_trip_time(slant_range_min) - _timeBuffer;
    _rttMax = round_trip_time(slant_range_max) + _timeBuffer;
    return;
}

//-----------------------//
// BeamControl::NoBuffer //
//-----------------------//

void
BeamControl::NoBuffer(
    double  altitude)
{
    _lookAngleMin = _lookAngle - _beamWidth / 2.0;
    _lookAngleMax = _lookAngle + _beamWidth / 2.0;
    double gch = r1_earth + altitude;
    double slant_range_min = slant_range(gch, _lookAngleMin);
    double slant_range_max = slant_range(gch, _lookAngleMax);
    _rttMin = round_trip_time(slant_range_min);
    _rttMax = round_trip_time(slant_range_max);
    return;
}

//------------------------//
// BeamControl::SetRanges //
//------------------------//

void
BeamControl::SetRanges()
{
    // PRI
    if (_priStep == 0.0)
    {
        _priMin = _pri;
        _priMax = _pri;
    }
    else
    {
        _priMin = _rttMax / (double)_inFlight;
        _priMax = _rttMin / (double)(_inFlight - 1);
        _pri = _priMin;
    }

    // pulse width
    if (_pulseWidthStep == 0.0)
    {
        _pulseWidthMin = _pulseWidth;
        _pulseWidthMax = _pulseWidth;
    }
    else
    {
        _pulseWidthMin = _pulseWidthStep;
        _pulseWidthMax = _priMax / 2.0;    // assumes echowidth = pulsewidth
        _pulseWidth = _pulseWidthMin;
    }

    return;
}

//------------------------//
// BeamControl::NextCombo //
//------------------------//

int
BeamControl::NextCombo()
{
    // PRI
    _pri += _priStep;
    if (_pri <= _priMax)
        return(1);
    _pri = _priMin;

    // pulse width
    _pulseWidth += _pulseWidthStep;
    if (_pulseWidth <= _pulseWidthMax)
        return(1);
    _pulseWidth = _pulseWidthMin;

    return(0);
}

//-----------------------------//
// BeamControl::GeneratePulses //
//-----------------------------//

int
BeamControl::GeneratePulses(
    int         count,
    PulseList*  pulse_list,
    int         check)
{
    //--------------------------//
    // create and append pulses //
    //--------------------------//

    for (int pulse_idx = 0; pulse_idx < count; pulse_idx++)
    {
        double tx_time = (double)pulse_idx * _pri;
        Pulse* new_pulse = new Pulse();
        if (new_pulse == NULL)
            return(0);
        new_pulse->Set(_beamNumber, tx_time, _pulseWidth, _rttMin, _rttMax);

        //---------------------------------//
        // if requested, check for overlap //
        //---------------------------------//

        if (check)
        {
            for (Pulse* pulse = pulse_list->GetHead(); pulse;
                pulse = pulse_list->GetNext())
            {
                // can't be receiving while someone else is transmitting
                if (new_pulse->startTransmit < pulse->endEcho &&
                    new_pulse->endTransmit > pulse->startEcho)
                {
                    // failure!
                    delete new_pulse;
                    return(0);
                }
            }
        }

        if (! pulse_list->Append(new_pulse))
            return(0);
    }

    return(1);
}

//-----------------------------//
// BeamControl::SetScaleFactor //
//-----------------------------//

void
BeamControl::SetScaleFactor()
{
    if (_pri < 1.0 && _pri > 1E-3)
    {
        Pulse::Milli();
    }
    else if (_pri < 1E-3)
    {
        Pulse::Micro();
    }
    return;
}

//------------------------//
// BeamControl::WriteData //
//------------------------//

int
BeamControl::WriteData(
    FILE* ofp)
{
    fprintf(ofp, "# Beam %d\n", _beamNumber);
    fprintf(ofp, "#   PRI = %g %s\n", _pri * Pulse::scale, Pulse::units);
    fprintf(ofp, "#   Pulse width = %g %s\n", _pulseWidth * Pulse::scale,
        Pulse::units);
    fprintf(ofp, "#   Offset = %g %s\n", _offset * Pulse::scale, Pulse::units);
    fprintf(ofp, "#   Look angle = %g\n", _lookAngle * rtd);
    fprintf(ofp, "#   Beam width = %g\n", _beamWidth * rtd);
    fprintf(ofp, "#   In flight = %d\n", _inFlight);
    fprintf(ofp, "@ subtitle %cPRI=%g %s, T=%g %s, %d in air%c\n", QUOTE,
        _pri * Pulse::scale, Pulse::units, _pulseWidth * Pulse::scale,
        Pulse::units, _inFlight, QUOTE);
    fprintf(ofp, "@ xaxis label %c%s%c\n", QUOTE, Pulse::units, QUOTE);

    return(1);
}

//=================//
// BeamControlList //
//=================//

BeamControlList::BeamControlList()
{
    return;
}

BeamControlList::~BeamControlList()
{
    FreeContents();
    return;
}

//-------------------------------//
// BeamControlList::FreeContents //
//-------------------------------//

void
BeamControlList::FreeContents()
{
    BeamControl* bc;
    GotoHead();
    while ((bc = RemoveCurrent()) != NULL)
        delete bc;
    return;
}

//----------------------------//
// BeamControlList::WriteData //
//----------------------------//

int
BeamControlList::WriteData(
    FILE*  ofp)
{
    for (BeamControl* bc = GetHead(); bc; bc = GetNext())
    {
        bc->WriteData(ofp);
    }
    return(1);
}

//==============//
// TotalControl //
//==============//

TotalControl::TotalControl()
:   _altitude(0.0)
{
    return;
}

TotalControl::~TotalControl()
{
    return;
}

//--------------------------//
// TotalControl::FullBuffer //
//--------------------------//

void
TotalControl::FullBuffer()
{
    for (BeamControl* bc = beamControlList.GetHead(); bc;
        bc = beamControlList.GetNext())
    {
        bc->FullBuffer(_altitude);
    }
    return;
}

//------------------------//
// TotalControl::NoBuffer //
//------------------------//

void
TotalControl::NoBuffer()
{
    for (BeamControl* bc = beamControlList.GetHead(); bc;
        bc = beamControlList.GetNext())
    {
        bc->NoBuffer(_altitude);
    }
    return;
}

//------------------------//
// TotalControl::Optimize //
//------------------------//

int
TotalControl::Optimize()
{
    //----------------------//
    // set up a full buffer //
    //----------------------//

    FullBuffer();

    //------------------------------------//
    // determine the ranges of parameters //
    //------------------------------------//

    for (BeamControl* bc = beamControlList.GetHead(); bc;
        bc = beamControlList.GetNext())
    {
        bc->SetRanges();
    }

    //--------------------------//
    // step through all choices //
    //--------------------------//

    double df_max = 0.0;
    do
    {
        BeamControl* bc = beamControlList.GetHead();
        while (! bc->NextCombo())
        {
            bc = beamControlList.GetNext();
            if (bc == NULL)
                break;    // no more choices
        }
        if (bc == NULL)
            break;    // no more choices

        //---------------//
        // is it doable? //
        //---------------//

        int success = 1;
        for (BeamControl* bc = beamControlList.GetHead(); bc;
            bc = beamControlList.GetNext())
        {
            int count = bc->GetInFlight() + 2;
            if (! bc->GeneratePulses(count, &pulseList, 1))
            {
                success = 0;
                break;
            }
        }
        pulseList.FreeContents();
        if (success == 0)
            continue;

        //----------------//
        // is it the best //
        //----------------//

        double df_sum = 0.0;
        double df_count = 0.0;
        for (BeamControl* bc = beamControlList.GetHead(); bc;
            bc = beamControlList.GetNext())
        {
            df_sum += bc->GetDutyFactor();
            df_count++;
        }
        double df_avg = df_sum / df_count;
        if (df_avg > df_max)
        {
            df_max = df_avg;
            for (BeamControl* bc = beamControlList.GetHead(); bc;
              bc = beamControlList.GetNext())
            {
                bc->Memorize();
            }
        }

    } while (1);

    if (df_max == 0.0)
        return(0);

    return(1);
}

//------------------------------//
// TotalControl::WriteMemSample //
//------------------------------//

int
TotalControl::WriteMemSample(
    const char* filename)
{
    pulseList.FreeContents();

    //---------------//
    // remove buffer //
    //---------------//

    NoBuffer();

    //-----------------//
    // generate pulses //
    //-----------------//

    for (BeamControl* bc = beamControlList.GetHead(); bc;
        bc = beamControlList.GetNext())
    {
        bc->Recall();
    }

    for (BeamControl* bc = beamControlList.GetHead(); bc;
        bc = beamControlList.GetNext())
    {
        int count = bc->GetInFlight() + 3;
        bc->GeneratePulses(count, &pulseList);
    }

    //--------//
    // output //
    //--------//

    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    BeamControl* bc = beamControlList.GetHead();
    bc->SetScaleFactor();
    beamControlList.WriteData(ofp);
    pulseList.WriteTransmitPulses(ofp);
    fprintf(ofp, "&\n");
    pulseList.WriteEchoes(ofp);
    fclose(ofp);

    return(1);
}

//-------------//
// slant_range //
//-------------//

double
slant_range(
    double  gc_height,
    double  look_angle)
{
    double value = gc_height * cos(look_angle) -
        sqrt(pow(r1_earth, 2.0) - pow(gc_height, 2.0) *
        pow(sin(look_angle), 2.0));
    return(value);
}

//-----------------//
// round_trip_time //
//-----------------//

double
round_trip_time(
    double  slant_range)
{
    double value = 2.0 * slant_range / speed_light_kps;
    return(value);
}
