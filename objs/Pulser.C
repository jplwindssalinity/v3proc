//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_pulser_c[] =
    "@(#) $Id$";

#include <stdlib.h>
#include <math.h>
#include "Pulser.h"
#include "Misc.h"
#include "List.h"

#define QUOTE '"'

//=======//
// Pulse //
//=======//

Pulse::Pulse()
:   pulserId(0), startTransmit(0.0), endTransmit(0.0), startEcho(0.0),
    startPeakEcho(0.0), endPeakEcho(0.0), endEcho(0.0), startNadir(0.0),
    endNadir(0.0)
{
    return;
}

Pulse::~Pulse()
{
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

//----------------------//
// PulseList::AddIfSafe //
//----------------------//

int
PulseList::AddIfSafe(
    Pulse*  new_pulse)
{
    for (Pulse* pulse = GetHead(); pulse; pulse = GetNext())
    {
        // can't transmit when someone else is receiving
        if (new_pulse->startTransmit < pulse->endEcho &&
            new_pulse->endTransmit > pulse->startEcho)
        {
            return(0);
        }

        // can't transmit when someone else is transmitting
        if (new_pulse->startTransmit < pulse->endTransmit &&
            new_pulse->endTransmit > pulse->startTransmit)
        {
            return(0);
        }

        // can't receive when someone else is receiving
        if (new_pulse->startEcho < pulse->endEcho &&
            new_pulse->endEcho > pulse->startEcho)
        {
            return(0);
        }

        // can't receive when someone else is transmitting
        if (new_pulse->startEcho < pulse->endTransmit &&
            new_pulse->endEcho > pulse->startTransmit)
        {
            return(0);
        }
    }
    if (! Append(new_pulse))
    {
        fprintf(stderr, "PulseList::AddIfSafe: error appending pulse\n");
        exit(1);
    }
    return(1);
}

//--------------------------------//
// PulseList::WriteTransmitPulses //
//--------------------------------//

int
PulseList::WriteTransmitPulses(
    int    pulser_id,
    FILE*  ofp)
{
    fprintf(ofp, "0.0 0.0\n");
    for (Pulse* pulse = GetHead(); pulse; pulse = GetNext())
    {
        if (pulse->pulserId == pulser_id)
        {
            fprintf(ofp, "%g %g\n", pulse->startTransmit, 0.0);
            fprintf(ofp, "%g %g\n", pulse->startTransmit, 1.0);
            fprintf(ofp, "%g %g\n", pulse->endTransmit, 1.0);
            fprintf(ofp, "%g %g\n", pulse->endTransmit, 0.0);
        }
    }
    return(1);
}

//------------------------//
// PulseList::WriteEchoes //
//------------------------//

int
PulseList::WriteEchoes(
    int    pulser_id,
    FILE*  ofp)
{
    fprintf(ofp, "0.0 0.0\n");
    for (Pulse* pulse = GetHead(); pulse; pulse = GetNext())
    {
        if (pulse->pulserId == pulser_id)
        {
            fprintf(ofp, "%g %g\n", pulse->startEcho, 0.0);
            fprintf(ofp, "%g %g\n", pulse->startPeakEcho, 0.5);
            fprintf(ofp, "%g %g\n", pulse->endPeakEcho, 0.5);
            fprintf(ofp, "%g %g\n", pulse->endEcho, 0.0);
        }
    }
    return(1);
}

//------------------------------//
// PulseList::WriteNadirReturns //
//------------------------------//

int
PulseList::WriteNadirReturns(
    int    pulser_id,
    FILE*  ofp)
{
    fprintf(ofp, "0.0 0.0\n");
    for (Pulse* pulse = GetHead(); pulse; pulse = GetNext())
    {
        if (pulse->pulserId == pulser_id)
        {
            fprintf(ofp, "%g %g\n", pulse->startNadir, 0.0);
            fprintf(ofp, "%g %g\n", pulse->startNadir, 1.0);
            fprintf(ofp, "%g %g\n", pulse->endNadir, 1.0);
            fprintf(ofp, "%g %g\n", pulse->endNadir, 0.0);
        }
    }
    return(1);
}

//========//
// Pulser //
//========//

Pulser::Pulser()
:   _pulsesInFlight(0),
    _priSet(-1.0), _priMinSet(-1.0), _priMaxSet(-1.0), _priStep(-1.0),
    _pri(-1.0), _priMin(0.0), _priMax(0.0),
    _pulseWidthSet(-1.0), _pulseWidthMinSet(-1.0), _pulseWidthMaxSet(-1.0),
    _pulseWidthStep(-1.0), _pulseWidth(-1.0), _pulseWidthMin(-1.0),
    _pulseWidthMax(-1.0),
    _offsetSet(-1.0), _offsetMinSet(-1.0), _offsetMaxSet(-1.0),
    _offsetStep(-1.0), _offset(-1.0), _offsetMin(-1.0), _offsetMax(-1.0),
    _lookAngle(-1.0), _twoWayBeamWidth(-1.0), _angleBuffer(-1.0),
    _timeBuffer(-1.0)
{
    return;
}

Pulser::~Pulser()
{
    return;
}

//----------------//
// Pulser::Config //
//----------------//

int
Pulser::Config(
    int          pulser_id,
    ConfigList*  config_list)
{
    _pulserId = pulser_id;

    //---------------------------------//
    // convert beam number to a string //
    //---------------------------------//

    char number[8];
    sprintf(number, "%d", pulser_id);

    char keyword[1024];
    config_list->DoNothingForMissingKeywords();

    //---------------------------//
    // get info from config list //
    //---------------------------//

    substitute_string("BEAM_x_PULSES_IN_FLIGHT", "x", number, keyword);
    config_list->GetInt(keyword, &_pulsesInFlight);

    //-----//
    // PRI //
    //-----//

    substitute_string("BEAM_x_PRI_MIN", "x", number, keyword);
    config_list->GetDouble(keyword, &_priMinSet);

    substitute_string("BEAM_x_PRI_MAX", "x", number, keyword);
    config_list->GetDouble(keyword, &_priMaxSet);

    substitute_string("BEAM_x_PRI_STEP", "x", number, keyword);
    config_list->GetDouble(keyword, &_priStep);

    substitute_string("BEAM_x_PRI", "x", number, keyword);
    if (config_list->GetDouble(keyword, &_priSet))
    {
        _priMinSet = _priSet;
        _priMin = _priMinSet;
        _priMaxSet = _priSet;
        _priMax = _priMaxSet;
        _priStep = 1.0;    // as long as it is > 0
    }
    else
    {
        if (_priStep < 0.0)
        {
            fprintf(stderr, "Pulser::Config: a PRI step is needed\n");
            exit(1);
        }
    }

    //-------------//
    // pulse width //
    //-------------//

    substitute_string("BEAM_x_PULSE_WIDTH_MIN", "x", number, keyword);
    config_list->GetDouble(keyword, &_pulseWidthMinSet);

    substitute_string("BEAM_x_PULSE_WIDTH_MAX", "x", number, keyword);
    config_list->GetDouble(keyword, &_pulseWidthMaxSet);

    substitute_string("BEAM_x_PULSE_WIDTH_STEP", "x", number, keyword);
    config_list->GetDouble(keyword, &_pulseWidthStep);

    substitute_string("BEAM_x_PULSE_WIDTH", "x", number, keyword);
    if (config_list->GetDouble(keyword, &_pulseWidthSet))
    {
        _pulseWidthMinSet = _pulseWidthSet;
        _pulseWidthMin = _pulseWidthMinSet;
        _pulseWidthMaxSet = _pulseWidthSet;
        _pulseWidthMax = _pulseWidthMaxSet;
        _pulseWidthStep = 1.0;    // as long as it is > 0
    }
    else
    {
        if (_pulseWidthStep < 0.0)
        {
            fprintf(stderr, "Pulser::Config: a pulse width step is needed\n");
            exit(1);
        }
        _pulseWidthMin = MAX(_pulseWidthStep, _pulseWidthMinSet);
    }

    //--------//
    // offset //
    //--------//

    substitute_string("BEAM_x_OFFSET_MIN", "x", number, keyword);
    config_list->GetDouble(keyword, &_offsetMin);

    substitute_string("BEAM_x_OFFSET_MAX", "x", number, keyword);
    config_list->GetDouble(keyword, &_offsetMax);

    substitute_string("BEAM_x_OFFSET_STEP", "x", number, keyword);
    config_list->GetDouble(keyword, &_offsetStep);

    substitute_string("BEAM_x_OFFSET", "x", number, keyword);
    if (config_list->GetDouble(keyword, &_offset))
    {
        _offsetMinSet = _offsetSet;
        _offsetMin = _offsetMinSet;
        _offsetMaxSet = _offsetSet;
        _offsetMax = _offsetMaxSet;
        _offsetStep = 1.0;    // as long as it is > 0
    }
    else
    {
        if (_offsetStep < 0.0)
        {
            fprintf(stderr, "Pulser::Config: an offset step is needed\n");
            exit(1);
        }
        _offsetMin = MAX(_offsetStep, _offsetMinSet);
    }

    //--------//
    // others //
    //--------//

    substitute_string("BEAM_x_LOOK_ANGLE", "x", number, keyword);
    config_list->GetDouble(keyword, &_lookAngle);
    _lookAngle *= dtr;

    substitute_string("BEAM_x_TWO_WAY_BEAM_WIDTH", "x", number, keyword);
    config_list->GetDouble(keyword, &_twoWayBeamWidth);
    _twoWayBeamWidth *= dtr;

    substitute_string("BEAM_x_ANGLE_BUFFER", "x", number, keyword);
    config_list->GetDouble(keyword, &_angleBuffer);
    _angleBuffer *= dtr;

    substitute_string("BEAM_x_TIME_BUFFER", "x", number, keyword);
    config_list->GetDouble(keyword, &_timeBuffer);

    return(1);
}

//---------------------------//
// Pulser::SetNadirLookAngle //
//---------------------------//

int
Pulser::SetNadirLookAngle(
    double  nadir_look_angle)
{
    _nadirLookAngle = nadir_look_angle;
    return(1);
}

//---------------------//
// Pulser::SetAltitude //
//---------------------//

int
Pulser::SetAltitude(
    double  altitude)
{
    //--------------------------------//
    // do some calculations on timing //
    //--------------------------------//

    double look_angle_min = _lookAngle - _twoWayBeamWidth / 2.0 - _angleBuffer;
    double look_angle_max = _lookAngle + _twoWayBeamWidth / 2.0 + _angleBuffer;
    double gch = r1_earth + altitude;
    double slant_range_min = slant_range(gch, look_angle_min);
    double slant_range_max = slant_range(gch, look_angle_max);
    _rttMin = round_trip_time(slant_range_min) - _timeBuffer;
    _rttMax = round_trip_time(slant_range_max) + _timeBuffer;

    // nadir
    slant_range_min = slant_range(gch, 0.0);
    slant_range_max = slant_range(gch, _nadirLookAngle);
    _rttMinNadir = round_trip_time(slant_range_min) - _timeBuffer;
    _rttMaxNadir = round_trip_time(slant_range_max) + _timeBuffer;

    return(1);
}

//---------------------------//
// Pulser::SetPulsesInFlight //
//---------------------------//

int
Pulser::SetPulsesInFlight(
    int  pulses_in_flight)
{
    _pulsesInFlight = pulses_in_flight;

    // need to change pri min
    _priMin = _rttMax / (double)_pulsesInFlight;
    if (_priMinSet >= 0.0)
        _priMin = MAX(_priMin, _priMinSet);
    SetPri(_priMin);

    // need to change pri max
    _priMax = _rttMin / (double)(_pulsesInFlight - 1);
    if (_priMaxSet >= 0.0)
        _priMax = MIN(_priMax, _priMaxSet);

    return(1);
}

//----------------//
// Pulser::SetPri //
//----------------//

int
Pulser::SetPri(
    double  pri)
{
printf("%g\n", pri);
    if (pri <= _priMax)
    {
        _pri = pri;

        // need to change pulse width max
        _pulseWidthMax = _pri / 2.0;
        if (_pulseWidthMaxSet >= 0.0)
            _pulseWidthMax = MIN(_pulseWidthMax, _pulseWidthMaxSet);

        // need to change offset max
        _offsetMax = _pri;
        if (_offsetMaxSet >= 0.0)
            _offsetMax = MIN(_offsetMax, _offsetMaxSet);

        return(1);
    }
    else
    {
        _pri = _priMin;
        return(0);
    }
}

//-----------------------//
// Pulser::SetPulseWidth //
//-----------------------//

int
Pulser::SetPulseWidth(
    double  pulse_width)
{
    if (pulse_width <= _pulseWidthMax)
    {
        _pulseWidth = pulse_width;
        return(1);
    }
    else
    {
        _pulseWidth = _pulseWidthMin;
        return(0);
    }
}

//-------------------//
// Pulser::SetOffset //
//-------------------//

int
Pulser::SetOffset(
    double  offset)
{
    if (offset <= _offsetMax)
    {
        _offset = offset;
        return(1);
    }
    else
    {
        _offset = _offsetMin;
        return(0);
    }
}

//------------------------//
// Pulser::GotoFirstCombo //
//------------------------//

void
Pulser::GotoFirstCombo()
{
    SetPulsesInFlight(_pulsesInFlight);    // must be set first!
    SetPri(_priMin);
    SetPulseWidth(_pulseWidthMin);
    SetOffset(_offsetMin);
    _pulseCount = 0;

    return;
}

//-----------------------//
// Pulser::GotoNextCombo //
//-----------------------//

int
Pulser::GotoNextCombo()
{
    _pulseCount = 0;

    // offset
    if (SetOffset(_offset + _offsetStep))
        return(1);

    // pulse width
    if (SetPulseWidth(_pulseWidth + _pulseWidthStep))
        return(1);

    // pri
    if (SetPri(_pri + _priStep))
        return(1);

    // pulses in flight
    // this one doesn't change yet, there is no next combo
    // we're done.

    return(0);
}

//-------------------//
// Pulser::NextPulse //
//-------------------//

Pulse*
Pulser::NextPulse()
{
    //----------------------//
    // allocate a new pulse //
    //----------------------//

    Pulse* new_pulse = new Pulse();
    if (new_pulse == NULL)
    {
        // don't bother going on if this happens
        fprintf(stderr, "Pulser::NextPulse: error allocating pulse\n");
        exit(1);
    }

    //--------------------//
    // set the pulse info //
    //--------------------//

    new_pulse->startTransmit = _pulseCount * _pri + _offset;
    new_pulse->endTransmit = new_pulse->startTransmit + _pulseWidth;
    new_pulse->startEcho = new_pulse->startTransmit + _rttMin;
    new_pulse->startPeakEcho = new_pulse->startTransmit + _rttMin +
        _pulseWidth;
    new_pulse->endPeakEcho = new_pulse->startTransmit + _rttMax;
    new_pulse->endEcho = new_pulse->startTransmit + _rttMax + _pulseWidth;

    new_pulse->startNadir = new_pulse->startTransmit + _rttMinNadir;
    new_pulse->endNadir = new_pulse->startTransmit + _rttMaxNadir +
        _pulseWidth;

    _pulseCount++;

    return(new_pulse);
}

//------------------//
// Pulser::Memorize //
//------------------//

void
Pulser::Memorize()
{
    _priMem = _pri;
    _pulseWidthMem = _pulseWidth;
    _offsetMem = _offset;
    return;
}

//----------------//
// Pulser::Recall //
//----------------//

void
Pulser::Recall()
{
    _pri = _priMem;
    _pulseWidth = _pulseWidthMem;
    _offset = _offsetMem;
    return;
}

//===============//
// PulserCluster //
//===============//

PulserCluster::PulserCluster()
{
    return;
}

PulserCluster::~PulserCluster()
{
    FreeContents();
    return;
}

//-----------------------------//
// PulserCluster::FreeContents //
//-----------------------------//

void
PulserCluster::FreeContents()
{
    Pulser* pulser;
    GotoHead();
    while ((pulser = RemoveCurrent()) != NULL)
        delete pulser;

    return;
}

//----------------------------//
// PulserCluster::SetAltitude //
//----------------------------//

void
PulserCluster::SetAltitude(
    double  altitude)
{
    for (Pulser* pulser = GetHead(); pulser; pulser = GetNext())
    {
        pulser->SetAltitude(altitude);
    }
    return;
}

//-------------------------------//
// PulserCluster::GotoFirstCombo //
//-------------------------------//

void
PulserCluster::GotoFirstCombo()
{
    for (Pulser* pulser = GetHead(); pulser; pulser = GetNext())
    {
        pulser->GotoFirstCombo();
    }
    return;
}

//------------------------------//
// PulserCluster::GotoNextCombo //
//------------------------------//

int
PulserCluster::GotoNextCombo()
{
    for (Pulser* pulser = GetHead(); pulser; pulser = GetNext())
    {
        // as soon as you can go to the next combo in a pulser,
        // you are done
        if (pulser->GotoNextCombo())
            return(1);
    }
    return(0);    // no more combos
}

//---------------------------------//
// PulserCluster::NeededPulseCount //
//---------------------------------//
// determine the number of pulses needed for an evaluation

int
PulserCluster::NeededPulseCount()
{
    int max_in_flight = 0;
    int product_in_flight = 1;
    for (Pulser* pulser = GetHead(); pulser; pulser = GetNext())
    {
        int in_flight = pulser->GetPulsesInFlight();
        if (in_flight > max_in_flight)
            max_in_flight = in_flight;

        product_in_flight *= in_flight;
    }

    int use_pulse_count = max_in_flight;
    for (int i = max_in_flight; i < product_in_flight; i++)
    {
        int good = 1;
        for (Pulser* pulser = GetHead(); pulser; pulser = GetNext())
        {
            int in_flight = pulser->GetPulsesInFlight();
            if (i % in_flight != 0)
            {
                good = 0;
                break;
            }
        }
        if (good)
        {
            use_pulse_count = i;
            break;
        }
    }
    return(use_pulse_count + 2);
}

//-------------------------//
// PulserCluster::Optimize //
//-------------------------//

double
PulserCluster::Optimize()
{
    //---------------------------------------------------------//
    // determine the number of pulses needed for an evaluation //
    //---------------------------------------------------------//

    int pulse_count = NeededPulseCount();

    //-----------------------//
    // go to the first combo //
    //-----------------------//

    GotoFirstCombo();

    //----------------------//
    // try the combinations //
    //----------------------//

    double max_duty_factor = 0.0;
    double duty_factor = 0.0;
    PulseList pulse_list;
    do
    {
        //---------------------------------//
        // for each of the required pulses //
        //---------------------------------//

        for (int pulse_idx = 0; pulse_idx < pulse_count; pulse_idx++)
        {
            //-----------------//
            // for each pulser //
            //-----------------//

            for (Pulser* pulser = GetHead(); pulser; pulser = GetNext())
            {
                Pulse* new_pulse = pulser->NextPulse();
                if (new_pulse == NULL)
                {
                    fprintf(stderr,
                        "PulserCluster::Optimize: error allocating pulse\n");
                    exit(1);
                }
                if (! pulse_list.AddIfSafe(new_pulse))
                {
                    free(new_pulse);
                    pulse_list.FreeContents();
                    goto donetrying;
                }
            }
        }

        //---------------------------------//
        // check for optimization criteria //
        //---------------------------------//

        duty_factor = DutyFactor();
        if (duty_factor > max_duty_factor)
        {
            max_duty_factor = duty_factor;
            Memorize();
        }

        donetrying:

        //------------------//
        // go to next combo //
        //------------------//

        if (! GotoNextCombo())
            break;
    } while (1);

    return(max_duty_factor);
}

//---------------------------//
// PulserCluster::DutyFactor //
//---------------------------//

double
PulserCluster::DutyFactor()
{
    // duty factor's just add
    double duty_factor_sum = 0.0;
    for (Pulser* pulser = GetHead(); pulser; pulser = GetNext())
    {
        duty_factor_sum += pulser->DutyFactor();
    }
    return(duty_factor_sum);
}

//-------------------------//
// PulserCluster::Memorize //
//-------------------------//

void
PulserCluster::Memorize()
{
    for (Pulser* pulser = GetHead(); pulser; pulser = GetNext())
    {
        pulser->Memorize();
    }
    return;
}

//-----------------------//
// PulserCluster::Recall //
//-----------------------//

void
PulserCluster::Recall()
{
    for (Pulser* pulser = GetHead(); pulser; pulser = GetNext())
    {
        pulser->Recall();
    }
    return;
}

//---------------------------------//
// PulserCluster::WritePulseTiming //
//---------------------------------//

int
PulserCluster::WritePulseTiming(
    const char*  filename)
{
    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    for (Pulser* pulser = GetHead(); pulser; pulser = GetNext())
    {
        if (! _pulseList.WriteTransmitPulses(pulser->GetId(), ofp))
            return(0);
        fprintf(ofp, "&\n");
    }

    for (Pulser* pulser = GetHead(); pulser; pulser = GetNext())
    {
        if (! _pulseList.WriteEchoes(pulser->GetId(), ofp))
            return(0);
        fprintf(ofp, "&\n");
    }

    for (Pulser* pulser = GetHead(); pulser; pulser = GetNext())
    {
        if (! _pulseList.WriteNadirReturns(pulser->GetId(), ofp))
            return(0);
        fprintf(ofp, "&\n");
    }

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
