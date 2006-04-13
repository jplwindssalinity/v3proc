//==============================================================//
// Copyright (C) 1998-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef OVWM_H
#define OVWM_H

static const char rcs_id_ovwm_h[] =
    "@(#) $Id$";

#include "Antenna.h"
#include "Tracking.h"
#include "Spacecraft.h"
#include "Scatterometer.h"
#include "Meas.h"
#include "AmbigTable.h"
#include "PointTargetResponseTable.h"

#define NUMBER_OF_OVWM_BEAMS  4

//======================================================================
// CLASSES
//    OvwmTargetInfo, OvwmSes, OvwmSas, OvwmCds, OvwmEvent, Ovwm
//======================================================================



//======================================================================
// CLASS
//    OvwmTargetInfo
//
// DESCRIPTION
//    The OvwmTargetInfo class holds target information.
//======================================================================

class OvwmTargetInfo : public ScatTargetInfo
{
public:
    int    WriteAscii();

    float  dopplerFreq;      // Hz
    float  basebandFreq;     // Hz
};

//======================================================================
// CLASS
//    OvwmSes
//
// DESCRIPTION
//    The OvwmSes acts like the OVWM SES.
//======================================================================


class OvwmSes : public ScatRF
{
public:
    //--------------//
    // construction //
    //--------------//

    OvwmSes();
    ~OvwmSes();

    //-----------------//
    // getting/setting //
    //-----------------//

    float         GetRxGateWidth(int current_beam_idx);
    int           GetQRel(int slice_idx, float* q_slice);

    int  CmdTxPulseWidthDn(unsigned char tx_pulse_width_dn);
    int  CmdPriDn(unsigned int pri_dn);
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

    float        bri;             // the burst repetition interval (s)
    int          numTWTAs;        // number of TWTAs used to constrain timing
    int          numReceivers;    // number of receiver chains 
                                  // used to constrain timing
    int          numPulses;       // number of pulses in the burst
    int          numRangeLooksAveraged; // number of range looks averaged
                                       // on onboard processor
    float        txDelay[NUMBER_OF_OVWM_BEAMS]; // Delay from start of 
                                                // transmit cycle
                                          // to start transmission on each beam
    float        txPulseWidth;    // the transmit pulse width (s)
    float        txDoppler;       // the Doppler frequency shift (Hz)
    float        txFrequency;     // the transmit frequency (Hz)
    float        rxGateDelay;     // the gate delay (s)
    float        maxRangeWidth;    // maximum width of range window (km)

    float        baseTxFrequency; // the base transmitter frequency (Hz)
    float        pri;             // the pulse repetition interval (s)
    float        transmitPower;   // the transmit power (W)
                                  // - Not including transmitPathLoss
    float        rxGainEcho;      // dimensionless multiplicative factor
    float        rxGainNoise;     // dimensionless multiplicative factor
    float        calibrationBias; // dimensionless multiplicative factor
    float        chirpRate;       // chirp rate (Hz/s)
    float        chirpBandwidth;  // chirp bandwidth in Hz
    float        rangeRes;        // range pixel resolution in km
    float        dopplerRes;      // doppler pixel resolution in km
    int          numRangePixels;  // number of range pixels
    float        burstLength;     // duration of burst in s
    float        noiseBandwidth;         // Hz

    float        receivePathLoss;        // dimensionless divisive factor
    float        transmitPathLoss;       // dimensionless divisive factor
    float        loopbackLoss;           // dimensionless divisive factor
    float        loopbackLossRatio;      // dimensionless ratio

    // Bandwidth ratios for slices using nominal mode (0.5 ms)
    float* Qtable;

    float  rxGateWidth[NUMBER_OF_OVWM_BEAMS];
    unsigned short beamOffset[NUMBER_OF_OVWM_BEAMS];
};


//======================================================================
// CLASS
//      OvwmSas
//
// DESCRIPTION
//      The OvwmSas acts like the OVWM SAS.
//======================================================================



