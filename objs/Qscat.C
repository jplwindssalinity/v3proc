//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_qscat_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include "Qscat.h"
#include "Constants.h"
#include "InstrumentGeom.h"
#include "GenericGeom.h"
#include "Interpolate.h"
#include "Misc.h"
#include "BYUXTable.h"
#include "AccurateGeom.h"

//=============//
// SesBeamInfo //
//=============//

SesBeamInfo::SesBeamInfo()
:    rxGateWidth(0.0)
{
    return;
}

SesBeamInfo::~SesBeamInfo()
{
    return;
}

//==========//
// QscatSes //
//==========//

QscatSes::QscatSes()
:   txPulseWidth(0.0), txDoppler(0.0), txFrequency(0.0), rxGateDelay(0.0),
    baseTxFrequency(0.0), pri(0.0), transmitPower(0.0), rxGainEcho(0.0),
    rxGainNoise(0.0), chirpRate(0.0), fftBinBandwidth(0.0),
    scienceSliceBandwidth(0.0),  scienceSlicesPerSpot(0),
    guardSliceBandwidth(0.0), guardSlicesPerSide(0), noiseBandwidth(0.0)
{
    return;
}

QscatSes::~QscatSes()
{
    return;
}

//------------------------------//
// QscatSes::GetCurrentBeamInfo //
//------------------------------//

SesBeamInfo*
QscatSes::GetCurrentBeamInfo(
    int  current_beam_idx)
{
    return(&(beamInfo[current_beam_idx]));
}

//-----------------------------------//
// QscatSes::GetTotalSignalBandwidth //
//-----------------------------------//

float
QscatSes::GetTotalSignalBandwidth()
{
    float bw = (float)scienceSlicesPerSpot * scienceSliceBandwidth +
        (float)(guardSlicesPerSide * 2) * guardSliceBandwidth;
    return(bw);
}

//------------------------------//
// QscatSes::GetTotalSliceCount //
//------------------------------//

int
QscatSes::GetTotalSliceCount()
{
    int slice_count = scienceSlicesPerSpot + 2 * guardSlicesPerSide;
    return(slice_count);
}

//--------------------------//
// QscatSes::GetSliceFreqBw //
//--------------------------//

int
QscatSes::GetSliceFreqBw(
    int     slice_idx,
    float*  f1,
    float*  bw)
{
    //----------------------------//
    // determine slice zero index //
    //----------------------------//

    float center_idx = ((float)GetTotalSliceCount() - 1.0) / 2.0;
    float zidx = (float)slice_idx - center_idx;     // zeroed index

    //-----------------------------------------------------------//
    // determine science index, guard index, and slice bandwidth //
    //-----------------------------------------------------------//

    float half_sci = (float)scienceSlicesPerSpot / 2.0;
    if (zidx < -half_sci)
    {
        // lower guard slice
        *f1 = -half_sci * scienceSliceBandwidth +
            (zidx + half_sci - 0.5) * guardSliceBandwidth;
        *bw = guardSliceBandwidth;
    }
    else if (zidx > half_sci)
    {
        // in upper guard slice
        *f1 = half_sci * scienceSliceBandwidth +
            (zidx - half_sci - 0.5) * guardSliceBandwidth;
        *bw = guardSliceBandwidth;
    }
    else
    {
        // science slices
        *f1 = (zidx - 0.5) * scienceSliceBandwidth;
        *bw = scienceSliceBandwidth;
    }

    //----------------------------------------------------------------//
    // Subtract one half FFT Bin Bandwidth from *f1                   //
    // to take account for the fact that the O Hz FFT bin             //
    // ranges from -0.5 fftBinBandwith to +0.5 fftBinBandwidth        //
    // and falls completely within slice +1                           //
    //----------------------------------------------------------------//

    *f1-=0.5*fftBinBandwidth;
    return(1);
}

//-----------------------------//
// QscatSes::CmdTxPulseWidthDn //
//-----------------------------//

int
QscatSes::CmdTxPulseWidthDn(
    unsigned char  tx_pulse_width_dn)
{
    txPulseWidth = (float)tx_pulse_width_dn * TX_PULSE_WIDTH_CMD_RESOLUTION;
    return(1);
}

//--------------------//
// QscatSes::CmdPriDn //
//--------------------//

int
QscatSes::CmdPriDn(
    unsigned char  pri_dn)
{
    pri = (float)pri_dn * PRI_CMD_RESOLUTION;
    return(1);
}

//----------------------------//
// QscatSes::CmdRxGateWidthDn //
//----------------------------//

int
QscatSes::CmdRxGateWidthDn(
    int            beam_idx,
    unsigned char  rx_gate_width_dn)
{
    beamInfo[beam_idx].rxGateWidth = (float)rx_gate_width_dn *
        RX_GATE_WIDTH_CMD_RESOLUTION;
    return(1);
}

//----------------------------//
// QscatSes::CmdRxGateWidthEu //
//----------------------------//

int
QscatSes::CmdRxGateWidthEu(
    int    beam_idx,
    float  rx_gate_width_eu)
{
    beamInfo[beam_idx].rxGateWidth = rx_gate_width_eu;
    return(1);
}

//----------------------------//
// QscatSes::CmdRxGateDelayDn //
//----------------------------//

