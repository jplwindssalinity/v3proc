//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

static const char rcs_id_qscat_c[] =
    "@(#) $Id$";

#include "Qscat.h"
#include "Constants.h"
#include "InstrumentGeom.h"

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
    useRgc(0), useDtc(0),
    useBYUDop(0), useBYURange(0), useSpectralDop(0), useSpectralRange(0),
    azimuthIntegrationRange(0), azimuthStepSize(0), rangeGateClipping(0),
    orbitTicksPerOrbit(0), currentBeamIdx(0), 
    orbitTime(0), orbitStep(0), time(0.0), eqxTime(0.0), rawEncoder(0),
    heldEncoder(0)
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
    return;
}

Qscat::~Qscat()
{
    return;
}

//-----------------------//
// Qscat::GetCurrentBeam //
//-----------------------//

Beam*
Qscat::GetCurrentBeam()
{
    return(&(sas.antenna.beam[cds.currentBeamIdx]));
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
    Spacecraft*         spacecraft,
    Qscat*              qscat,
    TargetInfoPackage*  tip)
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
        float rtt= IdealRtt(spacecraft, qscat, 1);
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
        IdealCommandedDoppler(spacecraft, qscat, tip);
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
	   Spacecraft* spacecraft,
	   double      angle){
 sas.antenna.SetGroundImpactAzimuthAngle(angle);
 if(!GroundImpactToTxCenterAzimuth(spacecraft))return(0); 
 if(!TxCenterToEncoderAzimuth()) return(0);
 return(1);
}
//----------------------------------------//
// Qscat::GroundImpactToTxCenterAzimuth   //
//----------------------------------------//
// Assumes Ground Impact Azimuth Angle Has been Set
// Computes and sets TxCenterAzimuth
int 
Qscat::GroundImpactToTxCenterAzimuth(Spacecraft* spacecraft){    

    // estimate the range to the surface using ground imapct
    CoordinateSwitch antenna_frame_to_gc =
        AntennaFrameToGC(&(spacecraft->orbitState), &(spacecraft->attitude),
        &(sas.antenna), sas.antenna.groundImpactAzimuthAngle);
    double look, azim;
    Beam* beam = GetCurrentBeam();
    if (! beam->GetElectricalBoresight(&look, &azim))
        return(0);
    Vector3 vector;
    vector.SphericalSet(1.0, look, azim);
    TargetInfoPackage tip;
    if (! TargetInfo(&antenna_frame_to_gc, spacecraft, this, vector, &tip))
        return(0);

    // rotate to the antenna to the ground impact azimuth
    double delta_t = tip.roundTripTime / 2.0;
    sas.antenna.SetTxCenterAzimuthAngle(sas.antenna.groundImpactAzimuthAngle -
        delta_t * sas.antenna.spinRate);
    return(1);
}
//----------------------------------------//
// Qscat::TxCenterToEncoderAzimuth        //
//----------------------------------------//
// Assumes Tx Center Azimuth Angle Has been Set
// Computes and sets UNQUANTIZED Encoder azimuth
int
Qscat::TxCenterToEncoderAzimuth(){
    double delta_t = GetEncoderToTxCenterDelay();
    sas.antenna.SetEncoderAzimuthAngle(sas.antenna.txCenterAzimuthAngle -
        delta_t * sas.antenna.spinRate);
    return(1);
}

//---------------------------------------//
// Qscat::GetEncodertoTxCenterDelay      //
//---------------------------------------//
double
Qscat::GetEncoderToTxCenterDelay(){
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
    TargetInfoPackage tip;
    if (! TargetInfo(&antenna_frame_to_gc, spacecraft, this, vector, &tip))
        return(0);

    // rotate to the antenna to the ground impact azimuth
    delta_t = tip.roundTripTime / 2.0;
    sas.antenna.SetGroundImpactAzimuthAngle(sas.antenna.txCenterAzimuthAngle +
        delta_t * sas.antenna.spinRate);

    return(1);
}
