//==============================================================//
// Copyright (C) 1998-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_ovwm_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include "Ovwm.h"
#include "Constants.h"
#include "InstrumentGeom.h"
#include "GenericGeom.h"
#include "Interpolate.h"
#include "Misc.h"
#include "BYUXTable.h"
#include "AccurateGeom.h"

//=================//
// OvwmTargetInfo //
//=================//

int
OvwmTargetInfo::WriteAscii()
{
    ScatTargetInfo::WriteAscii();
    printf("Doppler = %g\n", dopplerFreq);
    printf("Baseband = %g\n", basebandFreq);
    return(1);
}


//==========//
// OvwmSes //
//==========//

OvwmSes::OvwmSes()
:   txPulseWidth(0.0), txDoppler(0.0), txFrequency(0.0), rxGateDelay(0.0),
    baseTxFrequency(0.0), pri(0.0), transmitPower(0.0), rxGainEcho(0.0),
    rxGainNoise(0.0), calibrationBias(0.0), chirpRate(0.0),
    noiseBandwidth(0.0), receivePathLoss(0.0), transmitPathLoss(0.0),
    loopbackLoss(0.0), loopbackLossRatio(0.0)
{
    Qtable = (float*)malloc(12*sizeof(float));
    if (Qtable == NULL)
    {
        fprintf(stderr,
            "Error allocating memory while constructing OvwmSes\n");
        exit(1);
    }

    //-----------------------------------------------------------------------//
    // Nominal Q-table (0.5 ms effective gate width) converted to correction
    // relative to the nominal slice widths.
    //   ie., slice(i) bw = Qtable[i]*Bs where Bs is the nominal slice bw.
    //-----------------------------------------------------------------------//

    float Bs = 8314.0;   // Nominal value consistent with q-table.
    float Bg = 46190.0;  // Nominal value consistent with q-table.
    float Be = 10*Bs + 2*Bg;
    Qtable[0] = 0.262445 * Be/Bg;
    Qtable[1] = 0.0474238 * Be/Bs;
    Qtable[2] = 0.0474936 * Be/Bs;
    Qtable[3] = 0.0475268 * Be/Bs;
    Qtable[4] = 0.0475150 * Be/Bs;
    Qtable[5] = 0.0475633 * Be/Bs;
    Qtable[6] = 0.0475408 * Be/Bs;
    Qtable[7] = 0.0475313 * Be/Bs;
    Qtable[8] = 0.0475262 * Be/Bs;
    Qtable[9] = 0.0474972 * Be/Bs;
    Qtable[10] = 0.0474421 * Be/Bs;
    Qtable[11] = 0.262501 * Be/Bg;

    return;
}

OvwmSes::~OvwmSes()
{
    if (Qtable)
        free(Qtable);

    return;
}

//------------------------------//
// OvwmSes::GetRxGateWidth      //
//------------------------------//

float
OvwmSes::GetRxGateWidth(
    int  current_beam_idx)
{
    return(rxGateWidth[current_beam_idx]);
}



//-------------------//
// OvwmSes::GetQRel //
//-------------------//

int
OvwmSes::GetQRel(
    int     rel_slice_idx,
    float*  q_slice)
{
    //-------------------------------------------------------------------//
    // Get the noise energy ratio q from a table. (ref. IOM-3347-98-043) //
    //-------------------------------------------------------------------//
    // the Q table has 12 entries, so 12 should be used as the number
    // of slices in rel_to_abs_idx

    int i;
    if (! rel_to_abs_idx(rel_slice_idx, 12, &i))
    {
        fprintf(stderr, "GetQRel: Error converting relative slice index\n");
        return(0);
    }

    *q_slice = Qtable[i];
    return(1);
}

//-----------------------------//
// OvwmSes::CmdTxPulseWidthDn //
//-----------------------------//

int
OvwmSes::CmdTxPulseWidthDn(
    unsigned char  tx_pulse_width_dn)
{
    txPulseWidth = (float)tx_pulse_width_dn * TX_PULSE_WIDTH_CMD_RESOLUTION;
    return(1);
}

//--------------------//
// OvwmSes::CmdPriDn //
//--------------------//

int
OvwmSes::CmdPriDn(
    unsigned int  pri_dn)
{
    pri = (float)pri_dn * OVWM_PRI_CMD_RESOLUTION;
    return(1);
}

//----------------------------//
// OvwmSes::CmdRxGateWidthDn //
//----------------------------//

int
OvwmSes::CmdRxGateWidthDn(
    int            beam_idx,
    unsigned char  rx_gate_width_dn)
{
    rxGateWidth[beam_idx] = (float)rx_gate_width_dn *
        RX_GATE_WIDTH_CMD_RESOLUTION;
    return(1);
}

//----------------------------//
// OvwmSes::CmdRxGateWidthEu //
//----------------------------//

int
OvwmSes::CmdRxGateWidthEu(
    int    beam_idx,
    float  rx_gate_width_eu)
{
    rxGateWidth[beam_idx] = rx_gate_width_eu;
    return(1);
}

//----------------------------//
// OvwmSes::CmdRxGateDelayDn //
//----------------------------//

int
OvwmSes::CmdRxGateDelayDn(
    unsigned char  rx_gate_delay_dn)
{
    rxGateDelay = (float)rx_gate_delay_dn * RX_GATE_DELAY_CMD_RESOLUTION;
    return(1);
}

//-----------------------------//
// OvwmSes::CmdRxGateDelayFdn //
//-----------------------------//

int
OvwmSes::CmdRxGateDelayFdn(
    float  rx_gate_delay_fdn)
{
    rxGateDelay = rx_gate_delay_fdn * RX_GATE_DELAY_CMD_RESOLUTION;
    return(1);
}

//----------------------------//
// OvwmSes::CmdRxGateDelayEu //
//----------------------------//

int
OvwmSes::CmdRxGateDelayEu(
    float  rx_gate_delay_eu)
{
    rxGateDelay= rx_gate_delay_eu;
    return(1);
}

//--------------------------//
// OvwmSes::CmdTxDopplerDn //
//--------------------------//

int
OvwmSes::CmdTxDopplerDn(
    short  tx_doppler_dn)
{
    txDoppler = (float)tx_doppler_dn * TX_FREQUENCY_CMD_RESOLUTION;
    txFrequency = baseTxFrequency + txDoppler;
    return(1);
}

//--------------------------//
// OvwmSes::CmdTxDopplerEu //
//--------------------------//

int
OvwmSes::CmdTxDopplerEu(
    float  tx_doppler_eu)
{
    txDoppler = tx_doppler_eu;
    txFrequency = baseTxFrequency + txDoppler;
    return(1);
}

//--------------------//
// OvwmSes::tempToDn //
//--------------------//

unsigned char
OvwmSes::tempToDn(
    float  temp)
{
    // DN to EU conversion coefficients
    double a = 0.000328789;
    double b = -0.476764097;
    double c = 83.137271991;

    // invert with quadratic formula
    double dd = b*b - 4*a*(c-temp);
    if (dd < 0.0)
    {
        fprintf(stderr,
            "Error: invalid temperature = %g passed to OvwmSes::tempToDn\n",
            temp);
        exit(1);
    }
    double x1 = (-b + sqrt(dd)) / (2*a);
    double x2 = (-b - sqrt(dd)) / (2*a);
    unsigned char x = 0;
    if (x1 < 0 && x2 < 0)
    {
        fprintf(stderr,
            "Error: invalid temperature = %g passed to OvwmSes::tempToDn\n",
            temp);
        exit(1);
    }
    else if (x1 < 0)
    {
        x = (unsigned char)x2;
    }
    else if (x2 < 0)
    {
        x = (unsigned char)x1;
    }
    else if (x1 < x2)
    {
        if (x1 > 255)
        {
            fprintf(stderr,
                "Error: out of range temp = %g passed to OvwmSes::tempToDn\n",
                temp);
            exit(1);
        }
        x = (unsigned char)x1;
    }
    else
    {
        if (x2 > 255)
        {
            fprintf(stderr,
                "Error: out of range temp = %g passed to OvwmSes::tempToDn\n",
                temp);
            exit(1);
        }
        x = (unsigned char)x2;
    }
    return(x);
}

//==========//
// OvwmSas //
//==========//

OvwmSas::OvwmSas() 
:  encoderOffset(0) 
{
    return;
}

OvwmSas::~OvwmSas()
{
    return;
}

//---------------------------------//
// OvwmSas::SetAzimuthWithEncoder //
//---------------------------------//

#define ENCODER_MASK  0x8000

int
OvwmSas::SetAzimuthWithEncoder(
    unsigned short  encoder)
{


    // mask out the encoder bit (Hopefully I can remove this useless step)
    unsigned short use_encoder = encoder & 0x7fff;

    //---------------------//
    // calculate the angle //
    //---------------------//

    double angle = two_pi * (double)use_encoder / (double)ENCODER_N +
        encoderOffset;

    antenna.SetEncoderAzimuthAngle(angle);

    return(1);
}

//----------------------//
// OvwmSas::GetEncoder //
//----------------------//


unsigned short
OvwmSas::GetEncoder()
{
    unsigned short encoder = AzimuthToEncoder(antenna.encoderAzimuthAngle);
    return(encoder);
}

//----------------------------//
// OvwmSas::AzimuthToEncoder //
//----------------------------//

unsigned short
OvwmSas::AzimuthToEncoder(
    double  azimuth)
{


    //-----------------------//
    // calculate the encoder //
    //-----------------------//

    double angle = azimuth - encoderOffset;
    if (angle < 0.0)
        angle += two_pi;

    unsigned short encoder = (unsigned short)((double)ENCODER_N * angle /
        two_pi + 0.5);

    encoder %= ENCODER_N;

 
    return(encoder);
}

//-----------------------//
// OvwmSas::SetSpinRate //
//-----------------------//

int
OvwmSas::SetSpinRate( double spin_rate_in_rpm)
{
  antenna.spinRate=spin_rate_in_rpm * rpm_to_radps;
  return(1);
}