int
QscatSes::CmdRxGateDelayDn(
    unsigned char  rx_gate_delay_dn)
{
    rxGateDelay = (float)rx_gate_delay_dn * RX_GATE_DELAY_CMD_RESOLUTION;
    return(1);
}

//-----------------------------//
// QscatSes::CmdRxGateDelayFdn //
//-----------------------------//

int
QscatSes::CmdRxGateDelayFdn(
    float  rx_gate_delay_fdn)
{
    rxGateDelay = rx_gate_delay_fdn * RX_GATE_DELAY_CMD_RESOLUTION;
    return(1);
}

//----------------------------//
// QscatSes::CmdRxGateDelayEu //
//----------------------------//

int
QscatSes::CmdRxGateDelayEu(
    float  rx_gate_delay_eu)
{
    rxGateDelay = rx_gate_delay_eu;
    return(1);
}

//--------------------------//
// QscatSes::CmdTxDopplerDn //
//--------------------------//

int
QscatSes::CmdTxDopplerDn(
    short  tx_doppler_dn)
{
    txDoppler = (float)tx_doppler_dn * TX_FREQUENCY_CMD_RESOLUTION;
    txFrequency = baseTxFrequency + txDoppler;
    return(1);
}

//--------------------------//
// QscatSes::CmdTxDopplerEu //
//--------------------------//

int
QscatSes::CmdTxDopplerEu(
    float  tx_doppler_eu)
{
    txDoppler = tx_doppler_eu;
    txFrequency = baseTxFrequency + txDoppler;
    return(1);
}

//--------------------//
// QscatSes::tempToDn //
//--------------------//

unsigned char
QscatSes::tempToDn(
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
      "Error: invalid temperature = %g passed to QscatSes::tempToDn\n",temp);
    exit(1);
  }
  double x1 = (-b + sqrt(dd))/(2*a);
  double x2 = (-b - sqrt(dd))/(2*a);
  unsigned char x;
  if (x1 > x2) x = (unsigned char)x1; else x = (unsigned char)x2;
  return(x);
}

//==========//
// QscatSas //
//==========//

QscatSas::QscatSas()
:    encoderElectronics(ENCODER_A)
{
    return;
}

QscatSas::~QscatSas()
{
    return;
}

//---------------------------------//
// QscatSas::SetAzimuthWithEncoder //
//---------------------------------//

#define ENCODER_MASK         0x8000

int
QscatSas::SetAzimuthWithEncoder(
    unsigned short  encoder)
{
    //------------------------------------------//
    // determine the encoder and encoder offset //
    //------------------------------------------//

    double encoder_offset;
    if (encoder & ENCODER_MASK)
    {
        // encoder B
        encoder_offset = SAS_ENCODER_B_OFFSET * dtr;
    }
    else
    {
        // encoder A
        encoder_offset = SAS_ENCODER_A_OFFSET * dtr;
    }

    // mask out the encoder bit
    unsigned short use_encoder = encoder & 0x7fff;

    //---------------------//
    // calculate the angle //
    //---------------------//

    double angle = two_pi * (double)use_encoder / (double)ENCODER_N +
        encoder_offset;

    antenna.SetEncoderAzimuthAngle(angle);

    return(1);
}

//----------------------//
// QscatSas::GetEncoder //
//----------------------//

#define ENCODER_A_BIT  0x0000
#define ENCODER_B_BIT  0x8000

unsigned short
QscatSas::GetEncoder()
{
    unsigned short encoder = AzimuthToEncoder(antenna.encoderAzimuthAngle);
    return(encoder);
}

//----------------------------//
// QscatSas::AzimuthToEncoder //
//----------------------------//

unsigned short
QscatSas::AzimuthToEncoder(
    double  azimuth)
{
    //------------------------------------------//
    // determine the encoder and encoder offset //
    //------------------------------------------//

    unsigned short encoder_bit;
    double encoder_offset;
    if (encoderElectronics == ENCODER_B)
    {
        // encoder B
        encoder_bit = ENCODER_B_BIT;
        encoder_offset = SAS_ENCODER_B_OFFSET * dtr;
    }
    else
    {
        // encoder A
        encoder_bit = ENCODER_A_BIT;
        encoder_offset = SAS_ENCODER_A_OFFSET * dtr;
    }

    //-----------------------//
    // calculate the encoder //
    //-----------------------//

    double angle = azimuth - encoder_offset;
    if (angle < 0.0)
        angle += two_pi;

    unsigned short encoder = (unsigned short)((double)ENCODER_N * angle /
        two_pi + 0.5);

    encoder %= ENCODER_N;

    // mask in the encoder bit
    encoder |= encoder_bit;

    return(encoder);
}

//-----------------------//
// QscatSas::CmdSpinRate //
//-----------------------//

int
QscatSas::CmdSpinRate(
    SpinRateE  spin_rate)
{
    switch (spin_rate)
    {
        case LOW_SPIN_RATE:
            antenna.spinRate = SAS_LOW_SPIN_RATE * rpm_to_radps;
            return(1);
            break;
        case HIGH_SPIN_RATE:
            antenna.spinRate = SAS_HIGH_SPIN_RATE * rpm_to_radps;
            return(1);
            break;
        default:
            return(0);
    }
    return(0);
}

//=============//
// CdsBeamInfo //
//=============//