class OvwmSas : public ScatAnt
{
public:
    //--------------//
    // construction //
    //--------------//

    OvwmSas();
    ~OvwmSas();

    //-----------------//
    // setting/getting //
    //-----------------//

    int             SetAzimuthWithEncoder(unsigned short encoder_value);
    unsigned short  GetEncoder();
    unsigned short  AzimuthToEncoder(double azimuth);


    int  SetSpinRate(double spin_rate_in_rpm);
    double GetSpinRate();

    //-----------//
    // variables //
    //-----------//

    double    sampledAzimuth;    // the antenna azimuth angle when sampled
    float     encoderOffset;

};

//======================================================================
// CLASS
//      OvwmCds
//
// DESCRIPTION
//      The OvwmCds acts like the OVWM CDS.
//======================================================================

#define ORBIT_TICKS_PER_SECOND         32
#define INSTRUMENT_TICKS_PER_SECOND    32

#define TX_PULSE_WIDTH_CMD_RESOLUTION  4.9903E-5
#define RX_GATE_WIDTH_CMD_RESOLUTION   4.9903E-5
#define RX_GATE_DELAY_CMD_RESOLUTION   4.9903E-5
#define TX_FREQUENCY_CMD_RESOLUTION    2.0E3
#define OVWM_PRI_CMD_RESOLUTION        9.9806E-8

#define ORBIT_STEPS           256
#define BEAM_A_OFFSET         0
#define BEAM_B_OFFSET         0


class OvwmCds : public ScatDig
{
public:
    //--------------//
    // construction //
    //--------------//

    OvwmCds();
    ~OvwmCds();

    //-----------------//
    // getting/setting //
    //-----------------//

    int  SetTime(double new_time);
    int  SetEqxTime(double eqx_time);

    double          OrbitFraction();
    unsigned short  SetAndGetOrbitStep();
    unsigned char   GetRxGateWidthDn();
    unsigned short  EstimateIdealEncoder(int calculate_prev_delay = 0,
                        unsigned short prev_range_step = 0,
                        unsigned short prev_azimuth_step = 0,
                        unsigned char prev_rx_gate_width_dn = 0,
                        unsigned char prev_tx_pulse_width_dn = 0,
			double  spin_rate = 0);

    int   LoadRgc(int beam_idx, const char* file);
    int   LoadDtc(int beam_idx, const char* file);
    void  SetTrackingChirpRate(int beam_idx, double tracking_mu);

    int   CmdTxPulseWidthEu(float tx_pulse_width, OvwmSes* ovwm_ses);
    int   CmdPriEu(float pri, OvwmSes* ovwm_ses);
    int   CmdRxGateWidthEu(int beam_idx, float rx_gate_width,
              OvwmSes* ovwm_ses);

    int   CmdOrbitTicksPerOrbit(unsigned int orbit_ticks);
    DopplerTracker* GetDopplerTracker();
    RangeTracker* GetRangeTracker();
    //-----------//
    // variables //
    //-----------//

    double          turnOnTime;

    unsigned int    priDn;
    unsigned char   txPulseWidthDn;
    unsigned char   rxGateDelayDn;
    short           txDopplerDn;

    // Flags and Parameters which are convenient to put here
    // in order to perform Ideal Doppler tracking to BYU ref
    // Vector, Spatial Response Peak and Spectral Response
    // Peak, but which do not really belong here.
    int             useRgc;
    int             useDtc;
    float           azimuthIntegrationRange;
    float           azimuthStepSize;
    float           xRefLook[NUMBER_OF_OVWM_BEAMS];
    float           xRefAzim[NUMBER_OF_OVWM_BEAMS];