//-----------------------//
// OvwmSas::GetSpinRate //
//-----------------------//

double
OvwmSas::GetSpinRate()
{
  return(antenna.spinRate);
}




//==========//
// OvwmCds //
//==========//

OvwmCds::OvwmCds()
:   turnOnTime(-1.0), priDn(0), txPulseWidthDn(0), rxGateDelayDn(0),
    txDopplerDn(0), useRgc(0), useDtc(0), 
    azimuthIntegrationRange(0), azimuthStepSize(0),
    orbitTicksPerOrbit(0), orbitTime(0), orbitStep(0), eqxTime(0.0),
    rawEncoder(0), heldEncoder(0),encoderOffset(0)
{
  // for now set all the beam encoder offsets to zero
  for(int i=0;i<NUMBER_OF_OVWM_BEAMS;i++) beamEncoderOffset[i]=0;
    return;
}

OvwmCds::~OvwmCds()
{
    return;
}

//-------------------//
// OvwmCds::SetTime //
//-------------------//

int
OvwmCds::SetTime(
    double  new_time)
{
    if (turnOnTime < 0.0)
    {
        // this is the first time the time is getting set
        // we will boldly assume this is the turn on time
        turnOnTime = new_time;
    }
    time = new_time;
    double run_time = time - turnOnTime;
    instrumentTime = (unsigned int)(run_time * INSTRUMENT_TICKS_PER_SECOND);

    double time_since_eqx = time - eqxTime;
    orbitTime = (unsigned int)(time_since_eqx * ORBIT_TICKS_PER_SECOND);

    int loops = (int)(orbitTime / orbitTicksPerOrbit);
    orbitTime -= loops * orbitTicksPerOrbit;

    return(1);
}

//----------------------//
// OvwmCds::SetEqxTime //
//----------------------//

int
OvwmCds::SetEqxTime(
    double  eqx_time)
{
    eqxTime = eqx_time;
    return(1);
}

//-------------------------//
// OvwmCds::OrbitFraction //
//-------------------------//

double
OvwmCds::OrbitFraction()
{
    double frac = (double)(orbitTime % orbitTicksPerOrbit) /
        (double)orbitTicksPerOrbit;
    return(frac);
}

//------------------------------//
// OvwmCds::SetAndGetOrbitStep //
//------------------------------//

unsigned short
OvwmCds::SetAndGetOrbitStep()
{
    float ticks_per_orbit_step = (float)orbitTicksPerOrbit /
        (float)ORBIT_STEPS;
    orbitStep = (unsigned short)((float)(orbitTime % orbitTicksPerOrbit) /
        ticks_per_orbit_step);
    orbitStep %= ORBIT_STEPS;
    return(orbitStep);
}

//------------------------------//
// OvwmCds::GetRxGateWidthDn    //
//------------------------------//

unsigned char
OvwmCds::GetRxGateWidthDn()
{
    return(rxGateWidthDn[currentBeamIdx]);
}

//------------------------------//
// OvwmCds::GetDopplerTracker    //
//------------------------------//

DopplerTracker*
OvwmCds::GetDopplerTracker()
{
    return(&(dopplerTracker[currentBeamIdx]));
}


//------------------------------//
// OvwmCds::GetRangeTracker    //
//------------------------------//

RangeTracker*
OvwmCds::GetRangeTracker()
{
    return(&(rangeTracker[currentBeamIdx]));
}


//--------------------------------//
// OvwmCds::EstimateIdealEncoder //
//--------------------------------//
// This returns an ideal encoder estimate.  Basically, a quantized azimuth.
// There are two ways for this function to work:
// 1) Use the previously stored range gate delay (in rxRangeMem)
// 2) Calculate what the previous range gate delay was
// The optional argument, if non-zero, indicates to use method 2
// Option 1 should be used to simulate data, Option 2 should be used
// to process data

unsigned short
OvwmCds::EstimateIdealEncoder(
    int             calculate_prev_delay,
    unsigned short  prev_range_step,
    unsigned short  prev_azimuth_step,
    unsigned char   prev_rx_gate_width_dn,
    unsigned char   prev_tx_pulse_width_dn,
    double spin_rate)
{
    //-------------//
    // dereference //
    //-------------//


    RangeTracker* range_tracker = &(rangeTracker[currentBeamIdx]);

    //---------------------------------------//
    // get previous pulses raw encoder value //
    //---------------------------------------//

    unsigned int int_encoder = (unsigned int)(heldEncoder & 0x7fff);
    // printf("\nInitial encoder value = %d\n", int_encoder);

    //----------------------//
    // apply encoder offset //
    //----------------------//


    int_encoder += encoderOffset;
    // printf("Plus encoder offset = %d\n", int_encoder);

    //-------------------//
    // apply beam offset //
    //-------------------//

    unsigned int beam_offset=beamEncoderOffset[currentBeamIdx];
    int_encoder += beam_offset;
    // printf("Plus beam offset = %d\n", int_encoder);

    //-----------------------------------//
    // determine spin rate in dn per pri //
    //-----------------------------------//


    unsigned short ant_dn_per_pri = (unsigned short)
        ( ( ( ((float)priDn / 10.0) * 32768.0 * spin_rate) /
        (60.0 * 1000.0)) + 0.5);

    //--------------------------------------------//
    // get or calculate previous range gate delay //
    //--------------------------------------------//

    float rx_range_mem;
    if (calculate_prev_delay)
    {
        // do the previous range tracking calculation
        // the rxRangeMem variable will get set

        unsigned char prev_rx_gate_delay_dn;
        float prev_rx_gate_delay_fdn;
        range_tracker->GetRxGateDelay(prev_range_step, prev_azimuth_step,
            prev_rx_gate_width_dn, prev_tx_pulse_width_dn,
            &prev_rx_gate_delay_dn, &prev_rx_gate_delay_fdn);
    }
    rx_range_mem = range_tracker->rxRangeMem;
    // printf("rx_range_mem = %g\n", rx_range_mem);

    //------------------//
    // apply to encoder //
    //------------------//

    int_encoder += (unsigned int)( (float)ant_dn_per_pri * (1.0 +
        (rx_range_mem + (float)txPulseWidthDn) /
        ((float)priDn * 4.0)) + 0.5);
    // printf("Plus delay and centering offset = %d\n", int_encoder);

    //-----------------//
    // mod the encoder //
    //-----------------//

    unsigned short encoder = (unsigned short)(int_encoder % ENCODER_N);

    return(encoder);
}

//-------------------//
// OvwmCds::LoadRgc //
//-------------------//

int
OvwmCds::LoadRgc(
    int          beam_idx,
    const char*  file)
{
    if (! rangeTracker[beam_idx].ReadBinary(file))
        return(0);
    return(1);
}

//-------------------//
// OvwmCds::LoadDtc //
//-------------------//

int
OvwmCds::LoadDtc(
    int          beam_idx,
    const char*  file)
{
    if (! dopplerTracker[beam_idx].ReadBinary(file))
        return(0);
    return(1);
}

//--------------------------------//
// OvwmCds::SetTrackingChirpRate //
//--------------------------------//

void
OvwmCds::SetTrackingChirpRate(
    int     beam_idx,
    double  tracking_mu)
{
    dopplerTracker[beam_idx].SetTrackingChirpRate(tracking_mu);
    return;
}

//-----------------------------//
// OvwmCds::CmdTxPulseWidthEu //
//-----------------------------//

int
OvwmCds::CmdTxPulseWidthEu(
    float      tx_pulse_width,
    OvwmSes*  ovwm_ses)
{
    txPulseWidthDn = (unsigned char)(tx_pulse_width /
        TX_PULSE_WIDTH_CMD_RESOLUTION + 0.5);

    if (! ovwm_ses->CmdTxPulseWidthDn(txPulseWidthDn))
        return(0);

    return(1);
}

//--------------------//
// OvwmCds::CmdPriEu //
//--------------------//

int
OvwmCds::CmdPriEu(
    float      pri,
    OvwmSes*  ovwm_ses)
{
    priDn = (unsigned int)(pri / OVWM_PRI_CMD_RESOLUTION + 0.5);

    if (! ovwm_ses->CmdPriDn(priDn))
        return(0);

    return(1);
}

//----------------------------//
// OvwmCds::CmdRxGateWidthEu //
//----------------------------//

int
OvwmCds::CmdRxGateWidthEu(
    int        beam_idx,
    float      rx_gate_width,
    OvwmSes*  ovwm_ses)
{
    rxGateWidthDn[beam_idx] = (unsigned char)(rx_gate_width /
        RX_GATE_WIDTH_CMD_RESOLUTION + 0.5);

    if (! ovwm_ses->CmdRxGateWidthDn(beam_idx,
        rxGateWidthDn[beam_idx]))
    {
        return(0);
    }

    return(1);
}



//---------------------------------//
// OvwmCds::CmdOrbitTicksPerOrbit //
//---------------------------------//

int
OvwmCds::CmdOrbitTicksPerOrbit(
    unsigned int  orbit_ticks)
{
    orbitTicksPerOrbit = orbit_ticks;
    return(1);
}

//=======//
// Ovwm //
//=======//

Ovwm::Ovwm()
  :   systemLoss(0.0), systemTemperature(0.0)
{
    scatRf = &ses;
    scatDig = &cds;
    scatAnt = &sas;

    return;
}

Ovwm::~Ovwm()
{
    return;
}

//------------------------------//
// Ovwm::GetRxGateWidthDn    //
//------------------------------//

unsigned char
Ovwm::GetRxGateWidthDn()
{
    return(cds.GetRxGateWidthDn());
}

//------------------------------//
// Ovwm::GetNumberOfPixels      //
//------------------------------//
int
Ovwm::GetNumberOfPixels()
{
  if(useRealAperture) return(1);
  else return(ses.numPulses*ses.numRangePixels);
}