CdsBeamInfo::CdsBeamInfo()
:   rxGateWidthDn(0)
{
    return;
}

CdsBeamInfo::~CdsBeamInfo()
{
     return;
}

//==========//
// QscatCds //
//==========//

QscatCds::QscatCds()
:   priDn(0), txPulseWidthDn(0), rxGateDelayDn(0), spinRate(LOW_SPIN_RATE),
    useRgc(0), useDtc(0), useBYUDop(0), useBYURange(0), useSpectralDop(0),
    useSpectralRange(0), azimuthIntegrationRange(0), azimuthStepSize(0),
    orbitTicksPerOrbit(0), orbitTime(0), orbitStep(0), eqxTime(0.0),
    rawEncoder(0), heldEncoder(0)
{
    return;
}

QscatCds::~QscatCds()
{
    return;
}

//-------------------//
// QscatCds::SetTime //
//-------------------//

int
QscatCds::SetTime(
    double  new_time)
{
    time = new_time;
    instrumentTime = (unsigned int)(time * INSTRUMENT_TICKS_PER_SECOND);

    double time_since_eqx = time - eqxTime;
    orbitTime = (unsigned int)(time_since_eqx * ORBIT_TICKS_PER_SECOND);

    return(1);
}

//----------------------//
// QscatCds::SetEqxTime //
//----------------------//

int
QscatCds::SetEqxTime(
    double  eqx_time)
{
    eqxTime = eqx_time;
    return(1);
}

//-------------------------------------//
// QscatCds::SetTimeWithInstrumentTime //
//-------------------------------------//

int
QscatCds::SetTimeWithInstrumentTime(
    unsigned int  ticks)
{
    instrumentTime = ticks;
    time = ((double)ticks + 0.5) / INSTRUMENT_TICKS_PER_SECOND;
    return(1);
}

//-------------------------//
// QscatCds::OrbitFraction //
//-------------------------//

double
QscatCds::OrbitFraction()
{
    double frac = (double)(orbitTime % orbitTicksPerOrbit) /
        (double)orbitTicksPerOrbit;
    return(frac);
}

//------------------------------//
// QscatCds::SetAndGetOrbitStep //
//------------------------------//

unsigned short
QscatCds::SetAndGetOrbitStep()
{
    float ticks_per_orbit_step = (float)orbitTicksPerOrbit /
        (float)ORBIT_STEPS;
    orbitStep = (unsigned short)((float)(orbitTime % orbitTicksPerOrbit) /
        ticks_per_orbit_step);
    orbitStep %= ORBIT_STEPS;
    return(orbitStep);
}

//------------------------------//
// QscatCds::GetCurrentBeamInfo //
//------------------------------//

CdsBeamInfo*
QscatCds::GetCurrentBeamInfo()
{
    return(&(beamInfo[currentBeamIdx]));
}

//------------------------------//
// QscatCds::GetAssumedSpinRate //
//------------------------------//
// returns the assumed spin rate in rpm

double
QscatCds::GetAssumedSpinRate()
{
    switch(spinRate)
    {
        case LOW_SPIN_RATE:
            return(18.0);
            break;
        case HIGH_SPIN_RATE:
            return(19.8);
            break;
        default:
            return(0.0);
            break;
    }
    return(0.0);
}

//--------------------------------//
// QscatCds::EstimateIdealEncoder //
//--------------------------------//
// This returns an ideal encoder estimate.  Basically, a quantized azimuth.

unsigned short
QscatCds::EstimateIdealEncoder()
{
    //-------------//
    // dereference //
    //-------------//

    CdsBeamInfo* cds_beam_info = GetCurrentBeamInfo();
    RangeTracker* range_tracker = &(cds_beam_info->rangeTracker);

    //---------------------------------------//
    // get previous pulses raw encoder value //
    //---------------------------------------//

    unsigned int int_encoder = (unsigned int)(heldEncoder & 0x7fff);

    //----------------------//
    // apply encoder offset //
    //----------------------//

    unsigned int encoder_offset;
    if (heldEncoder & 0x8000)
    {
        // encoder B
        encoder_offset = CDS_ENCODER_B_OFFSET;
    }
    else
    {
        // encoder A
        encoder_offset = CDS_ENCODER_A_OFFSET;
    }
    int_encoder += encoder_offset;

    //-------------------//
    // apply beam offset //
    //-------------------//

    unsigned int beam_offset;
    if (currentBeamIdx == 0)
    {
        // beam A
        beam_offset = BEAM_A_OFFSET;
    }
    else
    {
        // beam B
        beam_offset = BEAM_B_OFFSET;
    }
    int_encoder += beam_offset;

    //-------------------------------------------//
    // apply internal delay and centering offset //
    //-------------------------------------------//

    float spin_rate = GetAssumedSpinRate();
    unsigned short ant_dn_per_pri = (unsigned short)
        ( ( ( ((float)priDn / 10.0) * 32768.0 * spin_rate) /
        (60.0 * 1000.0)) + 0.5);
    float rx_range_mem = range_tracker->rxRangeMem;

    int_encoder += (unsigned int)( (float)ant_dn_per_pri * (1.0 +
        (rx_range_mem + (float)txPulseWidthDn) /
        ((float)priDn * 4.0)) + 0.5);

    //-----------------//
    // mod the encoder //
    //-----------------//

    unsigned short encoder = (unsigned short)(int_encoder % ENCODER_N);

    return(encoder);
}

