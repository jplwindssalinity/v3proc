//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.2   27 Mar 1998 09:58:24   sally
// added L1A Derived data
// 
//    Rev 1.1   23 Feb 1998 10:27:58   sally
// add limit checker
// 
//    Rev 1.0   04 Feb 1998 14:15:02   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:58  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

const char rcs_id_CommonDefs_C[] =
    "@(#) $Header$";

#include "Parameter.h"
#include "CommonDefs.h"

//---------------------------
// Level 0 Quality Flag Maps 
//---------------------------

const char *crc_correct_map[] = {"OK", "Fix"};
const char *sc_time_err_map[] = {"OK", "Err"};
const char *att_correct_map[] = {"OK", "Held", "Zeroed"};

//---------------------------
// Level 1 Quality Flag Maps 
//---------------------------

const char *miss_ephem_map[] = {"OK", "Err"};
const char *pred_ephem_used_map[] = {"ELMD", "ELMP"};
const char *analog_cur_map[] = {"OK", "Old"};

//------------------------
// Instrument Status Maps 
//------------------------

const char *twta_map[] = {"#1", "#2", "??"};
const char *hvps_map[] = {"On", "Off", "???"};
const char *dss_map[] = {"A", "B", "?"};
const char *twta_trip_override_map[] = {"Dis", "En"};
const char *cmf_map[] = {"Sci", "Cal"};
const char *mode_map[] = {"WOM", "CBM", "SBM", "ROM", "Inv",
    "Inv", "Inv", "Inv"};
const char *ext_mode_map[] = {"WOM", "CBM", "SBM", "ROM", "RHM",
    "Inv", "Inv", "Inv"};
const char *unk_mode_map[] = {"WOM", "CBM", "SBM", "ROM", "RHM",
    "Inv", "???"};
const char *hvps_shut_en_map[] = {"Dis", "En"};
const char *twt_mon_en_map[] = {"Dis", "En"};
const char *bc_dump_map[] = {"Mem", "BC"};
const char *cur_beam_map[] = {"3V", "1V", "5V", "2H",
    "6V", "4V", "5H", "2V"};
const char *rx_pro_map[] = {"Off", "On"};

//------------------------
// Error Engineering Maps 
//------------------------

const char *error_msg_map[] = {
    "OK",
    "Unused",
    "Divide by Zero",
    "Interrupt Error",
    "Checksum Error-Binning (RAM)",
    "Checksum Error-Uplink",
    "Checksum Error-Cosine",
    "Checksum Error-Startup/NOP",
    "Checksum Error-Program",
    "Checksum Error-Binning (PROM)",
    "Read/Write Memory Error",
    "Rejected WTS Command",
    "Unexpected EqX",
    "Missed EqX",
    "Mode Condition Error",
    "Invalid Ground Command",
    "Flag 3 - Read Error - OK",
    "Flag 3 - Read Error - Bad",
    "Var 3 - Read Error - OK",
    "Var 3 - Read Error - Bad",
    "Compute Parameter Error - OK",
    "Compute Parameter Error - Bad",
    "Watchdog Timeout",
    "Data Uplink Timeout",
    "S/C-NSCAT Link Busy",
    "NSCAT-S/C Link Busy",
    "Synchronization Error",
    "POR",
    "Mode Agreement Error",
    "S/W Initiated Restart",
    "Bad Address Reset",
    "TWT Monitor Detected Trip"
};
const char *lack_start_reqs_map[] = {"OK", "Err"};
const char *err_queue_full_map[] = {"OK", "Full"};
const char *bin_param_err_map[] = {"OK", "Err"};
const char *def_bin_const_map[] = {"Uplnk", "Def"};
const char *twta_trip_map[] = {"OK", "Trip"};
const char *lock_map[] = {"Lock", "Unlock"};
const char *hvps_backup_off_map[] = {"OK", "Err"};

//-------------------------
// Antenna Deployment Maps 
//-------------------------

const char *ant_deploy_map[] = {"Undep", "Dep"};
const char *relay_map[] = {"Set", "Reset"};
const char *wts_1_map[] = {"TWTA 1", "Not TWTA 1"};
const char *wts_2_map[] = {"TWTA 2", "Not TWTA 2"};

//--------------------
// Relay derived maps 
//--------------------