//------------------------------//
// Ovwm::GetDopplerTracker    //
//------------------------------//

DopplerTracker*
Ovwm::GetDopplerTracker()
{
    return(cds.GetDopplerTracker());
}


//------------------------------//
// Ovwm::GetRangeTracker    //
//------------------------------//

RangeTracker*
Ovwm::GetRangeTracker()
{
    return(cds.GetRangeTracker());
}

//------------------------------//
// Ovwm::GetRxGateWidth         //
//------------------------------//

float
Ovwm::GetRxGateWidth()
{
    return(ses.GetRxGateWidth(cds.currentBeamIdx));
}


//--------------------------//
// Ovwm::SetEncoderAzimuth //
//--------------------------//

int
Ovwm::SetEncoderAzimuth(
    unsigned short  encoder,
    int             pri_delay)
{
    // set azimuth using encoder
    sas.SetAzimuthWithEncoder(encoder);

    // rotate by pri sampled delay
    double delta_t = (double)pri_delay * ses.pri;
    sas.antenna.SetEncoderAzimuthAngle(sas.antenna.encoderAzimuthAngle +
        delta_t * sas.antenna.spinRate);

    return(1);
}


//----------------------------------------//
// Qsact::SetAllAzimuthsUsingGroundImpact //
//----------------------------------------//
// Takes Ground Impact Azimuth as a parameter sets other angles

int
Ovwm::SetAllAzimuthsUsingGroundImpact(
    Spacecraft*  spacecraft,
    double       angle)
{
    sas.antenna.SetGroundImpactAzimuthAngle(angle);
    if (! GroundImpactToTxCenterAzimuth(spacecraft))
        return(0);
    if (! TxCenterToEncoderAzimuth())
        return(0);
    return(1);
}

//--------------------------------------//
// Ovwm::GroundImpactToTxCenterAzimuth //
//--------------------------------------//
// Assumes Ground Impact Azimuth Angle Has been Set
// Computes and sets TxCenterAzimuth

int
Ovwm::GroundImpactToTxCenterAzimuth(
    Spacecraft*  spacecraft)
{
    // estimate the range to the surface using ground imapct
    Antenna* antenna = &(sas.antenna);
    CoordinateSwitch antenna_frame_to_gc =
        AntennaFrameToGC(&(spacecraft->orbitState), &(spacecraft->attitude),
        antenna, antenna->groundImpactAzimuthAngle);
    double look, azim;
    Beam* beam = GetCurrentBeam();
    if (! beam->GetElectricalBoresight(&look, &azim))
        return(0);
    Vector3 vector;
    vector.SphericalSet(1.0, look, azim);
    OvwmTargetInfo qti;
    if (! TargetInfo(&antenna_frame_to_gc, spacecraft, vector, &qti))
        return(0);

    // rotate to the antenna to the ground impact azimuth
    double delta_t = qti.roundTripTime / 2.0;
    sas.antenna.SetTxCenterAzimuthAngle(antenna->groundImpactAzimuthAngle -
        delta_t * antenna->spinRate);
    return(1);
}

//--------------------------------------//
// Ovwm::TxCenterToGroundImpactAzimuth //
//--------------------------------------//
// assumes txCenterAzimuthAngle has been set.
// computes and sets the ground impact azimuth

int
Ovwm::TxCenterToGroundImpactAzimuth(
    Spacecraft*  spacecraft)
{
    // estimate the range to the surface using tx center
    CoordinateSwitch antenna_frame_to_gc =
        AntennaFrameToGC(&(spacecraft->orbitState), &(spacecraft->attitude),
        &(sas.antenna), sas.antenna.txCenterAzimuthAngle);
    double look, azim;
    Beam* beam = GetCurrentBeam();
    if (! beam->GetElectricalBoresight(&look, &azim))
        return(0);
    Vector3 vector;
    vector.SphericalSet(1.0, look, azim);
    OvwmTargetInfo qti;
    if (! TargetInfo(&antenna_frame_to_gc, spacecraft, vector, &qti))
        return(0);

    // rotate to the antenna to the ground impact azimuth
    double delta_t = qti.roundTripTime / 2.0;
    sas.antenna.SetGroundImpactAzimuthAngle(sas.antenna.txCenterAzimuthAngle +
        delta_t * sas.antenna.spinRate);

    return(1);
}

//---------------------------------//
// Ovwm::TxCenterToEncoderAzimuth //
//---------------------------------//
// Assumes Tx Center Azimuth Angle Has been Set
// Computes and sets UNQUANTIZED Encoder azimuth
int
Ovwm::TxCenterToEncoderAzimuth()
{
    double delta_t = GetEncoderToTxCenterDelay();
    sas.antenna.SetEncoderAzimuthAngle(sas.antenna.txCenterAzimuthAngle -
        delta_t * sas.antenna.spinRate);
    return(1);
}

//----------------------------------//
// Ovwm::GetEncodertoTxCenterDelay //
//----------------------------------//

double
Ovwm::GetEncoderToTxCenterDelay()
{
    return(ses.txPulseWidth / 2.0 - T_ENC + T_GRID + T_RC + T_EXC);
}

//-------------------------//
// Ovwm::SetOtherAzimuths //
//-------------------------//
// this assumes the encoder azimuth has been set

int
Ovwm::SetOtherAzimuths(
    Spacecraft*  spacecraft)
{
    //-----------//
    // Tx center //
    //-----------//

    double delta_t = GetEncoderToTxCenterDelay();
    sas.antenna.SetTxCenterAzimuthAngle(sas.antenna.encoderAzimuthAngle +
        delta_t * sas.antenna.spinRate);

    //---------------//
    // ground impact //
    //---------------//

    if (! TxCenterToGroundImpactAzimuth(spacecraft))
        return(0);

// Set all azimuths to ground impact
//    sas.antenna.txCenterAzimuthAngle = sas.antenna.groundImpactAzimuthAngle;

    return(1);
}

//-------------------//
// Ovwm::MakePixels //
//-------------------//
// This method creates the measurement list (of Meas's) for a spot
// and sets the Meas indices (with one pixel per Meas).

int
Ovwm::MakePixels(
    MeasSpot*  meas_spot)
{

    meas_spot->FreeContents();
    int total_pixels = GetNumberOfPixels();

    for (int slice_idx = 0; slice_idx < total_pixels; slice_idx++) {
        Meas* meas = new Meas();
        meas->startSliceIdx = slice_idx;
        meas->numSlices = 1;
        meas_spot->Append(meas);
    }

    return(1);

}