//-------------------//
// QscatCds::LoadRgc //
//-------------------//

int
QscatCds::LoadRgc(
    int          beam_idx,
    const char*  file)
{
    if (! beamInfo[beam_idx].rangeTracker.ReadBinary(file))
        return(0);
    return(1);
}

//-------------------//
// QscatCds::LoadDtc //
//-------------------//

int
QscatCds::LoadDtc(
    int          beam_idx,
    const char*  file)
{
    if (! beamInfo[beam_idx].dopplerTracker.ReadBinary(file))
        return(0);
    return(1);
}

//-----------------------------//
// QscatCds::CmdTxPulseWidthEu //
//-----------------------------//

int
QscatCds::CmdTxPulseWidthEu(
    float      tx_pulse_width,
    QscatSes*  qscat_ses)
{
    txPulseWidthDn = (unsigned char)(tx_pulse_width /
        TX_PULSE_WIDTH_CMD_RESOLUTION + 0.5);

    if (! qscat_ses->CmdTxPulseWidthDn(txPulseWidthDn))
        return(0);

    return(1);
}

//--------------------//
// QscatCds::CmdPriEu //
//--------------------//

int
QscatCds::CmdPriEu(
    float      pri,
    QscatSes*  qscat_ses)
{
    priDn = (unsigned char)(pri / PRI_CMD_RESOLUTION + 0.5);

    if (! qscat_ses->CmdPriDn(priDn))
        return(0);

    return(1);
}

//----------------------------//
// QscatCds::CmdRxGateWidthEu //
//----------------------------//

int
QscatCds::CmdRxGateWidthEu(
    int        beam_idx,
    float      rx_gate_width,
    QscatSes*  qscat_ses)
{
    beamInfo[beam_idx].rxGateWidthDn = (unsigned char)(rx_gate_width /
        RX_GATE_WIDTH_CMD_RESOLUTION + 0.5);

    if (! qscat_ses->CmdRxGateWidthDn(beam_idx,
        beamInfo[beam_idx].rxGateWidthDn))
    {
        return(0);
    }

    return(1);
}

//-----------------------//
// QscatCds::CmdSpinRate //
//-----------------------//

int
QscatCds::CmdSpinRate(
    SpinRateE  spin_rate,
    QscatSas*  qscat_sas)
{
    spinRate = spin_rate;

    if (! qscat_sas->CmdSpinRate(spinRate))
        return(0);

    return(1);
}

//---------------------------------//
// QscatCds::CmdOrbitTicksPerOrbit //
//---------------------------------//

int
QscatCds::CmdOrbitTicksPerOrbit(
    unsigned int  orbit_ticks)
{
    orbitTicksPerOrbit = orbit_ticks;
    return(1);
}

//=======//
// Qscat //
//=======//

Qscat::Qscat()
:   systemLoss(0.0), systemTemperature(0.0)
{
    scatRf = &ses;
    scatDig = &cds;
    scatAnt = &sas;

    return;
}

Qscat::~Qscat()
{
    return;
}

//------------------------------//
// Qscat::GetCurrentCdsBeamInfo //
//------------------------------//

CdsBeamInfo*
Qscat::GetCurrentCdsBeamInfo()
{
    return(cds.GetCurrentBeamInfo());
}

//------------------------------//
// Qscat::GetCurrentSesBeamInfo //
//------------------------------//

SesBeamInfo*
Qscat::GetCurrentSesBeamInfo()
{
    return(ses.GetCurrentBeamInfo(cds.currentBeamIdx));
}

//--------------------------//
// Qscat::SetEncoderAzimuth //
//--------------------------//