const char *heater_map[] =
{
    "Enabled", "Disabled"
};

//-----------------------------
// HK2 Data Block Header Maps 
//-----------------------------

const char *data_type_map[] = {"Real Telemetry Data", "Stored Telemetry Data"};
const char *dwell_mode_map[] = {"Normal Mode", "Dwell Mode", "Dwell Two Mode"};

//------------
// Relay Maps 
//------------

const unsigned char k1_k2_k3_map[2][2][2] =
{ // all
    { // k1 set
        { // k2 set
            ELECTRONICS_OFF,    // k1 set, k2 set, k3 set
            ELECTRONICS_OFF     // k1 set, k2 set, k3 rst
        },
        { // k2 reset
            ELECTRONICS_OFF,    // k1 set, k2 rst, k3 set
            ELECTRONICS_ON      // k1 set, k2 rst, k3 rst
        }
    },
    { // k1 reset
        { // k2 set
            ELECTRONICS_OFF,    // k1 rst, k2 set, k3 set
            ELECTRONICS_ON      // k1 rst, k2 set, k3 rst
        },
        { // k2 reset
            ELECTRONICS_ON,     // k1 rst, k2 rst, k3 set
            ELECTRONICS_ON      // k1 rst, k2 rst, k3 rst
        }
    }
};

const unsigned char k4_k5_k6_map[2][2][2] =
{ // all
    { // k4 set
        { // k5 set
            REPLACEMENT_HEATER_DISABLED,    // k4 set, k5 set, k6 set
            REPLACEMENT_HEATER_DISABLED     // k4 set, k5 set, k6 rst
        },
        { // k5 reset
            REPLACEMENT_HEATER_DISABLED,    // k4 set, k5 rst, k6 set
            REPLACEMENT_HEATER_ENABLED      // k4 set, k5 rst, k6 rst
        }
    },
    { // k4 reset
        { // k5 set
            REPLACEMENT_HEATER_DISABLED,    // k4 rst, k5 set, k6 set
            REPLACEMENT_HEATER_ENABLED      // k4 rst, k5 set, k6 rst
        },
        { // k5 reset
            REPLACEMENT_HEATER_ENABLED,     // k4 rst, k5 rst, k6 set
            REPLACEMENT_HEATER_ENABLED      // k4 rst, k5 rst, k6 rst
        }
    }
};

const unsigned char k7_k8_map[2][2] =
{ // all
    { // k7 set
        DSS_A,  // k7 set, k8 set
        DSS_B   // k7 set, k8 rst
    },
    { // k7 reset
        DSS_B,  // k7 rst, k8 set
        DSS_A   // k7 rst, k8 rst
    }
};

const unsigned char k9_k10_map[2][2] =
{ // all
    { // k9 set
        TWTA_1, // k9 set, k10 set
        TWTA_2  // k9 set, k10 rst
    },
    { // k9 reset
        TWTA_2, // k9 rst, k10 set
        TWTA_1  // k9 rst, k10 rst
    }
};

const unsigned char k11_k12_map[2][2] =
{ // all
    { // k11 set
        HVPS_ON,    // k11 set, k12 set
        HVPS_OFF    // k11 set, k12 rst
    },
    { // k11 reset
        HVPS_OFF,   // k11 rst, k12 set
        HVPS_ON     // k11 rst, k12 rst
    }
};

const unsigned char k13_k14_map[2][2] =
{ // all
    { // k13 set
        SPARE_HEATER_ENABLED,   // k13 set, k14 set
        SPARE_HEATER_DISABLED   // k13 set, k14 rst
    },
    { // k13 reset
        SPARE_HEATER_DISABLED,  // k13 rst, k14 set
        SPARE_HEATER_ENABLED    // k13 rst, k14 rst
    }
};

#define WTS_BOTH_ERROR 2
#define WTS_NEITHER_ERROR 3

const unsigned char wts1_wts2_map[2][2] =
{ // all
    { // wts1=0
        WTS_BOTH_ERROR,     // wts1=0, wts2=0
        TWTA_1              // wts1=0, wts2=1
    },
    { // wts1 = 1
        TWTA_2,             // wts1=1, wts2=0
        WTS_NEITHER_ERROR   // wts1=1, wts2=1
    }
};
