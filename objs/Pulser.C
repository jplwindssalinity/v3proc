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
:   _pulserId(0), _pulseWidthSet(-1.0), _pulseWidthMinSet(-1.0),
    _pulseWidthMaxSet(-1.0), _pulseWidthStep(-1.0), _pulseWidth(-1.0),
    _pulseWidthMin(-1.0), _pulseWidthMax(-1.0), _offsetSet(-1.0),
    _offsetMinSet(-1.0), _offsetMaxSet(-1.0), _offsetStep(-1.0),
    _offset(-1.0), _offsetMin(-1.0), _offsetMax(-1.0), _lookAngle(-1.0),
    _twoWayBeamWidth(-1.0), _pulseCount(0), _rttMin(0.0), _rttMax(0.0)
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

    return(1);
}

//-----------------//
// Pulser::SetRtts //
//-----------------//

int
Pulser::SetRtts(
    double  altitude,
    double  angle_buffer,
    double  time_buffer)
{
    //--------------------------------//
    // do some calculations on timing //
    //--------------------------------//

    double look_angle_min = _lookAngle - _twoWayBeamWidth / 2.0 - angle_buffer;
    double look_angle_max = _lookAngle + _twoWayBeamWidth / 2.0 + angle_buffer;
    double gch = r1_earth + altitude;
    double slant_range_min = slant_range(gch, look_angle_min);
    double slant_range_max = slant_range(gch, look_angle_max);
    _rttMin = round_trip_time(slant_range_min) - time_buffer;
    _rttMax = round_trip_time(slant_range_max) + time_buffer;

    return(1);
}

//--------------------------//
// Pulser::SetPulseWidthMax //
//--------------------------//

void
Pulser::SetPulseWidthMax(
    double  pulse_width_max)
{
    if (_pulseWidthMaxSet >= 0.0)
        _pulseWidthMax = MIN(pulse_width_max, _pulseWidthMaxSet);
    else
        _pulseWidthMax = pulse_width_max;
    return;
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

//----------------------//
// Pulser::SetOffsetMax //
//----------------------//

void
Pulser::SetOffsetMax(
    double  offset_max)
{
    if (_offsetMaxSet >= 0.0)
        _offsetMax = MIN(offset_max, _offsetMaxSet);
    else
        _offsetMax = offset_max;
    return;
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

    return(0);
}

//-------------------//
// Pulser::NextPulse //
//-------------------//

Pulse*
Pulser::NextPulse(
    double  pri)
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

    new_pulse->startTransmit = _pulseCount * pri + _offset;
    new_pulse->endTransmit = new_pulse->startTransmit + _pulseWidth;
    new_pulse->startEcho = new_pulse->startTransmit + _rttMin;
    new_pulse->startPeakEcho = new_pulse->endTransmit + _rttMin;
    new_pulse->endPeakEcho = new_pulse->startTransmit + _rttMax;
    new_pulse->endEcho = new_pulse->endTransmit + _rttMax;

    _pulseCount++;

    return(new_pulse);
}

//------------------//
// Pulser::Memorize //
//------------------//

void
Pulser::Memorize()
{
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
    _pulseWidth = _pulseWidthMem;
    _offset = _offsetMem;
    return;
}

//===============//
// PulserCluster //
//===============//

PulserCluster::PulserCluster()
:   _altitude(0.0), _pulsesInFlight(0),  _priSet(-1.0), _priMinSet(-1.0),
    _priMaxSet(-1.0), _priStep(-1.0), _pri(-1.0), _priMin(-1.0),
    _priMax(-1.0), _priMem(-1.0), _clusterRttMin(0.0), _clusterRttMax(0.0),
    _rttMinNadir(-1.0), _rttMaxNadir(-1.0), _angleBuffer(-1.0),
    _timeBuffer(-1.0)
{
    return;
}

PulserCluster::~PulserCluster()
{
    FreeContents();
    _pulseList.FreeContents();
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

//-----------------------//
// PulserCluster::Config //
//-----------------------//

int
PulserCluster::Config(
    ConfigList*  config_list)
{
    //------------------------//
    // get pulsercluster info //
    //------------------------//

    config_list->ExitForMissingKeywords();

    int pulses_in_flight;
    config_list->GetInt("PULSES_IN_FLIGHT", &pulses_in_flight);
    SetPulsesInFlight(pulses_in_flight);

    double nadir_look_angle;
    config_list->GetDouble("NADIR_LOOK_ANGLE", &nadir_look_angle);

    config_list->GetDouble("ALTITUDE", &_altitude);

    //---------------------------//
    // set the nadir slant range //
    //---------------------------//

    double gch = r1_earth + _altitude;
    double slant_range_min = slant_range(gch, 0.0);
    double slant_range_max = slant_range(gch, nadir_look_angle);
    _rttMinNadir = round_trip_time(slant_range_min) - _timeBuffer;
    _rttMaxNadir = round_trip_time(slant_range_max) + _timeBuffer;

    //-----//
    // PRI //
    //-----//

    config_list->DoNothingForMissingKeywords();
    config_list->GetDouble("PRI_MIN", &_priMinSet);
    config_list->GetDouble("PRI_MAX", &_priMaxSet);
    config_list->GetDouble("PRI_STEP", &_priStep);
    if (config_list->GetDouble("PRI", &_priSet))
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
            fprintf(stderr, "PulserCluster::Config: a PRI step is needed\n");
            return(0);
        }
    }

    //---------//
    // buffers //
    //---------//

    config_list->GetDouble("ANGLE_BUFFER", &_angleBuffer);
    _angleBuffer *= dtr;

    config_list->GetDouble("TIME_BUFFER", &_timeBuffer);

    //-----------------------------------------//
    // determine the number of pulsers to make //
    //-----------------------------------------//

    int number_of_pulsers;
    config_list->GetInt("NUMBER_OF_BEAMS", &number_of_pulsers);

    for (int pulser_id = 1; pulser_id <= number_of_pulsers; pulser_id++)
    {
        Pulser* new_pulser = new Pulser();
        if (new_pulser == NULL)
        {
            fprintf(stderr, "PulserCluster::Config: ");
            fprintf(stderr, "error creating pulser for beam %d\n", pulser_id);
            return(0);
        }
        if (! new_pulser->Config(pulser_id, config_list))
        {
            fprintf(stderr, "PulserCluster::Config: ");
            fprintf(stderr, "error configuring pulser for beam %d\n",
                pulser_id);
            return(0);
        }
        if (! Append(new_pulser))
        {
            fprintf(stderr, "PulserCluster::Config: ");
            fprintf(stderr,
                "error adding pulser to pulser cluster for beam %d\n",
                pulser_id);
            return(0);
        }
    }

    return(1);
}

