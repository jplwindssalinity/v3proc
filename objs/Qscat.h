//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef QSCAT_H
#define QSCAT_H

static const char rcs_id_qscat_h[] =
    "@(#) $Id$";

#include "Antenna.h"
#include "Tracking.h"
#include "Spacecraft.h"
#include "Scatterometer.h"
#include "Meas.h"

#define NUMBER_OF_QSCAT_BEAMS     2
#define ENCODER_N                 32768

//======================================================================
// CLASSES
//    QscatTargetInfo, QscatSes, QscatSas, QscatCds, QscatEvent, Qscat
//======================================================================

#define F_PROC  -1.06E3    // Hz

// most of these should be moved into the SES config
#define T_ENC   9E-9
#define T_GRID  19.87E-6
#define T_RC    9.968E-6
#define T_EXC   1E-6

//======================================================================
// CLASS
//      QscatTargetInfo
//
// DESCRIPTION
//      The QscatTargetInfo class holds target information.
//======================================================================

class QscatTargetInfo : public ScatTargetInfo
{
public:
    float  dopplerFreq;      // Hz
    float  basebandFreq;     // Hz
};

//======================================================================
// CLASS
//      QscatSes
//
// DESCRIPTION
//      The QscatSes acts like the QSCAT SES.
//======================================================================

class SesBeamInfo
{
public:
    SesBeamInfo();
    ~SesBeamInfo();

    float  rxGateWidth;    // the receiver gate width (s)
};

class QscatSes : public ScatRF
{
public:
    //--------------//
    // construction //
    //--------------//

    QscatSes();
    ~QscatSes();

    //-----------------//
    // getting/setting //
    //-----------------//

    SesBeamInfo*  GetCurrentBeamInfo(int current_beam_idx);
    float         GetTotalSignalBandwidth();
    int           GetTotalSliceCount();
    int           GetSliceFreqBw(int slice_idx, float* f1, float* bw);
    int           GetQRel(int slice_idx, float* q_slice);

    int  CmdTxPulseWidthDn(unsigned char tx_pulse_width_dn);
    int  CmdPriDn(unsigned char pri_dn);
    int  CmdRxGateWidthDn(int beam_idx, unsigned char rx_gate_width_dn);
    int  CmdRxGateWidthEu(int beam_idx, float rx_gate_width_eu);
    int  CmdRxGateDelayDn(unsigned char rx_gate_delay_dn);
    int  CmdRxGateDelayFdn(float rx_gate_delay_fdn);
    int  CmdRxGateDelayEu(float rx_gate_delay_eu);
    int  CmdTxDopplerDn(short tx_doppler_dn);
    int  CmdTxDopplerEu(float tx_doppler_eu);

    //------------//
    // Conversion //
    //------------//

    unsigned char tempToDn(float temp);

    //-----------//
    // variables //
    //-----------//

    float        txPulseWidth;    // the transmit pulse width (s)
    float        txDoppler;       // the Doppler frequency shift (Hz)
    float        txFrequency;     // the transmit frequency (Hz)
    float        rxGateDelay;     // the gate delay (s)

    float        baseTxFrequency; // the base transmitter frequency (Hz)
    float        pri;             // the pulse repetition interval (s)
    float        transmitPower;   // the transmit power (W)
                                  // - Not including transmitPathLoss
    float        rxGainEcho;      // dimensionless multiplicative factor
    float        rxGainNoise;     // dimensionless multiplicative factor
    float        calibrationBias; // dimensionless multiplicative factor
    float        chirpRate;       // chirp rate (Hz/s)
    float        fftBinBandwidth;        // Hz
    float        scienceSliceBandwidth;  // Hz
    int          scienceSlicesPerSpot;   // count
    float        guardSliceBandwidth;    // Hz
    int          guardSlicesPerSide;     // count
    float        noiseBandwidth;         // Hz

    float        receivePathLoss;        // dimensionless divisive factor
    float        transmitPathLoss;       // dimensionless divisive factor
    float        loopbackLoss;           // dimensionless divisive factor
    float        loopbackLossRatio;      // dimensionless ratio
    float        L13Coef[5];             // polynomial fit coef's
    float        L21Coef[5];             // polynomial fit coef's
    float        L23Coef[5];             // polynomial fit coef's
    float        LcalopCoef[5];          // polynomial fit coef's
    float        physicalTemperature;    // deg. C

    // Bandwidth ratios for slices using nominal mode (0.5 ms)
    float* Qtable;

    SesBeamInfo  beamInfo[NUMBER_OF_QSCAT_BEAMS];
};

//======================================================================
// CLASS
//      QscatSas
//
// DESCRIPTION
//      The QscatSas acts like the QSCAT SAS.
//======================================================================

// rpm
#define SAS_LOW_SPIN_RATE   18.0
#define SAS_HIGH_SPIN_RATE  19.8

// degrees
#define SAS_ENCODER_A_OFFSET     180.2646
#define SAS_ENCODER_B_OFFSET     0.3044

enum EncoderE { ENCODER_A, ENCODER_B };
enum SpinRateE { LOW_SPIN_RATE, HIGH_SPIN_RATE };

