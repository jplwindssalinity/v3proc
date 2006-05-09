//==============================================================//
// Copyright (C) 1998-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef OVWMCONFIGDEFS_H
#define OVWMCONFIGDEFS_H

static const char rcs_id_ovwmconfigdefs_h[] =
    "@(#) $Id$";

//----------//
// OvwmSim //
//----------//

#define EPOCH_TIME_STRING_KEYWORD  "EPOCH_TIME_STRING"
#define USE_HIGH_RES_SIM_KEYWORD  "USE_HIGH_RES_SIM"
#define INTEGRATION_STEP_SIZE_KEYWORD "INTEGRATION_STEP_SIZE"

//----------//
// OvwmSes //
//----------//

#define RECEIVE_PATH_LOSS_KEYWORD  "RECEIVE_PATH_LOSS"
#define TRANSMIT_PATH_LOSS_KEYWORD  "TRANSMIT_PATH_LOSS"
#define NUM_PULSES_KEYWORD          "NUM_PULSES"
#define BASE_TRANSMIT_FREQUENCY_KEYWORD  "BASE_TX_FREQUENCY"
#define RX_GAIN_ECHO_KEYWORD             "RX_GAIN_ECHO"
#define RX_GAIN_NOISE_KEYWORD            "RX_GAIN_NOISE"
#define CALIBRATION_BIAS_KEYWORD         "CALIBRATION_BIAS"

#define BURST_REPETITION_INTERVAL_KEYWORD "BRI"
#define MAX_RANGE_WIDTH_KEYWORD  "MAX_RANGE_WIDTH"
#define NUM_RANGE_LOOKS_AVERAGED_KEYWORD "NUM_RANGE_LOOKS_AVERAGED"
#define NUM_TWTAS_KEYWORD "NUM_TWTAS"
#define NUM_RECEIVERS_KEYWORD "NUM_RECEIVERS"
#define BEAM_x_TX_DELAY_KEYWORD "BEAM_x_TX_DELAY"
#define LANDSIGMA0_BEAM_x_KEYWORD "BEAM_x_LAND_SIGMA0"
#define USE_REAL_APERTURE_KEYWORD "USE_REAL_APERTURE"

//----------//
// OvwmSas //
//----------//


//----------//
// OvwmCds //
//----------//

#define TX_PULSE_WIDTH_KEYWORD     "TX_PULSE_WIDTH"
#define PRI_KEYWORD                "PRI"


#define ANTENNA_SPIN_RATE_KEYWORD  "ANTENNA_SPIN_RATE"


#define BEAM_x_RX_GATE_WIDTH_KEYWORD  "BEAM_x_RX_GATE_WIDTH"
#define BEAM_x_RGC_FILE_KEYWORD       "BEAM_x_RGC_FILE"
#define BEAM_x_DTC_FILE_KEYWORD       "BEAM_x_DTC_FILE"

#define USE_RGC_KEYWORD    "USE_RGC"
#define USE_DTC_KEYWORD    "USE_DTC"

#define TRACKING_CHIRP_RATE_KEYWORD  "TRACKING_CHIRP_RATE"


//------//
// Land //
//------//

#define SIM_LAND_FLAG_KEYWORD          "SIM_LAND_FLAG"
#define USE_SIGMA0_MAPS_KEYWORD        "USE_SIGMA0_MAPS"
#define INNER_BEAM_SIGMA0_MAP_KEYWORD  "INNER_BEAM_SIGMA0_MAP"
#define OUTER_BEAM_SIGMA0_MAP_KEYWORD  "OUTER_BEAM_SIGMA0_MAP"

//--------------------------//
// Latitude Bounds          //
//--------------------------//
#define SIM_LAT_MIN_KEYWORD  "SIM_LAT_MIN"
#define SIM_LAT_MAX_KEYWORD  "SIM_LAT_MAX"
#define SIM_LON_MIN_KEYWORD  "SIM_LON_MIN"
#define SIM_LON_MAX_KEYWORD  "SIM_LON_MAX"

//-----------//
// Tables    //
//-----------//
#define AMBIG_TABLE_FILE_KEYWORD "AMBIG_TABLE_FILE"
#define AMBIG_INDEX_FILE_KEYWORD "AMBIG_INDEX_FILE"

#define PTRESPONSE_TABLE_AUX_FILE_BEAM_x_KEYWORD "PTR_TABLE_AUX_FILE_BEAM_x"
#define PTRESPONSE_TABLE_DATA_FILE_BEAM_x_KEYWORD "PTR_TABLE_DATA_FILE_BEAM_x"
#endif
