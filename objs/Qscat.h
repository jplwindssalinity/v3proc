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

#define NUMBER_OF_QSCAT_BEAMS     2
#define ENCODER_N                 32768

//======================================================================
// CLASSES
//      QscatSes, QscatSas, QscatCds, Qscat
//======================================================================

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
    float  rxGateDelay;    // the gate delay (s)
};

class QscatSes
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

    int  CmdTxPulseWidthDn(unsigned char tx_pulse_width_dn);
    int  CmdPriDn(unsigned char pri_dn);
    int  CmdRxGateWidthDn(int beam_idx, unsigned char rx_gate_width_dn);
    int  CmdRxGateDelayDn(int beam_idx, unsigned char rx_gate_delay_dn);
    int  CmdTxDopplerDn(short tx_doppler_dn);


    //-----------//
    // variables //
    //-----------//

    float        txPulseWidth;    // the transmit pulse width (s)
    float        txDoppler;       // the Doppler frequency shift (Hz)
    float        txFrequency;     // the transmit frequency (Hz)

    float        baseTxFrequency; // the base transmitter frequency (Hz)
    float        pri;             // the pulse repetition interval (s)
    float        transmitPower;   // the transmit power (W)
    float        rxGainEcho;      // dimensionless multiplicative factor
    float        rxGainNoise;     // dimensionless multiplicative factor
    float        chirpRate;       // chirp rate (Hz/s)
    float        chirpStartM;     // chirp start slope (Hz/s)
    float        chirpStartB;     // chirp start offset (Hz)
    float        scienceSliceBandwidth;  // Hz
    int          scienceSlicesPerSpot;   // count
    float        guardSliceBandwidth;    // Hz
    int          guardSlicesPerSide;     // count
    float        noiseBandwidth;         // Hz

    SesBeamInfo  beamInfo[NUMBER_OF_QSCAT_BEAMS];
};

//======================================================================
// CLASS
//      QscatSas
//
// DESCRIPTION
//      The QscatSas acts like the QSCAT SAS.
//======================================================================

#define SAS_LOW_SPIN_RATE   18.0
#define SAS_HIGH_SPIN_RATE  19.8

enum EncoderE { ENCODER_A, ENCODER_B };
enum SpinRateE { LOW_SPIN_RATE, HIGH_SPIN_RATE };

class QscatSas
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
    int             ApplyAzimuthShift(double sample_delay);
    unsigned short  GetEncoder();

    int  CmdSpinRate(SpinRateE spin_rate);

    //-----------//
    // variables //
    //-----------//

    Antenna   antenna;
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

#define DOPPLER_ORBIT_STEPS  256
#define ENCODER_A_OFFSET     16408
#define ENCODER_B_OFFSET     28
#define BEAM_A_OFFSET        0
#define BEAM_B_OFFSET        0

class CdsBeamInfo
{
public:
    CdsBeamInfo();
    ~CdsBeamInfo();

    RangeTracker    rangeTracker;
    DopplerTracker  dopplerTracker;

    unsigned char   rxGateWidthDn;
};

class QscatCds
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

    int           SetTime(double new_time);
    int           SetEqxTime(double eqx_time);
    int           SetTimeWithInstrumentTime(unsigned int ticks);

    double        OrbitFraction();
    CdsBeamInfo*  GetCurrentBeamInfo();

    int  LoadRgc(int beam_idx, const char* file);
    int  LoadDtc(int beam_idx, const char* file);

    int  CmdTxPulseWidthEu(float tx_pulse_width, QscatSes* qscat_ses);
    int  CmdPriEu(float pri, QscatSes* qscat_ses);
    int  CmdRxGateWidthEu(int beam_idx, float rx_gate_width,
             QscatSes* qscat_ses);
    int  CmdSpinRate(SpinRateE spin_rate, QscatSas* qscat_sas);
    int  CmdOrbitTicksPerOrbit(unsigned int orbit_ticks);
    int  CmdRangeAndDoppler(QscatSas* qscat_sas, QscatSes* qscat_ses);

    //-----------//
    // variables //
    //-----------//

    unsigned char  priDn;
    unsigned char  txPulseWidthDn;
    SpinRateE      spinRate;
    int            useTracking;
    unsigned int   orbitTicksPerOrbit;
    CdsBeamInfo    beamInfo[NUMBER_OF_QSCAT_BEAMS];

    int            currentBeamIdx;
    unsigned int   orbitTime;
    unsigned int   instrumentTime;

    double         time;
    double         eqxTime;
};

//======================================================================
// CLASS
//      Qscat
//
// DESCRIPTION
//      The Qscat is the top level object for the QSCAT instrument.
//======================================================================

class Qscat
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

    Beam*         GetCurrentBeam();
    CdsBeamInfo*  GetCurrentCdsBeamInfo();
    SesBeamInfo*  GetCurrentSesBeamInfo();

    //-----------//
    // variables //
    //-----------//

    QscatCds  cds;
    QscatSas  sas;
    QscatSes  ses;

    float     systemLoss;           // dimensionless multiplicative factor
    float     systemTemperature;    // K
};

#endif