class QscatSas : public ScatAnt
{
public:
    //--------------//
    // construction //
    //--------------//

    QscatSas();
    ~QscatSas();

    //-----------------//
    // setting/getting //
    //-----------------//

    int             SetAzimuthWithEncoder(unsigned short encoder_value);
    unsigned short  GetEncoder();
    unsigned short  AzimuthToEncoder(double azimuth);

    int  CmdSpinRate(SpinRateE spin_rate);

    //-----------//
    // variables //
    //-----------//

    double    sampledAzimuth;    // the antenna azimuth angle when sampled
    EncoderE  encoderElectronics;
};

//======================================================================
// CLASS
//      QscatCds
//
// DESCRIPTION
//      The QscatCds acts like the QSCAT CDS.
//======================================================================

#define ORBIT_TICKS_PER_SECOND         32
#define INSTRUMENT_TICKS_PER_SECOND    32

#define TX_PULSE_WIDTH_CMD_RESOLUTION  4.9903E-5
#define RX_GATE_WIDTH_CMD_RESOLUTION   4.9903E-5
#define RX_GATE_DELAY_CMD_RESOLUTION   4.9903E-5
#define TX_FREQUENCY_CMD_RESOLUTION    2.0E3
#define PRI_CMD_RESOLUTION             9.9806E-5

#define ORBIT_STEPS           256
#define CDS_ENCODER_A_OFFSET  16408
#define CDS_ENCODER_B_OFFSET  28
#define BEAM_A_OFFSET         0
#define BEAM_B_OFFSET         0

class CdsBeamInfo
{
public:
    CdsBeamInfo();
    ~CdsBeamInfo();

    RangeTracker    rangeTracker;
    DopplerTracker  dopplerTracker;

    unsigned char   rxGateWidthDn;
};

class QscatCds : public ScatDig
{
public:
    //--------------//
    // construction //
    //--------------//

    QscatCds();
    ~QscatCds();

    //-----------------//
    // getting/setting //
    //-----------------//

    int  SetTime(double new_time);
    int  SetEqxTime(double eqx_time);
    int  SetTimeWithInstrumentTime(unsigned int ticks);

    double          OrbitFraction();
    unsigned short  SetAndGetOrbitStep();
    CdsBeamInfo*    GetCurrentBeamInfo();
    double          GetAssumedSpinRate();
    unsigned short  EstimateIdealEncoder(int calculate_prev_delay = 0,
                        unsigned short prev_range_step = 0,
                        unsigned short prev_azimuth_step = 0,
                        unsigned char prev_rx_gate_width_dn = 0,
                        unsigned char prev_tx_pulse_width_dn = 0);

    int  LoadRgc(int beam_idx, const char* file);
    int  LoadDtc(int beam_idx, const char* file);

    int  CmdTxPulseWidthEu(float tx_pulse_width, QscatSes* qscat_ses);
    int  CmdPriEu(float pri, QscatSes* qscat_ses);
    int  CmdRxGateWidthEu(int beam_idx, float rx_gate_width,
             QscatSes* qscat_ses);
    int  CmdSpinRate(SpinRateE spin_rate, QscatSas* qscat_sas);
    int  CmdOrbitTicksPerOrbit(unsigned int orbit_ticks);

    //-----------//
    // variables //
    //-----------//

    unsigned char   priDn;
    unsigned char   txPulseWidthDn;
    unsigned char   rxGateDelayDn;
    short           txDopplerDn;
    SpinRateE       spinRate;

// Flags and Parameters which are convenient to put here in order
// to perform Ideal Doppler tracking to BYU ref Vector, Spatial Response Peak
// and Spectral Response Peak, but which do not really belong here.
    int             useRgc;
    int             useDtc;
    int             useBYUDop;
    int             useBYURange;
    int             useSpectralDop;
    int             useSpectralRange;
    float           azimuthIntegrationRange;
    float           azimuthStepSize;
    float           xRefLook[NUMBER_OF_QSCAT_BEAMS];
    float           xRefAzim[NUMBER_OF_QSCAT_BEAMS];

    unsigned int    orbitTicksPerOrbit;
    CdsBeamInfo     beamInfo[NUMBER_OF_QSCAT_BEAMS];

    unsigned int    orbitTime;
    unsigned short  orbitStep;
    unsigned int    instrumentTime;

    double          eqxTime;

    unsigned short  rawEncoder;   // used for the current pulse (ex-held)
    unsigned short  heldEncoder;  // sampled for the next pulse
};

//======================================================================
// CLASS
//      QscatEvent
//
// DESCRIPTION
//      The QscatEvent object contains a QSCAT event time and ID.
//======================================================================

class QscatEvent
{
public:

    //-------//
    // enums //
    //-------//

    enum QscatEventE { NONE, SCAT_EVENT, LOOPBACK_EVENT, LOAD_EVENT };

    //--------------//
    // construction //
    //--------------//

    QscatEvent();
    ~QscatEvent();

    //-----------//
    // variables //
    //-----------//

    double       time;
    QscatEventE  eventId;
    int          beamIdx;
};