//---------------------//
// Ovwm::LocatePixels //
//---------------------//
#define DEBUG_LOCATE_PIXELS
int
Ovwm::LocatePixels(
    Spacecraft*  spacecraft,
    MeasSpot*    meas_spot)
{

    //-----------//
    // predigest //
    //-----------//

    Antenna* antenna = &(sas.antenna);
    Beam* beam = GetCurrentBeam();
    OrbitState* orbit_state = &(spacecraft->orbitState);
    Attitude* attitude = &(spacecraft->attitude);

    //------------------//
    // set up meas spot //
    //------------------//

    meas_spot->time = cds.time;
    meas_spot->scOrbitState = *orbit_state;
    meas_spot->scAttitude = *attitude;

    //--------------------------------//
    // generate the coordinate switch //
    //--------------------------------//

    CoordinateSwitch antenna_frame_to_gc = AntennaFrameToGC(orbit_state,
        attitude, antenna, antenna->groundImpactAzimuthAngle);

    //------------------//
    // find beam center //
    //------------------//

    double look, azim;
    if (! beam->GetElectricalBoresight(&look, &azim))
        return(0);
    Vector3 vector;
    vector.SphericalSet(1.0,look,azim);

    
    //fprintf(stderr,"BLookAntFrame %g %g %g\n",vector.Get(0),vector.Get(1),vector.Get(2));
    //-----------------------------------------------//
    // compute boresight vector,range and doppler   //
    //----------------------------------------------//
    OvwmTargetInfo oti;
    if(! TargetInfo(&antenna_frame_to_gc,spacecraft,vector,&oti)){
      fprintf(stderr,"Error:LocatePixels cannot find boresight on surface\n");
      exit(1);
    }
    double boresight_range=oti.slantRange;
    double boresight_doppler=oti.dopplerFreq;

    // compute radius and center for best fit local sphere
    // should probably make this a Earth Position method
    double alt,lon,gclat;
    oti.rTarget.GetAltLonGCLat(&alt,&lon,&gclat);
    double slat=sin(gclat);
    double local_radius=r1_earth*(1-flat*slat*slat);
    EarthPosition local_center;
    local_center.SetAltLonGCLat(-local_radius,lon,gclat);
    Vector3 scpos=spacecraft->orbitState.rsat-local_center;
    double scpos_mag=scpos.Magnitude();
    Vector3 boresight_look=oti.rTarget-spacecraft->orbitState.rsat;
    boresight_look/=boresight_look.Magnitude();
    int num_pixels=GetNumberOfPixels();

    //fprintf(stderr,"BLookGcLook %g %g %g\n",oti.gcLook.Get(0),oti.gcLook.Get(1),oti.gcLook.Get(2));

    //fprintf(stderr,"BLook %g %g %g\n",boresight_look.Get(0),boresight_look.Get(1),boresight_look.Get(2));
    //-------------------//
    // for each slice... //
    //-------------------//


    Meas* meas = meas_spot->GetHead();


    //--------------------//
    // Real aperture case //
    //--------------------//
    if(num_pixels==1){
      CoordinateSwitch gc_to_surface =
	oti.rTarget.SurfaceCoordinateSystem();
      Vector3 rlook_surface = gc_to_surface.Forward(oti.gcLook);
      double m, theta, phi;
      rlook_surface.SphericalGet(&m, &theta, &phi);
      // Note that eastAzimuth is referenced to true East (not North).
      meas->eastAzimuth = phi;
      
      // get incidence angle
      meas->incidenceAngle = pi - theta;
      
      meas->centroid = oti.rTarget;
      
      return(1);
    }

    int nomr_idx=ses.numRangePixels/2;
    int nomd_idx=ses.numPulses/2;

    for (int r_idx = 0; r_idx < ses.numRangePixels; r_idx++){
      
      double r=boresight_range+ses.rangeRes*(r_idx-nomr_idx);

      // compute doppler for given range and boresight azimuth

      // find look for range and 0 azimuth
      double br2=boresight_range*boresight_range;
      double sc2=scpos_mag*scpos_mag;
      double lr2=local_radius*local_radius;
      double boresight_lookang=acos((br2+sc2-lr2)/(2*boresight_range*scpos_mag));
      double r_look=acos((r*r+sc2-lr2)/(2*r*scpos_mag));
      double el=r_look-boresight_lookang;
      Vector3 look2;
      look2.SphericalSet(1,look+el,azim);
      if(r_idx==nomr_idx){
	//fprintf(stderr,"look2AntFrame %g %g %g\n",boresight_look.Get(0),boresight_look.Get(1),boresight_look.Get(2));
      }
      Vector3 dlook21=vector-look2;
      if(r_idx==nomr_idx){
	//fprintf(stderr,"dlook2AntFrame %g %g %g\n",dlook21.Get(0),dlook21.Get(1),dlook21.Get(2));
      }
      OvwmTargetInfo oti2;
      SphericalTargetInfo(&antenna_frame_to_gc,spacecraft,look2,&oti2,r);
      double d=oti2.dopplerFreq;
      for (int d_idx = 0; d_idx < ses.numPulses; d_idx++)
	{
	  //----------------------------------------//
	  // determine pixel doppler and  range     //
	  //----------------------------------------//
          double d2=d+ses.dopplerRes*(nomd_idx-d_idx);
	  double u1[3],u2[3];
          Vector3 plook_vector;
          if(!FastDopplerRangeToTbfLook(d2,r,local_radius,local_center,spacecraft,u1,u2)){
	    meas->centroid=EarthPosition(0,0,0);
#ifdef DEBUG_LOCATE_PIXELS
	    //  fprintf(stderr,"time =%15.15g azim=%g, range=%g dop=%g Pixel not on surface\n",cds.time,antenna->groundImpactAzimuthAngle*rtd,r,d2);
#endif	  
	  } 
          else{
	  // pick look closest to boresight
	  Vector3 look1(u1[0],u1[1],u1[2]);
	  Vector3 look2(u2[0],u2[1],u2[2]);

	  double dot1 =   boresight_look % look1;
	  double dot2 =   boresight_look % look2;
	  if(dot1>dot2) plook_vector=look1;
          else plook_vector=look2;
          if(r_idx==nomr_idx && d_idx==nomd_idx){
	    fprintf(stderr,"BL %g %g %g L1 %g %g %g L2 %g %g %g %g %g\n",
		    boresight_look.Get(0),boresight_look.Get(1),
		    boresight_look.Get(2),look1.Get(0),look1.Get(1),
		    look1.Get(2),look2.Get(0),look2.Get(1),look2.Get(2),dot1,dot2);
	  }
          EarthPosition surfpt=spacecraft->orbitState.rsat+plook_vector*r;
          
	  //---------------------------//
	  // generate measurement data //
	  //---------------------------//
	  
	  // get local measurement azimuth
	  CoordinateSwitch gc_to_surface =
            surfpt.SurfaceCoordinateSystem();
	  Vector3 rlook_surface = gc_to_surface.Forward(plook_vector);
	  double m, theta, phi;
	  rlook_surface.SphericalGet(&m, &theta, &phi);
          // Note that eastAzimuth is referenced to true East (not North).
	  meas->eastAzimuth = phi;
	  
	  // get incidence angle
	  meas->incidenceAngle = pi - theta;

	  meas->centroid = surfpt;

 #ifdef DEBUG_LOCATE_PIXELS
	    double alt,lon,latgd;
	    surfpt.GetAltLonGDLat(&alt,&lon,&latgd);
	    Vector3 beam_plook=antenna_frame_to_gc.Backward(plook_vector);
	    OvwmTargetInfo oti3;
	    if(! TargetInfo(&antenna_frame_to_gc,spacecraft,beam_plook,&oti3)){
	      fprintf(stderr,"Error:LocatePixels cannot find pixel on surface during check\n");
	      exit(1);
	    }
	    double az3,look3, mag;
	    beam_plook.SphericalGet(&mag,&look3,&az3);
	    //fprintf(stderr,"time %15.15g azim %g, dr %g ddop %g dheight %g dlook %g daz %g ridx %d didx %d\n",
	    //	    cds.time,antenna->groundImpactAzimuthAngle*rtd,
	    //	    r-oti3.slantRange,d2-oti3.dopplerFreq,alt,
	    //	    rtd*(look3-(look+el)),rtd*(az3-azim),r_idx,d_idx);

          while(lon>2*pi)lon-=2*pi;
          while(lon<0)lon+=2*pi;

          
	  printf("%15.15g %15.15g\n",lon*rtd,latgd*rtd);

	    if(fabs(alt)>0.05 || fabs(r-oti3.slantRange)>0.05 
	       || fabs(d2-oti3.dopplerFreq)>0.05*ses.dopplerRes){
	      exit(1);
	    }
#endif	  
	 
	  }
	  //--------------------------//
	  // move to next measurement //
	  //--------------------------//
	  
	  meas = meas_spot->GetNext();
	}
    }
    return(1);

    }

//------------------//
// Ovwm::FindSlice //
//------------------//

int
Ovwm::FindSlice(
    CoordinateSwitch*  antenna_frame_to_gc,
    Spacecraft*        spacecraft,
    double             look,
    double             azim,
    float              freq_1,
    float              freq_2,
    float              freq_tol,
    Outline*           outline,
    Vector3*           look_vector,
    EarthPosition*     centroid)
{
    //--------------------------------//
    // find peak gain for frequency 1 //
    //--------------------------------//

    double f_look[2], f_azim[2];
    float f_gain[2];

    f_look[0] = look;
    f_azim[0] = azim;
    if (! FindPeakResponseAtFreq(antenna_frame_to_gc, spacecraft, freq_1,
        freq_tol, &(f_look[0]), &(f_azim[0]), &(f_gain[0])))
    {
        fprintf(stderr,
            "FindSlice: error finding peak gain for frequency %g\n", freq_1);
        return(0);
    }

    //--------------------------------//
    // find peak gain for frequency 2 //
    //--------------------------------//

    f_look[1] = look;
    f_azim[1] = azim;
    if (! FindPeakResponseAtFreq(antenna_frame_to_gc, spacecraft, freq_2,
        freq_tol, &(f_look[1]), &(f_azim[1]), &(f_gain[1])))
    {
        fprintf(stderr,
            "FindSlice: error finding peak gain for frequency %g\n", freq_2);
        return(0);
    }

    //--------------------------//
    // find peak gain for slice //
    //--------------------------//

    float peak_gain;
    if (! FindPeakResponseForSlice(antenna_frame_to_gc, spacecraft, f_look,
        f_azim, f_gain, &peak_gain))
    {
        fprintf(stderr, "FindSlice: error finding peak gain for slice\n");
        return(0);
    }

    //------------------------------//
    // find target gain for corners //
    //------------------------------//

    float target_gain = peak_gain / pow(10.0, 0.3);

    int corner_idx = 0;
    double c_look[4], c_azim[4];

    //------------------------------//
    // find corners for frequency 1 //
    //------------------------------//

    double two_look[2];
    double two_azim[2];
    if (f_gain[0] > target_gain)
    {
        if (! FindSliceCorners(antenna_frame_to_gc, spacecraft, f_look[0],
            f_azim[0], target_gain, two_look, two_azim))
        {
            fprintf(stderr,
                "FindSlice: error finding slice corners for frequency 1\n");
            return(0);
        }

        c_look[corner_idx] = two_look[0];
        c_azim[corner_idx] = two_azim[0];
        corner_idx++;

        c_look[corner_idx] = two_look[1];
        c_azim[corner_idx] = two_azim[1];
        corner_idx++;
    }
    else
    {
        // you've got a triangle
        double tc_look, tc_azim;
        if (! find_target(f_look, f_azim, f_gain, target_gain, &tc_look,
            &tc_azim))
        {
            fprintf(stderr, "FindSlice: error finding triangle corner\n");
            return(0);
        }
        c_look[corner_idx] = tc_look;
        c_azim[corner_idx] = tc_azim;
        corner_idx++;
    }

    //------------------------------//
    // find corners for frequency 2 //
    //------------------------------//

    if (f_gain[1] > target_gain)
    {
        if (! FindSliceCorners(antenna_frame_to_gc, spacecraft, f_look[1],
            f_azim[1], target_gain, two_look, two_azim))
        {
            fprintf(stderr,
                "FindSlice: error finding slice corners for frequency 2\n");
            return(0);
        }

        c_look[corner_idx] = two_look[1];
        c_azim[corner_idx] = two_azim[1];
        corner_idx++;

        c_look[corner_idx] = two_look[0];
        c_azim[corner_idx] = two_azim[0];
        corner_idx++;
    }
    else
    {
        // you've got a triangle
        double tc_look, tc_azim;
        if (! find_target(f_look, f_azim, f_gain, target_gain, &tc_look,
            &tc_azim))
        {
            fprintf(stderr, "FindSlice: error finding triangle corner\n");
            return(0);
        }
        c_look[corner_idx] = tc_look;
        c_azim[corner_idx] = tc_azim;
        corner_idx++;
    }

    //----------------------//
    // generate the outline //
    //----------------------//

    EarthPosition sum;
    float gain;
    sum.SetPosition(0.0, 0.0, 0.0);
    for (int i = 0; i < corner_idx; i++)
    {
        Vector3 rlook_antenna;
        rlook_antenna.SphericalSet(1.0, c_look[i], c_azim[i]);
        Vector3 rlook_gc = antenna_frame_to_gc->Forward(rlook_antenna);
        EarthPosition* spot_on_earth = new EarthPosition();
            if(earth_intercept(spacecraft->orbitState.rsat, rlook_gc,
                   spot_on_earth)!=1)
          return(0);

        if (! outline->Append(spot_on_earth))
            return(0);

        if (! SpatialResponse(antenna_frame_to_gc, spacecraft, c_look[i],
            c_azim[i], &gain))
        {
            return(0);
        }
        sum += (*spot_on_earth * gain);
    }

    //------------------------//
    // determine the centroid //
    //------------------------//

    EarthPosition earth_center;
    earth_center.SetPosition(0.0, 0.0, 0.0);
    if(earth_intercept(earth_center,sum,centroid)!=1)
      return(0);

    //---------------------------//
    // determine the look vector //
    //---------------------------//

    *look_vector = *centroid - spacecraft->orbitState.rsat;

    return(1);
}