int
Qscat::SetEncoderAzimuth(
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
Qscat::SetAllAzimuthsUsingGroundImpact(
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
// Qscat::GroundImpactToTxCenterAzimuth //
//--------------------------------------//
// Assumes Ground Impact Azimuth Angle Has been Set
// Computes and sets TxCenterAzimuth

int
Qscat::GroundImpactToTxCenterAzimuth(
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
    QscatTargetInfo qti;
    if (! TargetInfo(&antenna_frame_to_gc, spacecraft, vector, &qti))
        return(0);

    // rotate to the antenna to the ground impact azimuth
    double delta_t = qti.roundTripTime / 2.0;
    sas.antenna.SetTxCenterAzimuthAngle(antenna->groundImpactAzimuthAngle -
        delta_t * antenna->spinRate);
    return(1);
}

//--------------------------------------//
// Qscat::TxCenterToGroundImpactAzimuth //
//--------------------------------------//
// assumes txCenterAzimuthAngle has been set.
// computes and sets the ground impact azimuth

int
Qscat::TxCenterToGroundImpactAzimuth(
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
    QscatTargetInfo qti;
    if (! TargetInfo(&antenna_frame_to_gc, spacecraft, vector, &qti))
        return(0);

    // rotate to the antenna to the ground impact azimuth
    double delta_t = qti.roundTripTime / 2.0;
    sas.antenna.SetGroundImpactAzimuthAngle(sas.antenna.txCenterAzimuthAngle +
        delta_t * sas.antenna.spinRate);

    return(1);
}

//---------------------------------//
// Qscat::TxCenterToEncoderAzimuth //
//---------------------------------//
// Assumes Tx Center Azimuth Angle Has been Set
// Computes and sets UNQUANTIZED Encoder azimuth
int
Qscat::TxCenterToEncoderAzimuth()
{
    double delta_t = GetEncoderToTxCenterDelay();
    sas.antenna.SetEncoderAzimuthAngle(sas.antenna.txCenterAzimuthAngle -
        delta_t * sas.antenna.spinRate);
    return(1);
}

//----------------------------------//
// Qscat::GetEncodertoTxCenterDelay //
//----------------------------------//

double
Qscat::GetEncoderToTxCenterDelay()
{
    return(ses.txPulseWidth / 2.0 - T_ENC + T_GRID + T_RC + T_EXC);
}

//-------------------------//
// Qscat::SetOtherAzimuths //
//-------------------------//
// this assumes the encoder azimuth has been set

int
Qscat::SetOtherAzimuths(
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

    return(1);
}

//---------------------//
// Qscat::LocateSlices //
//---------------------//

int
Qscat::LocateSlices(
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

    meas_spot->FreeContents();
    meas_spot->time = cds.time;
    meas_spot->scOrbitState = *orbit_state;
    meas_spot->scAttitude = *attitude;

    //--------------------------------//
    // generate the coordinate switch //
    //--------------------------------//

    CoordinateSwitch antenna_frame_to_gc = AntennaFrameToGC(orbit_state,
        attitude, antenna, antenna->txCenterAzimuthAngle);

    //------------------//
    // find beam center //
    //------------------//

    double look, azim;
    if (! beam->GetElectricalBoresight(&look, &azim))
        return(0);

    //-------------------//
    // for each slice... //
    //-------------------//

    int total_slices = ses.GetTotalSliceCount();
    for (int slice_idx = 0; slice_idx < total_slices; slice_idx++)
    {
        //-------------------------//
        // create a new measurment //
        //-------------------------//

        Meas* meas = new Meas();
        meas->pol = beam->polarization;

        //----------------------------------------//
        // determine the baseband frequency range //
        //----------------------------------------//

        float f1, bw, f2;
        if (! ses.GetSliceFreqBw(slice_idx, &f1, &bw))
            return(0);
        f2 = f1 + bw;

        //----------------//
        // find the slice //
        //----------------//

        EarthPosition centroid;
        Vector3 look_vector;
        // guess at a reasonable slice frequency tolerance of .1%
        float ftol = bw / 1000.0;
        if (! FindSlice(&antenna_frame_to_gc, spacecraft, look, azim,
            f1, f2, ftol, &(meas->outline), &look_vector, &centroid))
        {
            return(0);
        }

        //---------------------------//
        // generate measurement data //
        //---------------------------//

        // get local measurement azimuth
        CoordinateSwitch gc_to_surface =
            centroid.SurfaceCoordinateSystem();
        Vector3 rlook_surface = gc_to_surface.Forward(look_vector);
        double r, theta, phi;
        rlook_surface.SphericalGet(&r, &theta, &phi);
        meas->eastAzimuth = phi;

        // get incidence angle
        meas->incidenceAngle = centroid.IncidenceAngle(look_vector);
        meas->centroid = centroid;
        meas->bandwidth = bw;

        //-----------------------------//
        // add measurment to meas spot //
        //-----------------------------//

        meas_spot->Append(meas);
    }
    return(1);
}

//------------------//
// Qscat::FindSlice //
//------------------//

int
Qscat::FindSlice(
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
// Qscat::FindPeakResponseAtFreq //
//-------------------------------//
// Finds the look, azimuth, and response value for the peak spatial response at
// a given baseband frequency

#define LOOK_OFFSET         0.005
#define AZIMUTH_OFFSET      0.005
#define ANGLE_OFFSET        0.002       // start delta for golden section
#define ANGLE_TOL           0.00001     // within this of peak gain
#define MAX_PASSES          10  // Maximum  number of passes through main loop

int
Qscat::FindPeakResponseAtFreq(
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
    QscatTargetInfo qti;
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
// Qscat::FindFreq //
//-----------------//
// Finds the look and azimuth of a point with the given frequency

int
Qscat::FindFreq(
    CoordinateSwitch*  antenna_frame_to_gc,
    Spacecraft*        spacecraft,
    float              target_freq,
    float              freq_tol,
    double*            look,
    double*            azim)
{
    Vector3 vector;
    QscatTargetInfo qti;
    do
    {
        //-------------------------------------//
        // get the starting baseband frequency //
        //-------------------------------------//

        vector.SphericalSet(1.0, *look, *azim);
        QscatTargetInfo qti;
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
// Qscat::FindSliceCorners //
//-------------------------//

#define PEAK_ANGLE_OFFSET       0.00875     // about 0.50 degree
#define LOCAL_ANGLE_OFFSET      0.000875    // about 0.050 degree

int
Qscat::FindSliceCorners(
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
// Qscat::FindPeakResponseForSlice //
//---------------------------------//
// Finds the peak response in a slice

int
Qscat::FindPeakResponseForSlice(
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
// Qscat::SpatialResponse //
//------------------------//

int
Qscat::SpatialResponse(
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
// Qscat::SpatialResponse //
//------------------------//
// same as above but returns float

int
Qscat::SpatialResponse(
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
// Qscat::FreqGradient //
//---------------------//
// Calculate the baseband frequency gradient

int
Qscat::FreqGradient(
    CoordinateSwitch*  antenna_frame_to_gc,
    Spacecraft*        spacecraft,
    double             look,
    double             look_offset,
    double             azim,
    double             azim_offset,
    double*            df_dlook,
    double*            df_dazim)
{
    QscatTargetInfo qti_1, qti_2;

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
// Qscat::FindPeakResponseUsingDeltas //
//------------------------------------//

int
Qscat::FindPeakResponseUsingDeltas(
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
// Qscat::TargetInfo //
//-------------------//
// Compute some useful numbers for the target on the earth's surface
// intercepted by a particular direction in the antenna frame.
// The vector is a directional vector specified in the antenna frame.

int
Qscat::TargetInfo(
    CoordinateSwitch*  antenna_frame_to_gc,
    Spacecraft*        spacecraft,
    Vector3            vector,
    QscatTargetInfo*   qti)
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
    SesBeamInfo* ses_beam_info = GetCurrentSesBeamInfo();
    double pulse_width = ses.txPulseWidth;
    double gate_width = ses_beam_info->rxGateWidth;
    double mu = ses.chirpRate;

    double chirp_start = mu *
        ( (pulse_width + RX_GATE_DELAY_CMD_RESOLUTION) / 2.0 + T_GRID);
    double dechirp_start = mu * (gate_width / 2.0);

    qti->basebandFreq = -ses.txDoppler - qti->dopplerFreq -
        chirp_start + dechirp_start + F_PROC -
        mu * (qti->roundTripTime - ses.rxGateDelay);

    return(1);
}

//-------------------------------//
// Qscat::SpatialResponseQuadFit //
//-------------------------------//
// Generates coefficients for a quadratic fit of a beam pattern

int
Qscat::SpatialResponseQuadFit(
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

    QscatTargetInfo qti;
    Vector3 vector;
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

//-----------------------------//
// Qscat::LocateSliceCentroids //
//-----------------------------//

int
Qscat::LocateSliceCentroids(
    Spacecraft*  spacecraft,
    MeasSpot*    meas_spot,
    float*       Esn,
    float        gain_threshold,
    int          max_slices)
{
    //-----------//
    // predigest //
    //-----------//

    Antenna* antenna = &(sas.antenna);
    Beam* beam = GetCurrentBeam();
    OrbitState* orbit_state = &(spacecraft->orbitState);
    Attitude* attitude = &(spacecraft->attitude);
    QscatTargetInfo qti;

    //------------------//
    // set up meas spot //
    //------------------//

    meas_spot->FreeContents();
    meas_spot->time = cds.time;
    meas_spot->scOrbitState = *orbit_state;
    meas_spot->scAttitude = *attitude;

    //--------------------------------//
    // generate the coordinate switch //
    //--------------------------------//

    CoordinateSwitch antenna_frame_to_gc = AntennaFrameToGC(orbit_state,
        attitude, antenna, antenna->txCenterAzimuthAngle);

    //------------------//
    // find beam center //
    //------------------//

    double center_look, center_azim;
    if (! GetPeakSpatialResponse2(&antenna_frame_to_gc, spacecraft, beam,
        sas.antenna.spinRate, &center_look, &center_azim))
    {
        return(0);
    }

        //-----------------------------------//
        // Calculate center line of the spot //
        // for use in determining centroids  //
        // of outer slices.                  //
        //-----------------------------------//

        // Find central frequency of spot
        Vector3 center_vector;
        center_vector.SphericalSet(1.0, center_look, center_azim);
        TargetInfo(&antenna_frame_to_gc, spacecraft, center_vector,
            &qti);
        float spot_center_freq=qti.basebandFreq;

    // Find Peak Gain Locations + and - 5 kHz from the center frequency
        // for now just use 8 Hz as the tolerance for calculating the
        // these locations.

        double neg_look=center_look;
    double neg_azim=center_azim;
        double pos_look=center_look;
        double pos_azim=center_azim;

    float dummy_gain;
        float freq_offset=5000;
        double ftol_center_line=8;
    if (! FindPeakResponseAtFreq(&antenna_frame_to_gc, spacecraft,
            spot_center_freq + freq_offset, ftol_center_line, &pos_look,
            &pos_azim, &dummy_gain) ||
        ! FindPeakResponseAtFreq(&antenna_frame_to_gc, spacecraft,
            spot_center_freq - freq_offset, ftol_center_line, &neg_look,
            &neg_azim, &dummy_gain))
    {
        fprintf(stderr,
            "LocateSliceCentroids: error finding peak gains for centerline.\n");
        return(0);
    }

    // Calculate coefficients for (look,azimuth) vs freq center lines
        double dlookdfreq=(pos_look-neg_look)/(2*freq_offset);
        double dazimdfreq=(pos_azim-neg_azim)/(2*freq_offset);

    //--------------------------//
    // find gain at beam center //
    //--------------------------//

    float peak_two_way_gain;
    if (gain_threshold)
    {
        if (SpatialResponse(&antenna_frame_to_gc, spacecraft,
            center_look, center_azim, &peak_two_way_gain) != 1)
        {
            return(0);
        }
    }

    //-------------------//
    // for each slice... //
    //-------------------//

    // Assume that every incoming spot has the same number of slices.
    int total_slices = ses.GetTotalSliceCount();
    int slice_count = 0;
    float* gains = (float*)malloc(total_slices*sizeof(float));
    if (! gains)
    {
        fprintf(stderr,"Error allocating memory in LocateSliceCentroids\n");
        return(0);
    }

    for (int slice_idx = 0; slice_idx < total_slices; slice_idx++)
    {
        //----------------------------------//
        // determine the centroid frequency //
        //----------------------------------//

        float f1, bw;
        if (! ses.GetSliceFreqBw(slice_idx, &f1, &bw))
            return(0);
        float centroid_freq = f1 + bw / 2.0;
        float ftol = bw / 1000.0;

        //------------------------------------------//
        // find the slice centroid look and azimuth //
        //------------------------------------------//

        float gain;
        double look = center_look;
        double azim = center_azim;
        if (! FindPeakResponseAtFreq(&antenna_frame_to_gc, spacecraft,
            centroid_freq, ftol, &look, &azim, &gain))
        {
            // If all else fails
            // Calculate centroid look and
            // azimuth using precalculated center line

            look=center_look+(centroid_freq-spot_center_freq)*dlookdfreq;
            azim=center_azim+(centroid_freq-spot_center_freq)*dazimdfreq;
            if (! SpatialResponse(&antenna_frame_to_gc, spacecraft,
                look, azim, &gain))
            {
                return(0);
            }
        }

        //--------------------//
        // threshold the gain //
        //--------------------//

        if (gain_threshold && gain / peak_two_way_gain < gain_threshold)
            continue;

        gains[slice_count] = gain;

        //-------------------------//
        // create a new measurment //
        //-------------------------//

        Meas* meas = new Meas();
        // We assume that the slices are sequential.
        abs_to_rel_idx(slice_idx,total_slices,&(meas->startSliceIdx));
        meas->numSlices = 1;
        meas->pol = beam->polarization;
        if (Esn) meas->value = Esn[slice_idx];

        //--------------------------------//
        // find the centroid on the earth //
        //--------------------------------//

        Vector3 rlook_antenna;
        rlook_antenna.SphericalSet(1.0, look, azim);
        Vector3 rlook_gc = antenna_frame_to_gc.Forward(rlook_antenna);
        EarthPosition centroid;
        switch(earth_intercept(spacecraft->orbitState.rsat, rlook_gc,
                       &centroid)){
        case 0:
          return(0);
        case 1:
          break;
        case 2:
          continue;
        }

        //---------------------------//
        // generate measurement data //
        //---------------------------//

        // get local measurement azimuth
        CoordinateSwitch gc_to_surface = centroid.SurfaceCoordinateSystem();
        Vector3 rlook_surface = gc_to_surface.Forward(rlook_gc);
        double r, theta, phi;
        rlook_surface.SphericalGet(&r, &theta, &phi);
        meas->eastAzimuth = phi;

        // get incidence angle
        meas->incidenceAngle = centroid.IncidenceAngle(rlook_gc);
        meas->centroid = centroid;
        meas->bandwidth = bw;

        //-----------------------------//
        // add measurment to meas spot //
        //-----------------------------//

        meas_spot->Append(meas);
        slice_count++;
    }

    //------------------------------------------------------------------//
    // Throw out the low gain slices to stay within the max slice count //
    //------------------------------------------------------------------//

    if (max_slices && slice_count > max_slices)
    {
        int* indx;
        insertion_sort(slice_count,gains,&indx);

        meas_spot->GotoHead();
        Meas* meas;
        int flag;

        for (int i = 0; i < slice_count; i++)
        {   // Check each slice for possible deletion.
            flag = 0;   // not removed yet
            for (int j = 0; j < slice_count - max_slices; j++)
            {   // Check for a match with the index of a low gain slice.
                if (indx[j] == i)
                {   // Found a low gain slice, so dump it.
                    meas = meas_spot->RemoveCurrent();
                    if (meas) delete meas;
                    flag = 1;   // removed
                }
            }
            if (!flag) meas = meas_spot->GetNext();
        }
        if (indx) free(indx);
    }

    if (gains)
    {
        free(gains);
    }

    return(1);
}

//-----------------//
// Qscat::IdealRtt //
//-----------------//
// Calculate the ideal round trip time.  The attitude is set to 0,0,0.
// Used for the RGC or for calculating the commanded round trip time.
// If use_flags is nonzero then the round trip time BYU reference vector
// or the Spectral peak is returned if the appropriate flag in qscat.cds
// is set.
// Otherwise the Spatial peak is returned regardless of the values of the
// flags.

double
Qscat::IdealRtt(
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
    if (cds.useBYURange && use_flags)
    {
      if(! GetBYUBoresight(&sp_zero_att, this, &look, &azim)){
        return(0);
      }
    }
    else if(cds.useSpectralRange && use_flags)
    {
        if (! GetPeakSpectralResponse(&zero_rpy_antenna_frame_to_gc,
            &sp_zero_att, this, &look, &azim))
        {
            return(0);
        }
    }
    else
    {
        if (! GetPeakSpatialResponse2(&zero_rpy_antenna_frame_to_gc,
            &sp_zero_att, beam, azimuth_rate, &look, &azim))
        {
            exit(1);
        }
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
// Qscat::IdealCommandedDoppler //
//------------------------------//
// Estimate the ideal commanded doppler frequency.

#define DOPPLER_ACCURACY    1.0     // 1 Hz

int
Qscat::IdealCommandedDoppler(
    Spacecraft*       spacecraft,
    QscatTargetInfo*  qti_out = NULL)
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
        sp_zero_att.orbitState=*sc_orbit_state;
        sp_zero_att.attitude=zero_rpy;

    //-------------------------------------------//
    // find the current beam's two-way peak gain //
    //-------------------------------------------//

    Beam* beam = GetCurrentBeam();
    double azimuth_rate = sas.antenna.spinRate;
    double look, azim;
    if (cds.useBYUDop)
    {
        if (! GetBYUBoresight(&sp_zero_att, this, &look, &azim))
        {
            return(0);
        }
    }
    else if(cds.useSpectralDop)
    {
        if (! GetPeakSpectralResponse(&zero_rpy_antenna_frame_to_gc,
            &sp_zero_att, this, &look, &azim))
        {
            return(0);
        }
    }
    else
    {
        if (! GetPeakSpatialResponse2(&zero_rpy_antenna_frame_to_gc,
            &sp_zero_att, beam, azimuth_rate, &look, &azim))
        {
            return(0);
        }
    }

    //-----------------------------//
    // calculate commanded Doppler //
    //-----------------------------//

    Vector3 vector;
    vector.SphericalSet(1.0, look, azim);
    QscatTargetInfo qti;
    ses.CmdTxDopplerEu(0.0);
    do
    {
        TargetInfo(&zero_rpy_antenna_frame_to_gc, spacecraft, vector,
            &qti);
        float freq = ses.txDoppler + qti.basebandFreq;
        ses.CmdTxDopplerEu(freq);
    } while (fabs(qti.basebandFreq) > DOPPLER_ACCURACY);

    if (qti_out)
        *qti_out = qti;

    return(1);
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

int
SetDelayAndFrequency(
    Spacecraft*       spacecraft,
    Qscat*            qscat,
    QscatTargetInfo*  qti)
{
    //------------------------------------------------------//
    // calculate the encoder value to use for the algorithm //
    //------------------------------------------------------//

    // remember the raw encoder for putting it in telemetry
    qscat->cds.rawEncoder = qscat->cds.heldEncoder;

    // estimate current encoder from the held encoder
    unsigned short ideal_encoder = qscat->cds.EstimateIdealEncoder();

    //---------------------------------------------//
    // sample the encoder for the next calculation //
    //---------------------------------------------//
    // this needs to be done before rotating the antenna

    qscat->cds.heldEncoder = qscat->sas.GetEncoder();

    //-----------------------------//
    // calculate the rx gate delay //
    //-----------------------------//

    // these defaults will produce no delay quantization correction
    // if ideal range tracking is used
    qscat->cds.rxGateDelayDn = 0;
    float rx_gate_delay_fdn = 0.0;

    if (qscat->cds.useRgc)
    {
        // tracking algorithm
        CdsBeamInfo* cds_beam_info = qscat->GetCurrentCdsBeamInfo();
        RangeTracker* range_tracker = &(cds_beam_info->rangeTracker);
        range_tracker->GetRxGateDelay(qscat->cds.orbitStep, ideal_encoder,
            cds_beam_info->rxGateWidthDn, qscat->cds.txPulseWidthDn,
            &(qscat->cds.rxGateDelayDn), &rx_gate_delay_fdn);
        qscat->ses.CmdRxGateDelayDn(qscat->cds.rxGateDelayDn);
    }
    else
    {
        // ideal delay
        float rtt = qscat->IdealRtt(spacecraft, 1);
        SesBeamInfo* ses_beam_info = qscat->GetCurrentSesBeamInfo();
        float delay = rtt +
            (qscat->ses.txPulseWidth - ses_beam_info->rxGateWidth) / 2.0;
        qscat->ses.CmdRxGateDelayEu(delay);
    }

    //----------------------------//
    // calculate the tx frequency //
    //----------------------------//

    if (qscat->cds.useDtc)
    {
        // tracking algorithm
        CdsBeamInfo* cds_beam_info = qscat->GetCurrentCdsBeamInfo();
        DopplerTracker* doppler_tracker = &(cds_beam_info->dopplerTracker);
        short doppler_dn;
        doppler_tracker->GetCommandedDoppler(qscat->cds.orbitStep,
            ideal_encoder, qscat->cds.rxGateDelayDn, rx_gate_delay_fdn,
            &doppler_dn);
        qscat->ses.CmdTxDopplerDn(doppler_dn);
    }
    else
    {
        // ideal frequency
        qscat->IdealCommandedDoppler(spacecraft, qti);
    }

    return(1);
}

//-------------------------------//
// SetOrbitStepDelayAndFrequency //
//-------------------------------//

int
SetOrbitStepDelayAndFrequency(
    Spacecraft*  spacecraft,
    Qscat*       qscat)
{
    //---------------------------------//
    // calculate orbit step, if needed //
    //---------------------------------//

    if (qscat->cds.useRgc || qscat->cds.useDtc)
        qscat->cds.SetAndGetOrbitStep();

    //-------------------------------//
    // calculate delay and frequency //
    //-------------------------------//

    return(SetDelayAndFrequency(spacecraft, qscat));
}