//======================================================================
// CLASS
//      Qscat
//
// DESCRIPTION
//      The Qscat is the top level object for the QSCAT instrument.
//======================================================================

class Qscat : public Scatterometer
{
public:
    //--------------//
    // construction //
    //--------------//

    Qscat();
    ~Qscat();

    //-----------------//
    // setting/getting //
    //-----------------//

    CdsBeamInfo*  GetCurrentCdsBeamInfo();
    SesBeamInfo*  GetCurrentSesBeamInfo();

    int  SetEncoderAzimuth(unsigned short encoder, int pri_delay);
    int  SetEncoderAzimuthUnquantized(double angle)
             { 
                 sas.antenna.SetEncoderAzimuthAngle(angle);
                 return(1);
             }

    int  SetAllAzimuthsUsingGroundImpact(Spacecraft* spacecraft, double angle);
    int  GroundImpactToTxCenterAzimuth(Spacecraft* spacecraft);
    int  TxCenterToGroundImpactAzimuth(Spacecraft* spacecraft);
    int  TxCenterToEncoderAzimuth();
    int  SetOtherAzimuths(Spacecraft* spacecraft);

    double  GetEncoderToTxCenterDelay();

    //----------//
    // geometry //
    //----------//

    int  MakeSlices(MeasSpot* meas_spot);
    int  LocateSlices(Spacecraft* spacecraft, MeasSpot* meas_spot);
    int  FindSlice(CoordinateSwitch* antenna_frame_to_gc,
             Spacecraft* spacecraft, double look, double azim, float freq_1,
             float freq_2, float freq_tol, Outline* outline,
             Vector3* look_vector, EarthPosition* centroid);
    int  FindPeakResponseAtFreq(CoordinateSwitch* antenna_frame_to_gc,
             Spacecraft* spacecraft, float target_freq, float freq_tol,
             double* look, double* azim, float* response,
             int ignore_range = 0);
    int  FindFreq(CoordinateSwitch* antenna_frame_to_gc,
             Spacecraft* spacecraft, float target_freq, float freq_tol,
             double* look, double* azim);
    int  FindSliceCorners(CoordinateSwitch* antenna_frame_to_gc,
             Spacecraft* spacecraft, double look, double azim,
             float target_gain, double corner_look[2], double corner_azim[2]);
    int  FindPeakResponseForSlice(CoordinateSwitch* antenna_frame_to_gc,
             Spacecraft* spacecraft, double look[2], double azim[2],
             float response[2], float* peak_response, int ignore_range = 0);
    int  SpatialResponse(CoordinateSwitch* antenna_frame_to_gc,
             Spacecraft* spacecraft, double look, double azim,
             double* response, int ignore_range = 0);
    int  SpatialResponse(CoordinateSwitch* antenna_frame_to_gc,
             Spacecraft* spacecraft, double look, double azim,
             float* response, int ignore_range = 0);
    int  FreqGradient(CoordinateSwitch* antenna_frame_to_gc,
             Spacecraft* spacecraft, double look, double look_offset,
             double azim, double azim_offset, double* df_dlook,
             double* df_dazim);
    int  FindPeakResponseUsingDeltas(CoordinateSwitch* antenna_frame_to_gc,
             Spacecraft* spacecraft, double delta_look, double delta_azim,
             double angle_offset, double angle_tol, double* look,
             double* azim, float* response, int ignore_range = 0);
    int  TargetInfo(CoordinateSwitch* antenna_frame_to_gc,
             Spacecraft* spacecraft, Vector3 vector, QscatTargetInfo* qti);
    int  SpatialResponseQuadFit(CoordinateSwitch* antenna_frame_to_gc,
             Spacecraft* spacecraft, double look[3], double azim[3],
             double s[3], double c[3]);
    int  LocateSliceCentroids(Spacecraft* spacecraft, MeasSpot* meas_spot,
             float gain_threshold = 0.0, int max_slices = 0);
    double  IdealRtt(Spacecraft* spacecraft, int use_flags = 0);
    int     IdealCommandedDoppler(Spacecraft* spacecraft,
                QscatTargetInfo* qti_out = NULL);

/*
    int  SetAntennaToTxCenter(int pri_delay = 0);
    int  SetAntennaToGroundImpact(Spacecraft* spacecraft, int pri_delay = 0);
    int  SetAntennaFromHeldEncoder(unsigned short encoder);
*/

    //-----------//
    // variables //
    //-----------//

    QscatCds  cds;
    QscatSas  sas;
    QscatSes  ses;

    float     systemLoss;           // dimensionless multiplicative factor
    float     systemTemperature;    // K
};

//------------------//
// helper functions //
//------------------//

int  SetDelayAndFrequency(Spacecraft* spacecraft, Qscat* qscat,
         QscatTargetInfo* qti = NULL, int estimate_encoder_method = 0);
int  SetOrbitStepDelayAndFrequency(Spacecraft* spacecraft, Qscat* qscat);
Meas::MeasTypeE  PolToMeasType(PolE pol);
PolE  MeasTypeToPol(Meas::MeasTypeE meas_type);

#endif