//-------------------------------//
// Ovwm::FindPeakResponseAtFreq //
//-------------------------------//
// Finds the look, azimuth, and response value for the peak spatial response at
// a given baseband frequency

#define LOOK_OFFSET         0.005
#define AZIMUTH_OFFSET      0.005
#define ANGLE_OFFSET        0.002       // start delta for golden section
#define ANGLE_TOL           0.00001     // within this of peak gain
#define MAX_PASSES          10  // Maximum  number of passes through main loop

int
Ovwm::FindPeakResponseAtFreq(
    CoordinateSwitch*  antenna_frame_to_gc,
    Spacecraft*        spacecraft,
    float              target_freq,
    float              freq_tol,
    double*            look,
    double*            azim,
    float*             response,
    int                ignore_range)
{
    float dif;
    float min_dif = 0.0;
    float best_response = 0.0;
    double best_look = 0.0;
    double best_azim = 0.0;
    Vector3 vector;
    int num_passes = 0;
    OvwmTargetInfo qti;
    do
    {
        num_passes++;

        //--------------------//
        // find the frequency //
        //--------------------//

        if (! FindFreq(antenna_frame_to_gc, spacecraft, target_freq, freq_tol,
            look, azim))
        {
            return(0);
        }

        //------------------------------------//
        // determine the iso-frequency deltas //
        //------------------------------------//

        double df_dlook, df_dazim;
        if (! FreqGradient(antenna_frame_to_gc, spacecraft, *look,
            LOOK_OFFSET, *azim, AZIMUTH_OFFSET, &df_dlook, &df_dazim))
        {
            return(0);
        }
        // rotate gradient 90 degrees for iso-frequency deltas
        double delta_look = df_dazim;
        double delta_azim = -df_dlook;

        //--------------------------//
        // locate the peak response //
        //--------------------------//

        if (! FindPeakResponseUsingDeltas(antenna_frame_to_gc, spacecraft,
            delta_look, delta_azim, ANGLE_OFFSET, ANGLE_TOL, look, azim,
            response, ignore_range))
        {
            return(0);
        }

        //--------------------------------------//
        // check if still on iso-frequency line //
        //--------------------------------------//

        vector.SphericalSet(1.0, *look, *azim);
        if (! TargetInfo(antenna_frame_to_gc, spacecraft, vector, &qti))
            return(0);

        dif = fabs(qti.basebandFreq - target_freq);

        //----------------------------------------//
        //  Keep track of best answer so far      //
        //----------------------------------------//

        if (num_passes == 1 || dif < min_dif)
        {
            min_dif=dif;
            best_look=*look;
                  best_azim=*azim;
                  best_response=*response;
        }

                //-----------------------------------------//
                // If maximum number of passes is reached  //
                // without meeting frequency tolerance use //
                // best so far and exit loop.              //
        //-----------------------------------------//
        if(num_passes >= MAX_PASSES && dif > freq_tol)
        {
            fprintf(stderr,
          "Error: FindPeakResponseAtFreq could not meet required freq_tol=%g Hz\
n",
                freq_tol);
            fprintf(stderr,"for target_freq=%g and scan_angle=%g.\n",
                target_freq, sas.antenna.txCenterAzimuthAngle*rtd);
            fprintf(stderr, "Actual frequency error was %g Hz.\n", min_dif);
            *look=best_look;
            *azim=best_azim;
            *response=best_response;
          break;
        }
    } while (dif > freq_tol);

    return(1);
}

//-----------------//
// Ovwm::FindFreq //
//-----------------//
// Finds the look and azimuth of a point with the given frequency

int
Ovwm::FindFreq(
    CoordinateSwitch*  antenna_frame_to_gc,
    Spacecraft*        spacecraft,
    float              target_freq,
    float              freq_tol,
    double*            look,
    double*            azim)
{
    Vector3 vector;
    OvwmTargetInfo qti;
    do
    {
        //-------------------------------------//
        // get the starting baseband frequency //
        //-------------------------------------//

        vector.SphericalSet(1.0, *look, *azim);
        OvwmTargetInfo qti;
        if (! TargetInfo(antenna_frame_to_gc, spacecraft, vector, &qti))
            return(0);

        //---------------//
        // check if done //
        //---------------//

        if (fabs(target_freq - qti.basebandFreq) < freq_tol)
            break;

        //------------------------//
        // calculate the gradient //
        //------------------------//

        double df_dlook, df_dazim;
        if (! FreqGradient(antenna_frame_to_gc, spacecraft, *look,
            LOOK_OFFSET, *azim, AZIMUTH_OFFSET, &df_dlook, &df_dazim))
        {
            return(0);
        }

        //---------------------------------------//
        // calculate the target look and azimuth //
        //---------------------------------------//

        double delta_look, delta_azim;
        if (df_dazim == 0.0)
            delta_azim = 0.0;
        else
        {
            delta_azim = (target_freq - qti.basebandFreq) /
                (df_dlook * df_dlook / df_dazim + df_dazim);
        }

        if (df_dlook == 0.0)
            delta_look = 0.0;
        else
        {
            delta_look = (target_freq - qti.basebandFreq) /
                (df_dazim * df_dazim / df_dlook + df_dlook);
        }

        //------------------------------//
        // jump to the look and azimuth //
        //------------------------------//

        *look += delta_look;
        *azim += delta_azim;

    } while (1);

    return(1);
}

//-------------------------//
// Ovwm::FindSliceCorners //
//-------------------------//

#define PEAK_ANGLE_OFFSET       0.00875     // about 0.50 degree
#define LOCAL_ANGLE_OFFSET      0.000875    // about 0.050 degree

int
Ovwm::FindSliceCorners(
    CoordinateSwitch*  antenna_frame_to_gc,
    Spacecraft*        spacecraft,
    double             look,
    double             azim,
    float              target_gain,
    double             corner_look[2],
    double             corner_azim[2])
{
    //------------------------------------//
    // determine the iso-frequency deltas //
    //------------------------------------//

    double df_dlook, df_dazim;
    if (! FreqGradient(antenna_frame_to_gc, spacecraft, look, LOOK_OFFSET,
        azim, AZIMUTH_OFFSET, &df_dlook, &df_dazim))
    {
        return(0);
    }
    // rotate gradient 90 degrees for iso-frequency deltas
    double delta_look = df_dazim;
    double delta_azim = -df_dlook;

    //------------------------//
    // scale to radian deltas //
    //------------------------//

    double scale = sqrt(delta_look * delta_look + delta_azim * delta_azim);
    delta_look /= scale;
    delta_azim /= scale;

    //-----------------//
    // fit a quadratic //
    //-----------------//

    double peak_look_step = PEAK_ANGLE_OFFSET * delta_look;
    double peak_azim_step = PEAK_ANGLE_OFFSET * delta_azim;

    double look_array[3];
    look_array[0] = look - peak_look_step;
    look_array[1] = look;
    look_array[2] = look + peak_look_step;

    double azim_array[3];
    azim_array[0] = azim - peak_azim_step;
    azim_array[1] = azim;
    azim_array[2] = azim + peak_azim_step;

    double s[3], c[3];
    if (! SpatialResponseQuadFit(antenna_frame_to_gc, spacecraft, look_array,
        azim_array, s, c))
    {
        fprintf(stderr, "FindSliceCorners: error fitting main quadratic\n");
        return(0);
    }

    //-------------------------//
    // check out the quadratic //
    //-------------------------//

    double qr = c[1] * c[1] - 4.0 * c[2] * (c[0] - target_gain);
    if (qr < 0.0)
    {
        corner_look[0] = look;
        corner_azim[0] = azim;
        corner_look[1] = look;
        corner_azim[1] = azim;
        return(1);
    }

    //-------------------//
    // for each point... //
    //-------------------//

    double q = sqrt(qr);
    double twoa = 2.0 * c[2];

    double local_look_step = LOCAL_ANGLE_OFFSET * delta_look;
    double local_azim_step = LOCAL_ANGLE_OFFSET * delta_azim;

    static const double coef[2] = { -1.0, 1.0 };
    for (int i = 0; i < 2; i++)
    {
        //-----------------------------------//
        // estimate using the peak quadratic //
        //-----------------------------------//

        double target_s = (-c[1] + coef[i] * q) / twoa;
        double local_look = look + target_s * delta_look;
        double local_azim = azim + target_s * delta_azim;

        //-------------------//
        // initialize points //
        //-------------------//

        double local_look_array[3];
        local_look_array[0] = local_look - local_look_step;
        local_look_array[1] = local_look;
        local_look_array[2] = local_look + local_look_step;

        double local_azim_array[3];
        local_azim_array[0] = local_azim - local_azim_step;
        local_azim_array[1] = local_azim;
        local_azim_array[2] = local_azim + local_azim_step;

        float local_gain_array[3];
        for (int j = 0; j < 3; j++)
        {
            if (! SpatialResponse(antenna_frame_to_gc, spacecraft,
                local_look_array[j], local_azim_array[j],
                &(local_gain_array[j])))
            {
                return(0);
            }
        }

        //-------------//
        // step search //
        //-------------//

        while (target_gain > local_gain_array[0] &&
                local_gain_array[0] > local_gain_array[1] &&
                local_gain_array[1] > local_gain_array[2])
        {
            for (int j = 2; j > 0; j--)
            {
                local_look_array[j] = local_look_array[j-1];
                local_azim_array[j] = local_azim_array[j-1];
                local_gain_array[j] = local_gain_array[j-1];
            }
            local_look_array[0] -= local_look_step;
            local_azim_array[0] -= local_azim_step;
            if (! SpatialResponse(antenna_frame_to_gc, spacecraft,
                local_look_array[0], local_azim_array[0],
                &(local_gain_array[0])))
            {
                return(0);
            }
        }

        while (target_gain > local_gain_array[2] &&
                local_gain_array[2] > local_gain_array[1] &&
                local_gain_array[1] > local_gain_array[0])
        {
            for (int j = 0; j < 2; j++)
            {
                local_look_array[j] = local_look_array[j+1];
                local_azim_array[j] = local_azim_array[j+1];
                local_gain_array[j] = local_gain_array[j+1];
            }
            local_look_array[2] += local_look_step;
            local_azim_array[2] += local_azim_step;
            if (! SpatialResponse(antenna_frame_to_gc, spacecraft,
                local_look_array[2], local_azim_array[2],
                &(local_gain_array[2])))
            {
                return(0);
            }
        }

        //-----------------//
        // fit a quadratic //
        //-----------------//

        double local_s[3], local_c[3];
        if (! SpatialResponseQuadFit(antenna_frame_to_gc, spacecraft,
            local_look_array, local_azim_array, local_s, local_c))
        {
            fprintf(stderr,
                "FindSliceCorners: error fitting local quadratic\n");
            return(0);
        }

        //-------------------------//
        // check out the quadratic //
        //-------------------------//

        if (local_c[2] == 0.0)
        {
            // linear fit
            target_s = (target_gain - local_c[0]) / local_c[1];
        }
        else
        {
            // quadratic fit

            double local_qr = local_c[1] * local_c[1] - 4.0 * local_c[2] *
                (local_c[0] - target_gain);
            if (local_qr < 0.0)
            {
                corner_look[i] = local_look;
                corner_azim[i] = local_azim;
                continue;
            }

            double local_q = sqrt(local_qr);
            double local_twoa = 2.0 * local_c[2];
            double target_s_1 = (-local_c[1] + local_q) / local_twoa;
            double target_s_2 = (-local_c[1] - local_q) / local_twoa;
            double abs_1 = fabs(target_s_1);
            double abs_2 = fabs(target_s_2);
            if (abs_1 < abs_2)
                target_s = target_s_1;
            else
                target_s = target_s_2;
        }

        corner_look[i] = local_look + target_s * delta_look;
        corner_azim[i] = local_azim + target_s * delta_azim;
    }

    return(1);
}

