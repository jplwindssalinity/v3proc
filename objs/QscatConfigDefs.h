//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef QSCATCONFIGDEFS_H
#define QSCATCONFIGDEFS_H

static const char rcs_id_qscatconfigdefs_h[] =
	"@(#) $Id$";

//----------//
// QscatSes //
//----------//

#define BASE_TX_FREQUENCY_KEYWORD  "BASE_TX_FREQUENCY"
#define RX_GAIN_ECHO_KEYWORD       "RX_GAIN_ECHO"
#define RX_GAIN_NOISE_KEYWORD      "RX_GAIN_NOISE"

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

#endif