    unsigned int    orbitTicksPerOrbit;
    unsigned char   rxGateWidthDn[NUMBER_OF_OVWM_BEAMS];
    RangeTracker    rangeTracker[NUMBER_OF_OVWM_BEAMS];
    DopplerTracker  dopplerTracker[NUMBER_OF_OVWM_BEAMS];
    unsigned short  beamEncoderOffset[NUMBER_OF_OVWM_BEAMS];
    unsigned int    orbitTime;
    unsigned short  orbitStep;
    unsigned int    instrumentTime;

    double          eqxTime;

    unsigned short  rawEncoder;   // used for the current pulse (ex-held)
    unsigned short  heldEncoder;  // sampled for the next pulse

    int             encoderOffset;
};

//======================================================================
// CLASS
//      OvwmEvent
//
// DESCRIPTION
//      The OvwmEvent object contains a OVWM event time and ID.
//======================================================================

class OvwmEvent
{
public:

    //-------//
    // enums //
    //-------//

    enum OvwmEventE { NONE, SCAT_EVENT};

    //--------------//
    // construction //
    //--------------//

    OvwmEvent();
    ~OvwmEvent();

    //-----------//
    // variables //
    //-----------//

    double       time;
    OvwmEventE  eventId;
    int          beamIdx;
};

//======================================================================
// CLASS
//      Ovwm
//
// DESCRIPTION
//      The Ovwm is the top level object for the OVWM instrument.
//======================================================================

class Ovwm : public Scatterometer
{
public:
    //--------------//
    // construction //
    //--------------//

    Ovwm();
    ~Ovwm();

    //-----------------//
    // setting/getting //
    //-----------------//

    unsigned char  GetRxGateWidthDn();
    float  GetRxGateWidth();
    int GetNumberOfPixels();
    DopplerTracker* GetDopplerTracker();
    RangeTracker* GetRangeTracker();

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

    int  MakePixels(MeasSpot* meas_spot);
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
             Spacecraft* spacecraft, Vector3 vector, OvwmTargetInfo* qti);
    int  SphericalTargetInfo(CoordinateSwitch* antenna_frame_to_gc,
             Spacecraft* spacecraft, Vector3 vector, OvwmTargetInfo* qti,
	     double range);
    int  FastDopplerRangeToTbfLook(double doppler, double range, 
	     double local_radius, Vector3 local_center,
	     Spacecraft* spacecraft,double u1[3],
	     double u2[3]);
    int  SpatialResponseQuadFit(CoordinateSwitch* antenna_frame_to_gc,
             Spacecraft* spacecraft, double look[3], double azim[3],
             double s[3], double c[3]);
    int  LocatePixels(Spacecraft* spacecraft, MeasSpot* meas_spot);
    double  IdealRtt(Spacecraft* spacecraft, int use_flags = 0);
    int     IdealCommandedDoppler(Spacecraft* spacecraft,
                OvwmTargetInfo* qti_out = NULL, int use_attitude = 0);

/*
    int  SetAntennaToTxCenter(int pri_delay = 0);
    int  SetAntennaToGroundImpact(Spacecraft* spacecraft, int pri_delay = 0);
    int  SetAntennaFromHeldEncoder(unsigned short encoder);
*/

    //-----------//
    // variables //
    //-----------//

    OvwmCds  cds;
    OvwmSas  sas; 
    OvwmSes  ses;

    float     systemLoss;           // dimensionless multiplicative factor
    float     systemTemperature;    // K
    int       useRealAperture;      // turns off SAR simulation/processing.

};

//------------------//
// helper functions //
//------------------//

int  SetDelayAndFrequency(Spacecraft* spacecraft, Ovwm* ovwm,
         OvwmTargetInfo* qti = NULL, int estimate_encoder_method = 0);
int  SetOrbitStepDelayAndFrequency(Spacecraft* spacecraft, Ovwm* ovwm);
Meas::MeasTypeE  PolToMeasType(PolE pol);
PolE  MeasTypeToPol(Meas::MeasTypeE meas_type);
float effective_gate_width_to_slice_bandwidth(float egw);

#endif