//---------------------------------//
// Ovwm::FindPeakResponseForSlice //
//---------------------------------//
// Finds the peak response in a slice

int
Ovwm::FindPeakResponseForSlice(
    CoordinateSwitch*  antenna_frame_to_gc,
    Spacecraft*        spacecraft,
    double             look[2],
    double             azim[2],
    float              response[2],
    float*             peak_response,
    int                ignore_range)
{
    double mid_look = (look[0] + look[1]) / 2.0;
    double mid_azim = (azim[0] + azim[1]) / 2.0;

    float mid_response;
    if (! SpatialResponse(antenna_frame_to_gc, spacecraft, mid_look,
        mid_azim, &mid_response, ignore_range))
    {
        return(0);
    }

    if (response[0] >= mid_response && response[0] >= response[1])
    {
        *peak_response = response[0];
        return(1);
    }
    else if (response[1] >= mid_response && response[1] >= response[0])
    {
        *peak_response = response[1];
        return(1);
    }
    else if (mid_response >= response[0] && mid_response >= response[1])
    {
        double look_array[3], azim_array[3];
        look_array[0] = look[0];
        look_array[1] = mid_look;
        look_array[2] = look[1];
        azim_array[0] = azim[0];
        azim_array[1] = mid_azim;
        azim_array[2] = azim[1];

        double s[3], c[3];
        if (! SpatialResponseQuadFit(antenna_frame_to_gc, spacecraft,
            look_array, azim_array, s, c))
        {
            return(0);
        }
        double peak_s, peak_r;
        if (! get_quad_peak(c, &peak_s, &peak_r))
            return(0);

        *peak_response = (float)peak_r;

        return(1);
    }
    return(0);
}

//------------------------//
// Ovwm::SpatialResponse //
//------------------------//

int
Ovwm::SpatialResponse(
    CoordinateSwitch*  antenna_frame_to_gc,
    Spacecraft*        spacecraft,
    double             look,
    double             azim,
    double*            response,
    int                ignore_range)
{
    Vector3 vector;
    vector.SphericalSet(1.0, look, azim);
    ScatTargetInfo sti;
    if (! sti.GetScatTargetInfo(antenna_frame_to_gc,
        spacecraft->orbitState.rsat, vector))
    {
        return(0);
    }
    Beam* beam = GetCurrentBeam();

    //-------------------------------------------------//
    // Calculate Two-way Gain Only if ignore_range = 1 //
    //-------------------------------------------------//

    int retval;
    if(ignore_range)
    {
        retval = beam->GetPowerGainProduct(look, azim, sti.roundTripTime,
            sas.antenna.spinRate, response);
    }
    else
    {
        retval = beam->GetSpatialResponse(look, azim, sti.roundTripTime,
            sas.antenna.spinRate, response);
    }
    return(retval);
}

//------------------------//
// Ovwm::SpatialResponse //
//------------------------//
// same as above but returns float

int
Ovwm::SpatialResponse(
    CoordinateSwitch*  antenna_frame_to_gc,
    Spacecraft*        spacecraft,
    double             look,
    double             azim,
    float*             response,
    int                ignore_range)
{
  int ret_val;
  double val;
  ret_val = SpatialResponse(antenna_frame_to_gc, spacecraft, look,azim, &val,
      ignore_range);
  *response = (float)val;
  return(ret_val);
}

//---------------------//
// Ovwm::FreqGradient //
//---------------------//
// Calculate the baseband frequency gradient

int
Ovwm::FreqGradient(
    CoordinateSwitch*  antenna_frame_to_gc,
    Spacecraft*        spacecraft,
    double             look,
    double             look_offset,
    double             azim,
    double             azim_offset,
    double*            df_dlook,
    double*            df_dazim)
{
    OvwmTargetInfo qti_1, qti_2;

    //------------------------//
    // calculate look vectors //
    //------------------------//

    double look_1 = look - look_offset;
    double look_2 = look + look_offset;
    Vector3 vector_look_1, vector_look_2;
    vector_look_1.SphericalSet(1.0, look_1, azim);
    vector_look_2.SphericalSet(1.0, look_2, azim);

    //------------------------------//
    // calculate baseband frequency //
    //------------------------------//

    if (! TargetInfo(antenna_frame_to_gc, spacecraft, vector_look_1, &qti_1) ||
        ! TargetInfo(antenna_frame_to_gc, spacecraft, vector_look_2, &qti_2))
    {
        return(0);
    }

    //-------------------------//
    // calculate look gradient //
    //-------------------------//

    *df_dlook = (qti_2.basebandFreq - qti_1.basebandFreq) / (look_2 - look_1);

    //---------------------------//
    // calculate azimuth vectors //
    //---------------------------//

    double azim_1 = azim - azim_offset;
    double azim_2 = azim + azim_offset;
    Vector3 vector_azim_1, vector_azim_2;
    vector_azim_1.SphericalSet(1.0, look, azim_1);
    vector_azim_2.SphericalSet(1.0, look, azim_2);

    //------------------------------//
    // calculate baseband frequency //
    //------------------------------//

    if (! TargetInfo(antenna_frame_to_gc, spacecraft, vector_azim_1, &qti_1) ||
        ! TargetInfo(antenna_frame_to_gc, spacecraft, vector_azim_2, &qti_2))
    {
        return(0);
    }

    //----------------------------//
    // calculate azimuth gradient //
    //----------------------------//

    *df_dazim = (qti_2.basebandFreq - qti_1.basebandFreq) / (azim_2 - azim_1);

    return(1);
}

//------------------------------------//
// Ovwm::FindPeakResponseUsingDeltas //
//------------------------------------//