//------------------------//
// PulserCluster::SetRtts //
//------------------------//

void
PulserCluster::SetRtts(
    int  use_buffer)
{
    //----------------------------------------------------------//
    // set the round trip times for the pulsers and the cluster //
    //----------------------------------------------------------//

    _clusterRttMin = 0.0;
    _clusterRttMax = 0.0;
    for (Pulser* pulser = GetHead(); pulser; pulser = GetNext())
    {
        if (use_buffer)
            pulser->SetRtts(_altitude, _angleBuffer, _timeBuffer);
        else
            pulser->SetRtts(_altitude);
        double pulser_min_rtt = pulser->GetRttMin();
        double pulser_max_rtt = pulser->GetRttMax();
        if (_clusterRttMin == 0.0)
        {
            // haven't set it yet, just use the min and max rtt
            _clusterRttMin = pulser_min_rtt;
            _clusterRttMax = pulser_max_rtt;
        }
        else
        {
            if (pulser_min_rtt < _clusterRttMin)
                _clusterRttMin = pulser_min_rtt;
            if (pulser_max_rtt > _clusterRttMax)
                _clusterRttMax = pulser_max_rtt;
        }
    }

    return;
}

//----------------------------------//
// PulserCluster::SetPulsesInFlight //
//----------------------------------//

int
PulserCluster::SetPulsesInFlight(
    int  pulses_in_flight)
{
    _pulsesInFlight = pulses_in_flight;

    // need to change pri min
    _priMin = _clusterRttMax / (double)_pulsesInFlight;
    if (_priMinSet >= 0.0)
        _priMin = MAX(_priMin, _priMinSet);
    SetPri(_priMin);

    // need to change pri max
    if (_pulsesInFlight == 1)
    {
        // technically, there is no PRI max, but we will make up one.
        // assuming that the largest pulse width is rtt_min,
        // we will calculate the PRI associated with pulsing and
        // receiving each beam sequentially.
        // i.e. rtt_min + rtt_max time for each beam
        _priMax = 0.0;
        for (Pulser* pulser = GetHead(); pulser; pulser = GetNext())
        {
            _priMax += pulser->GetRttMin();
            _priMax += pulser->GetRttMax();
        }
    }
    else
    {
        // interlaced gives us a "true" max PRI
        _priMax = _clusterRttMin / (double)(_pulsesInFlight - 1);
    }
    if (_priMaxSet >= 0.0)
        _priMax = MIN(_priMax, _priMaxSet);

    return(1);
}

//-----------------------//
// PulserCluster::SetPri //
//-----------------------//

int
PulserCluster::SetPri(
    double  pri)
{
    if (pri <= _priMax)
    {
        _pri = pri;

        // need to change pulse width max and offset max
        double pulse_width_max = _pri / 2.0;
        double offset_max = _pri;
        for (Pulser* pulser = GetHead(); pulser; pulser = GetNext())
        {
            pulser->SetPulseWidthMax(pulse_width_max);
            pulser->SetOffsetMax(offset_max);
        }

        return(1);
    }
    else
    {
        _pri = _priMin;
        return(0);
    }
}

//-------------------------------//
// PulserCluster::GotoFirstCombo //
//-------------------------------//

void
PulserCluster::GotoFirstCombo()
{
    SetPulsesInFlight(_pulsesInFlight);
    SetPri(_priMin);
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

    // pri
    if (SetPri(_pri + _priStep))
        return(1);

    // pulses in flight
    // this one doesn't change yet, there is no next combo
    // we're done.

    return(0);    // no more combos
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

    int pulse_count = _pulsesInFlight + 2;

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
                //----------------//
                // set pulse info //
                //----------------//

                Pulse* new_pulse = pulser->NextPulse(_pri);
                if (new_pulse == NULL)
                {
                    fprintf(stderr,
                        "PulserCluster::Optimize: error allocating pulse\n");
                    exit(1);
                }

                //----------------//
                // set nadir info //
                //----------------//

               new_pulse->startNadir = new_pulse->startTransmit + _rttMinNadir;
               new_pulse->endNadir = new_pulse->endTransmit + _rttMaxNadir;

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
        duty_factor_sum += pulser->DutyFactor(_pri);
    }
    return(duty_factor_sum);
}

//-------------------------//
// PulserCluster::Memorize //
//-------------------------//

void
PulserCluster::Memorize()
{
    _priMem = _pri;
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
    _pri = _priMem;
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
