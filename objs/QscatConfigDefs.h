//==============================================================//
// Copyright (C) 1998-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef QSCATCONFIGDEFS_H
#define QSCATCONFIGDEFS_H

static const char rcs_id_qscatconfigdefs_h[] =
	"@(#) $Id$";

//----------//
// QscatSim //
//----------//

#define EPOCH_TIME_STRING_KEYWORD  "EPOCH_TIME_STRING"

//----------//
// QscatSes //
//----------//

#define FFT_BIN_BANDWIDTH_KEYWORD  "FFT_BIN_BANDWIDTH"

#define BASE_TRANSMIT_FREQUENCY_KEYWORD  "BASE_TX_FREQUENCY"
#define RX_GAIN_ECHO_KEYWORD             "RX_GAIN_ECHO"
#define RX_GAIN_NOISE_KEYWORD            "RX_GAIN_NOISE"
#define CALIBRATION_BIAS_KEYWORD         "CALIBRATION_BIAS"

#define A0_L13_KEYWORD             "A0_L13"
#define A1_L13_KEYWORD             "A1_L13"
#define A2_L13_KEYWORD             "A2_L13"
#define A3_L13_KEYWORD             "A3_L13"
#define A4_L13_KEYWORD             "A4_L13"

#define A0_L21_KEYWORD             "A0_L21"
#define A1_L21_KEYWORD             "A1_L21"
#define A2_L21_KEYWORD             "A2_L21"
#define A3_L21_KEYWORD             "A3_L21"
#define A4_L21_KEYWORD             "A4_L21"

#define A0_L23_KEYWORD             "A0_L23"
#define A1_L23_KEYWORD             "A1_L23"
#define A2_L23_KEYWORD             "A2_L23"
#define A3_L23_KEYWORD             "A3_L23"
#define A4_L23_KEYWORD             "A4_L23"

#define A0_LCALOP_KEYWORD             "A0_LCALOP"
#define A1_LCALOP_KEYWORD             "A1_LCALOP"
#define A2_LCALOP_KEYWORD             "A2_LCALOP"
#define A3_LCALOP_KEYWORD             "A3_LCALOP"
#define A4_LCALOP_KEYWORD             "A4_LCALOP"

#define PHYSICAL_TEMPERATURE_KEYWORD  "PHYSICAL_TEMPERATURE"

//----------//
// QscatCds //
//----------//

#define TX_PULSE_WIDTH_KEYWORD     "TX_PULSE_WIDTH"
#define PRI_KEYWORD                "PRI"
#define ANTENNA_SPIN_RATE_KEYWORD  "ANTENNA_SPIN_RATE"

#define BEAM_x_RX_GATE_WIDTH_KEYWORD  "BEAM_x_RX_GATE_WIDTH"
#define BEAM_x_RGC_FILE_KEYWORD       "BEAM_x_RGC_FILE"
#define BEAM_x_DTC_FILE_KEYWORD       "BEAM_x_DTC_FILE"

#define USE_RGC_KEYWORD    "USE_RGC"
#define USE_DTC_KEYWORD    "USE_DTC"

#define USE_BYU_DOPPLER_KEYWORD  "USE_BYU_DOPPLER"
#define USE_BYU_RANGE_KEYWORD  "USE_BYU_RANGE"

#define USE_SPECTRAL_DOPPLER_KEYWORD  "USE_SPECTRAL_DOPPLER"
#define USE_SPECTRAL_RANGE_KEYWORD  "USE_SPECTRAL_RANGE"

#define BYU_INNER_BEAM_LOOK_ANGLE_KEYWORD "BYU_INNER_BEAM_LOOK_ANGLE"
#define BYU_INNER_BEAM_AZIMUTH_ANGLE_KEYWORD "BYU_INNER_BEAM_AZIMUTH_ANGLE"
#define BYU_OUTER_BEAM_LOOK_ANGLE_KEYWORD "BYU_OUTER_BEAM_LOOK_ANGLE"
#define BYU_OUTER_BEAM_AZIMUTH_ANGLE_KEYWORD "BYU_OUTER_BEAM_AZIMUTH_ANGLE"

//------------//
// Sigma0Maps //
//------------//

#define USE_SIGMA0_MAPS_KEYWORD        "USE_SIGMA0_MAPS"
#define INNER_BEAM_SIGMA0_MAP_KEYWORD  "INNER_BEAM_SIGMA0_MAP"
#define OUTER_BEAM_SIGMA0_MAP_KEYWORD  "OUTER_BEAM_SIGMA0_MAP"

#endif