int
Ovwm::FindPeakResponseUsingDeltas(
    CoordinateSwitch*  antenna_frame_to_gc,
    Spacecraft*        spacecraft,
    double             delta_look,
    double             delta_azim,
    double             angle_offset,
    double             angle_tol,
    double*            look,
    double*            azim,
    float*             response,
    int                ignore_range)
{
    //------------------------//
    // scale to radian deltas //
    //------------------------//

    double scale = sqrt(delta_look * delta_look + delta_azim * delta_azim);
    delta_look /= scale;
    delta_azim /= scale;

    //--------------------------------//
    // golden section search for peak //
    //--------------------------------//

    double ax = -angle_offset;
    double cx = angle_offset;
    double bx = ax + (cx - ax) * golden_r;

    //--------------------//
    // widen if necessary //
    //--------------------//

    float again, bgain, cgain;

    if (! SpatialResponse(antenna_frame_to_gc, spacecraft,
            *look + ax * delta_look, *azim + ax * delta_azim, &again,
            ignore_range ) ||
        ! SpatialResponse(antenna_frame_to_gc, spacecraft,
            *look + bx * delta_look, *azim + bx * delta_azim, &bgain,
            ignore_range ) ||
        ! SpatialResponse(antenna_frame_to_gc, spacecraft,
            *look + cx * delta_look, *azim + cx * delta_azim, &cgain,
            ignore_range))
    {
        return(0);
    }

    do
    {
        if (again > bgain)
        {
            ax -= angle_offset;
            if (! SpatialResponse(antenna_frame_to_gc, spacecraft,
                *look + ax * delta_look, *azim + ax * delta_azim, &again,
                ignore_range))
            {
                return(0);
            }
            continue;
        }

        if (cgain > bgain)
        {
            cx += angle_offset;
            if (! SpatialResponse(antenna_frame_to_gc, spacecraft,
                *look + cx * delta_look, *azim + cx * delta_azim, &cgain,
                ignore_range))
            {
                return(0);
            }
            continue;
        }
                if(again==bgain && bgain==cgain) return(0);
        break;
    } while (1);

    //---------------//
    // do the search //
    //---------------//

    double x0, x1, x2, x3;
    x0 = ax;
    x3 = cx;
    if (cx - bx > bx - ax)
    {
        x1 = bx;
        x2 = bx + golden_c * (cx - bx);
    }
    else
    {
        x2 = bx;
        x1 = bx - golden_c * (bx - ax);
    }

    float f1, f2;
    if (! SpatialResponse(antenna_frame_to_gc, spacecraft,
            *look + x1 * delta_look, *azim + x1 * delta_azim, &f1,
            ignore_range) ||
        ! SpatialResponse(antenna_frame_to_gc, spacecraft,
            *look + x2 * delta_look, *azim + x2 * delta_azim, &f2,
            ignore_range))
    {
        return(0);
    }

    while (x3 - x0 > angle_tol)
    {
        if (f2 > f1)
        {
            x0 = x1;
            x1 = x2;
            x2 = x2 + golden_c * (x3 - x2);
            f1 = f2;
            if (! SpatialResponse(antenna_frame_to_gc, spacecraft,
                *look + x2 * delta_look, *azim + x2 * delta_azim, &f2,
                ignore_range))
            {
                return(0);
            }
        }
        else
        {
            x3 = x2;
            x2 = x1;
            x1 = x1 - golden_c * (x1 - x0);
            f2 = f1;
            if (! SpatialResponse(antenna_frame_to_gc, spacecraft,
                *look + x1 * delta_look, *azim + x1 * delta_azim, &f1,
                ignore_range))
            {
                return(0);
            }
        }
    }

    if (f1 > f2)
    {
        *look += x1 * delta_look;
        *azim += x1 * delta_azim;
        *response = f1;
    }
    else
    {
        *look += x2 * delta_look;
        *azim += x2 * delta_azim;
        *response = f2;
    }

    return(1);
}

//-------------------//
// Ovwm::TargetInfo //
//-------------------//
// Compute some useful numbers for the target on the earth's surface
// intercepted by a particular direction in the antenna frame.
// The vector is a directional vector specified in the antenna frame.

int
Ovwm::TargetInfo(
    CoordinateSwitch*  antenna_frame_to_gc,
    Spacecraft*        spacecraft,
    Vector3            vector,
    OvwmTargetInfo*   qti)
{
    // first, get the generic scatterometer target info
    qti->GetScatTargetInfo(antenna_frame_to_gc, spacecraft->orbitState.rsat,
        vector);

    // Compute doppler shift for the earth intercept point.
    Vector3 vspot(-w_earth * qti->rTarget.Get(1),
        w_earth * qti->rTarget.Get(0), 0);
    Vector3 vrel = spacecraft->orbitState.vsat - vspot;
    double lambda = speed_light_kps / ses.txFrequency;
    qti->dopplerFreq = 2.0 * (vrel % qti->gcLook) / lambda;

    // compute baseband frequency
    /******
    double pulse_width = ses.txPulseWidth;
    double gate_width = GetRxGateWidth();
    double mu = ses.chirpRate;

    double chirp_start = mu *
        ( (pulse_width + RX_GATE_DELAY_CMD_RESOLUTION) / 2.0 + T_GRID);
    double dechirp_start = mu * (gate_width / 2.0);

    qti->basebandFreq = -ses.txDoppler - qti->dopplerFreq -
        chirp_start + dechirp_start + F_PROC -
        mu * (qti->roundTripTime - ses.rxGateDelay);
    *****/

    
    return(1);
}



//---------------------------//
// Ovwm::SphericalTargetInfo //
//---------------------------//
// Compute some useful numbers for the target on the earth's surface
// intercepted by a particular direction in the antenna frame.
// The vector is a directional vector specified in the antenna frame.
// Uses a known range to speed things up

int
Ovwm::SphericalTargetInfo(
    CoordinateSwitch*  antenna_frame_to_gc,
    Spacecraft*        spacecraft,
    Vector3            vector,
    OvwmTargetInfo*    qti,
    double             range)
{

    qti->gcLook=antenna_frame_to_gc->Forward(vector);
    qti->slantRange=range;
    qti->rTarget = spacecraft->orbitState.rsat + qti->gcLook*qti->slantRange;
    qti->roundTripTime = 2.0 * qti->slantRange / speed_light_kps;
    // Compute doppler shift for the earth intercept point.
    Vector3 vspot(-w_earth * qti->rTarget.Get(1),
        w_earth * qti->rTarget.Get(0), 0);
    Vector3 vrel = spacecraft->orbitState.vsat - vspot;
    double lambda = speed_light_kps / ses.txFrequency;
    qti->dopplerFreq = 2.0 * (vrel % qti->gcLook) / lambda;

    return(1);
}

int  
Ovwm:: FastDopplerRangeToTbfLook(double doppler, double range, 
	     double local_radius,Vector3 local_center,
	     Spacecraft* spacecraft,double u1[3],
	     double u2[3]){
  //------------------------------------------------
  // Input parameters
  // doppler = desired doppler in Hz
  // range = desired range in km
  // local_radius = approximate radius of target body for local region
  //--------STUFF computed from spacecraft
  // p = s/c position in TBF (km)
  // v = s/c velocity in TBF (km/s) (need to translate from non-rotating frame)
  //--------STUFF obtained from Ovwm object 
  // lambda  = center wavelength of transmitted signal (km)
  //----------------------------------------------
  // Output parameters
  // u1,u2 pair of direction vectors in TBF pointing toward
  // desired doppler,range pair on surface of target body
  double v[3], p[3];
  Vector3 vscpos(-w_earth * spacecraft->orbitState.rsat.Get(1),
        w_earth * spacecraft->orbitState.rsat.Get(0), 0);
  // convert from non-rotating frame
  v[0]=spacecraft->orbitState.vsat.Get(0)-vscpos.Get(0);
  v[1]=spacecraft->orbitState.vsat.Get(1)-vscpos.Get(1);
  v[2]=spacecraft->orbitState.vsat.Get(2)-vscpos.Get(2);

  p[0]=spacecraft->orbitState.rsat.Get(0)-local_center.Get(0);
  p[1]=spacecraft->orbitState.rsat.Get(1)-local_center.Get(1);
  p[2]=spacecraft->orbitState.rsat.Get(2)-local_center.Get(2);
  

  double spd=sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
  double pos_mag=sqrt(p[0]*p[0]+p[1]*p[1]+p[2]*p[2]);  
  double pos_in_rad=pos_mag/local_radius;

  // scale pos and velocity vectors to unity
  double p1,p2,p3,v1,v2,v3;
  p1=p[0]/pos_mag;
  p2=p[1]/pos_mag;
  p3=p[2]/pos_mag;
  v1=v[0]/spd;
  v2=v[1]/spd;
  v3=v[2]/spd;

  double range_in_radius=range/local_radius;
  double A =  (1 - pos_in_rad*pos_in_rad 
		 - range_in_radius*range_in_radius)
    /(2.0 * range_in_radius * pos_in_rad);

  double lambda = speed_light_kps / ses.txFrequency; 
  double B =  lambda * doppler/(2 * spd);


  double C = (v3 * A - p3*B)/(v3*p1 - p3*v1);
  double D = -(v3*p2 - p3*v2)/(v3*p1 - p3*v1);
  double E = (A - p1 *C)/p3;
  double F = -(p2 + p1*D)/p3;
  
  double sol = (C*D + E*F)*(C*D + E*F) - (D*D + F*F + 1.0)*(C*C + E*E-1.0);

  if (sol > 0){
     // get 1 solution
     u1[1] = ( -(C*D + E*F) -sqrt( sol))/(D*D + F*F + 1.0);
     u1[0] = C + D*u1[1];
     u1[2] = E + F*u1[1];

     // get the other solution
     u2[1] = ( -(C*D + E*F) +sqrt( sol))/(D*D + F*F + 1.0);
     u2[0] = C + D*u2[1];
     u2[2] = E + F*u2[1];
     return(1);
  }
  return(0);
}

//-------------------------------//
// Ovwm::SpatialResponseQuadFit //
//-------------------------------//
// Generates coefficients for a quadratic fit of a beam pattern

int
Ovwm::SpatialResponseQuadFit(
    CoordinateSwitch*  antenna_frame_to_gc,
    Spacecraft*        spacecraft,
    double             look[3],
    double             azim[3],
    double             s[3],
    double             c[3])
{
    //-------------------------------//
    // calculate projected distances //
    //-------------------------------//

    s[1] = 0.0;
    double dl, da;
    dl = (look[1] - look[0]);
    da = (azim[1] - azim[0]);
    s[0] = -sqrt(dl*dl + da*da);
    dl = (look[2] - look[1]);
    da = (azim[2] - azim[1]);
    s[2] = sqrt(dl*dl + da*da);

    //-------------------//
    // for each point... //
    //-------------------//

    OvwmTargetInfo qti;
    float gain[3];
    double dgain[3];
    for (int i = 0; i < 3; i++)
    {
        if (! SpatialResponse(antenna_frame_to_gc, spacecraft,
            look[i], azim[i], &(gain[i])))
        {
            return(0);
        }
        dgain[i] = gain[i];     // transfer to double for quad fit
    }

    //-----------------//
    // fit a quadratic //
    //-----------------//

    if (! polcoe(s, dgain, 2, c))
        return(0);

    return(1);
}



//-----------------//
// Ovwm::IdealRtt //
//-----------------//
// Calculate the ideal round trip time.  The attitude is set to 0,0,0.
// Used for the RGC or for calculating the commanded round trip time.
// If use_flags is nonzero then the round trip time BYU reference vector
// or the Spectral peak is returned if the appropriate flag in ovwm.cds
// is set.
// Otherwise the Spatial peak is returned regardless of the values of the
// flags.

double
Ovwm::IdealRtt(
    Spacecraft*  spacecraft,
    int          use_flags)
{
    //------------------------------//
    // zero the spacecraft attitude //
    //------------------------------//

    OrbitState* sc_orbit_state = &(spacecraft->orbitState);
    Attitude zero_rpy;
    zero_rpy.Set(0.0, 0.0, 0.0, 1, 2, 3);
    CoordinateSwitch zero_rpy_antenna_frame_to_gc =
        AntennaFrameToGC(sc_orbit_state, &zero_rpy, &(sas.antenna),
        sas.antenna.txCenterAzimuthAngle);
    Spacecraft sp_zero_att;
    sp_zero_att.orbitState = *sc_orbit_state;
    sp_zero_att.attitude = zero_rpy;

    //-------------------------------------------//
    // find the current beam's two-way peak gain //
    //-------------------------------------------//

    Beam* beam = GetCurrentBeam();
    double azimuth_rate = sas.antenna.spinRate;
    double look, azim;
    if (! GetPeakSpatialResponse2(&zero_rpy_antenna_frame_to_gc,
            &sp_zero_att, beam, azimuth_rate, &look, &azim))
      {
	exit(1);
      }
 

    //-------------------------------//
    // calculate the round trip time //
    //-------------------------------//

    Vector3 vector;
    vector.SphericalSet(1.0, look, azim);
    Vector3 ulook_gc = zero_rpy_antenna_frame_to_gc.Forward(vector);
    EarthPosition r_target;
    if(earth_intercept(sc_orbit_state->rsat, ulook_gc,&r_target)!=1)
      exit(1);
    double slant_range = (sc_orbit_state->rsat - r_target).Magnitude();
    double round_trip_time = 2.0 * slant_range / speed_light_kps;

    return(round_trip_time);
}

//------------------------------//
// Ovwm::IdealCommandedDoppler //
//------------------------------//
// Estimate the ideal commanded doppler frequency.
// This method has the option of using the spacecraft attitude. If
// you want to configure the instrument like it will be used postlaunch,
// make a copy of the spacecraft and send it in with the desired biases
// as the attitude, and, of course, set the use_attitude flag. If you are
// doing something else, don't bother.
// The default is to not use the attitude, but to zero it out.

#define DOPPLER_ACCURACY    1.0     // 1 Hz

int
Ovwm::IdealCommandedDoppler(
    Spacecraft*       spacecraft,
    OvwmTargetInfo*  qti_out,
    int               use_attitude)
{
    OrbitState* sc_orbit_state = &(spacecraft->orbitState);

    Spacecraft* use_spacecraft = NULL;
    CoordinateSwitch antenna_frame_to_gc;

    // if zeroing the attitude, this will get used
    Spacecraft sp_zero_att;

    if (use_attitude)
    {
        //--------------------------------------------------//
        // this section is for REALLY tracking the attitude //
        //--------------------------------------------------//

        Attitude* sc_attitude = &(spacecraft->attitude);
        antenna_frame_to_gc = AntennaFrameToGC(sc_orbit_state, sc_attitude,
            &(sas.antenna), sas.antenna.txCenterAzimuthAngle);

        use_spacecraft = spacecraft;
    }
    else
    {
        //---------------------------------------------------------//
        // this section is for zeroing out the spacecraft attitude //
        //---------------------------------------------------------//

        Attitude zero_rpy;
        zero_rpy.Set(0.0, 0.0, 0.0, 2, 1, 3);
        antenna_frame_to_gc = AntennaFrameToGC(sc_orbit_state, &zero_rpy,
            &(sas.antenna), sas.antenna.txCenterAzimuthAngle);
        sp_zero_att.orbitState = *sc_orbit_state;
        sp_zero_att.attitude = zero_rpy;

        use_spacecraft = &sp_zero_att;
    }

    //-------------------------------------------//
    // find the current beam's two-way peak gain //
    //-------------------------------------------//

    Beam* beam = GetCurrentBeam();
    double azimuth_rate = sas.antenna.spinRate;
    double look, azim;
    if (! GetPeakSpatialResponse2(&antenna_frame_to_gc, use_spacecraft,
				  beam, azimuth_rate, &look, &azim))
      {
	return(0);
      }


    //-----------------------------//
    // calculate commanded Doppler //
    //-----------------------------//

    Vector3 vector;
    vector.SphericalSet(1.0, look, azim);
    OvwmTargetInfo qti;
    ses.CmdTxDopplerEu(0.0);
    do
    {
        TargetInfo(&antenna_frame_to_gc, use_spacecraft, vector, &qti);
        float freq = ses.txDoppler + qti.basebandFreq;
        ses.CmdTxDopplerEu(freq);
    } while (fabs(qti.basebandFreq) > DOPPLER_ACCURACY);

    if (qti_out)
        *qti_out = qti;

    return(1);
}

//============//
// OvwmEvent //
//============//

OvwmEvent::OvwmEvent()
:   time(0.0), eventId(NONE), beamIdx(0)
{
    return;
}

OvwmEvent::~OvwmEvent()
{
    return;
}

//==================//
// Helper Functions //
//==================//

// This function probably belongs somewhere else, but it is so convenient
// to put it here.

//----------------------//
// SetDelayAndFrequency //
//----------------------//
// the orbit step must be set prior to calling this function
// estimate_encoder_method = 0 : use stored delay
// estimate_encoder_method = 1 : estimate delay (in case a pulse was missed)

int
SetDelayAndFrequency(
    Spacecraft*       spacecraft,
    Ovwm*            ovwm,
    OvwmTargetInfo*  qti,
    int               estimate_encoder_method)
{
    //------------------------------------------------------//
    // calculate the encoder value to use for the algorithm //
    //------------------------------------------------------//

    // remember the raw encoder for putting it in telemetry
    ovwm->cds.rawEncoder = ovwm->cds.heldEncoder;

    unsigned short ideal_encoder;
    unsigned char rx_gate_width_dn;
    switch (estimate_encoder_method)
    {
    case 0:    // used stored delay
        ideal_encoder = ovwm->cds.EstimateIdealEncoder();
        break;
    case 1:    // use estimated delay
        rx_gate_width_dn= ovwm->GetRxGateWidthDn();
        ideal_encoder = ovwm->cds.EstimateIdealEncoder(1,
            ovwm->cds.orbitStep, ovwm->cds.heldEncoder,
            rx_gate_width_dn, ovwm->cds.txPulseWidthDn,
	    ovwm->sas.GetSpinRate());
        break;
    default:
        return(0);
        break;
    }

    //---------------------------------------------//
    // sample the encoder for the next calculation //
    //---------------------------------------------//
    // this needs to be done before rotating the antenna

    ovwm->cds.heldEncoder = ovwm->sas.GetEncoder();

    //-----------------------------//
    // calculate the rx gate delay //
    //-----------------------------//

    // these defaults will produce no delay quantization correction
    // if ideal range tracking is used
    ovwm->cds.rxGateDelayDn = 0;
    float rx_gate_delay_fdn = 0.0;

    if (ovwm->cds.useRgc)
    {
        // tracking algorithm
        RangeTracker* range_tracker = ovwm->GetRangeTracker();
        range_tracker->GetRxGateDelay(ovwm->cds.orbitStep, ideal_encoder,
            ovwm->GetRxGateWidthDn(), ovwm->cds.txPulseWidthDn,
            &(ovwm->cds.rxGateDelayDn), &rx_gate_delay_fdn);
        ovwm->ses.CmdRxGateDelayDn(ovwm->cds.rxGateDelayDn);
    }
    else
    {
        // ideal delay
        float rtt = ovwm->IdealRtt(spacecraft, 1);
        float rx_gate_width=ovwm->GetRxGateWidth();
        float delay = rtt +
            (ovwm->ses.txPulseWidth - rx_gate_width) / 2.0;
        ovwm->ses.CmdRxGateDelayEu(delay);
    }

    //----------------------------//
    // calculate the tx frequency //
    //----------------------------//

    if (ovwm->cds.useDtc)
    {
        // tracking algorithm
        DopplerTracker* doppler_tracker = ovwm->GetDopplerTracker();
//        printf("rx_gate_delay_DN,fDN = %d %g\n",ovwm->cds.rxGateDelayDn,
//               rx_gate_delay_fdn);
//        printf("orbitStep = %d, ideal_encoder = %d\n",ovwm->cds.orbitStep,
//               ideal_encoder);
        doppler_tracker->GetCommandedDoppler(ovwm->cds.orbitStep,
            ideal_encoder, ovwm->cds.rxGateDelayDn, rx_gate_delay_fdn,
            &(ovwm->cds.txDopplerDn));
//        printf("doppler_dn = %d\n",ovwm->cds.txDopplerDn);
        ovwm->ses.CmdTxDopplerDn(ovwm->cds.txDopplerDn);
    }
    else
    {
        // ideal frequency
        ovwm->IdealCommandedDoppler(spacecraft, qti);
    }

    return(1);
}

//-------------------------------//
// SetOrbitStepDelayAndFrequency //
//-------------------------------//

int
SetOrbitStepDelayAndFrequency(
    Spacecraft*  spacecraft,
    Ovwm*       ovwm)
{
    //---------------------------------//
    // calculate orbit step, if needed //
    //---------------------------------//

    if (ovwm->cds.useRgc || ovwm->cds.useDtc)
        ovwm->cds.SetAndGetOrbitStep();

    //-------------------------------//
    // calculate delay and frequency //
    //-------------------------------//

    return(SetDelayAndFrequency(spacecraft, ovwm));
}


