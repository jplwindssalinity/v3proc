//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.5   22 May 1998 16:22:10   sally
// added cds error message tables
// 
//    Rev 1.4   19 May 1998 14:42:50   sally
// add type 1 error messages
// 
//    Rev 1.3   04 May 1998 17:20:04   sally
// added setup HK2 Limits
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
const char *twt_map[] = {"On", "Off", "???"};
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

const char *type1_error_msg_map[] =
{
    "No Error",                                             // 0
    "Error Mess Ring Create Error",                         // 1
    "Too Many CDS Actions",                                 // 2
    "Invalid CDS Action ID",                                // 3
    "Invalid STLM Req",                                     // 4
    "Unexpected Major Frame",                               // 5
    "Invalid STLM Size",                                    // 6
    "Missing Serial Mag Cmd Words",                         // 7
    "Bad Serial Msg Cmd Checksum",                          // 8
    "Bad Serial Msg ID Const",                              // 9
    "Bad Serial Msg ID Arg",                                // 10
    "Bad Active Default Int Standby PRF Table",             // 11
    "Bad Inactive Default Int Standby PRF Table",           // 12
    "Bad Active Default Int WOBS PRF Table",                // 13
    "Bad Inactive Default Int WOBS PRF Table",              // 14
    "Bad Active Default Int Cal PRF Table",                 // 15
    "Bad Inactive Default Int Cal PRF Table",               // 16
    "Bad Active Default Int RO PRF Table",                  // 17
    "Bad Inactive Default Int RO PRF Table",                // 18
    "SES Cmd Byte Count Over Max of 24",                    // 19
    "Inavlid MTLM Entry",                                   // 20
    "MTLM PRF Count Too Low",                               // 21
    "MTLM PRF Count Too High",                              // 22
    "MTLM SES MRO Count Trunc",                             // 23
    "MTLM Ill CDS MRO Count",                               // 24
    "MTLM Ill SES MRO Count",                               // 25
    "MTLM CDS MRO Count Trunc",                             // 26
    "MTLM PRF Table Exhausted",                             // 27
    "MTLM Illegal SES MRO Address",                         // 28
    "MTLM Illegal CDS MRO Address",                         // 29
    "MTLM Dupl Tx Commands",                                // 30
    "MTLM Tx Command Not First",                            // 31
    "MTLM SES Status Req In Table",                         // 32
    "MTLM SES Byte Count Exceeded",                         // 33
    "MTLM SES Command Count Exceeded",                      // 34
    "MTLM Too Many CDS Actions",                            // 35
    "MTLM No Entry Terminator",                             // 36
    "MTLM No Room For C1",                                  // 37
    "MTLM No Room For C2",                                  // 38
    "MTLM No Room For C3",                                  // 39
    "MTLM Not All of Table Used",                           // 40
    "MTLM Sci Pkt Too Large",                               // 41
    "MTLM Op Pkt Too Large",                                // 42
    "Standby PRF Table Unavailable",                        // 43
    "WOBS PRF Table Unavailable",                           // 44
    "Cal PRF Table Unavailable",                            // 45
    "RO PRF Table Unavailable",                             // 46
    "MTLM No Room for SES MRO",                             // 47
    "MTLM No Room for CDS MRO",                             // 48
    "MTLM Tx in Standby",                                   // 49
    "MTLM Missing Tx in Non-Standby",                       // 50
    "STLM Illegal Table Length",                            // 51
    "STLM Illegal Entry",                                   // 52
    "STLM Param Illegal Table Length",                      // 53
    "STLM Param Illegal Entry",                             // 54
    "Doppler Illegal Table Length",                         // 55
    "Range Illegal Table Length",                           // 56
    "Internal Table Type Invalid",                          // 57
    "STLM Eng Table Unavailable",                           // 58
    "STLM Status Table Unavailable",                        // 59
    "'A' Doppler Table Unavailable",                        // 60
    "'B' Doppler Table Unavailable",                        // 61
    "'A' Range Table Unavailable",                          // 62
    "'B' Range Table Unavailable",                          // 63
    "Standby SES Param Table Unavailable",                  // 64
    "Cal SES Param Table Unavailable",                      // 65
    "RO SES Param Table Unavailable",                       // 66
    "WOBS SES Param Table Unavailable",                     // 67
    "Bad Active Default STLM Status Table",                 // 68
    "Bad Inactive Default STLM Status Table",               // 69
    "Bad Active Default STLM Eng Table",                    // 70
    "Bad Inactive Default STLM Eng Table",                  // 71
    "Bad Active Default 'A' Doppler Table",                 // 72
    "Bad Inactive Default 'A' Doppler Table",               // 73
    "Bad Active Default 'B' Doppler Table",                 // 74
    "Bad Inactive Default 'B' Doppler Table",               // 75
    "Bad Active Default 'A' Range Table",                   // 76
    "Bad Inactive Default 'A' Range Table",                 // 77
    "Bad Active Default 'B' Range Table",                   // 78
    "Bad Inactive Default 'B' Range Table",                 // 79
    "Bad Active Default Standby SES Param Table,"           // 80
    "Bad Inactive Default Standby SES Param Table,"         // 81
    "Bad Active Default Cal SES Param Table,"               // 82
    "Bad Inactive Default Cal SES Param Table,"             // 83
    "Bad Active Default RO SES Param Table,"                // 84
    "Bad Inactive Default RO SES Param Table,"              // 85
    "Bad Active Default WOBS SES Param Table,"              // 86
    "Bad Inactive Default WOBS SES Param Table,"            // 87
    "Suppl Heater Off Relays Stuck",                        // 88
    "TWTA Off Relays Stuck",                                // 89
    "TWTA Power On Trip Fault",                             // 90
    "TWTA Monitor Trip Fault",                              // 91
    "Command Invalid CDS MRO Address",                      // 92
    "Command Invalid Table RO Type",                        // 93
    "TFG Science Packet Header Write",                      // 94
    "TFG Packet Status Write",                              // 95
    "TFG SES Error Flags Write",                            // 96
    "TFG was Busy at the GO",                               // 97
    "TFG was Busy at Header Write",                         // 98
    "MTLM No SES A Current",                                // 99
    "MTLM No SES B Current",                                // 100
    "MTLM No SAS A Current",                                // 101
    "MTLM No SAS B Current",                                // 102
    "MTLM No TWTA 1 Current",                               // 103
    "MTLM No TWTA 2 Current",                               // 104
    "All SES Param Tables Unavailable",                     // 105
    "All SES Param Illegal Table Length",                   // 106
    "All SES Param Illegal Entry",                          // 107
    "Bad Active Default All SES Param Table",               // 108
    "Bad Inactive Default All SES Param Table",             // 109
    "Multiple Sequential SES Data Loss",                    // 110
    "Multiple Sequential SAS Data Loss",                    // 111
    "Multiple SES Data Loss Fault",                         // 112
    "Multiple SAS Data Loss Fault",                         // 113
    "Missed Interrupt",                                     // 114
    "Missed Interrupt Fault",                               // 115
    "Command MRO LIM Timeout",                              // 116
    "Command Invalid CDS MRO LIM",                          // 117
    "MTLM SES RET Byte Count Exceeded",                     // 118
    "MTLM Consecutive SES MRO",                             // 119
    "Bad Cal PRF Tolerance",                                // 120
    "Patch Command Set Address Not in HIMEM",               // 121
    "Patch Command Bad Patch Set Length",                   // 122
    "Patch Command Patch Set Checksum Bad",                 // 123
    "Patch Cmd Patch Element Addr Not in Boundry of set",   // 124
    "Patch Cmd Bad Patch Element Length",                   // 125
    "Patch Cmd Patch Next Element Ptr Not in Boundry of set",// 126
    "Patch Command Wrong Pset Length",                      // 127
    "Patch Command Illegal Patch Type",                     // 128
    "Patch Command Out of Bound High Memory Address",       // 129
    "Patch Command Out of Bound In Line Memory Address",    // 130
    "Patch Command Prelim Bad Patch Set Length",            // 131
    "Patch Command Prelim Address Not in HIMEM",            // 132
    "Patch Command Prelim Bad Patch End Address",           // 133
    "Patch Command Prelim ID is Not Unique",                // 134
    "Patch Command Prelim Memory Address is Not Unique",    // 135
    "Patch Cmd Prelim Mem Addr is Overlapping Prev HIMEM Addr",// 136
    "Patch Cmd Critical Table Checksum in Progress",        // 137
    "Patch Cmd Critical Table Checksum Conversion Error",   // 138
    "Patch Cmd Internal Checksum Conversion Error",         // 139
    "Patch Cmd Verify Conversion Error",                    // 140
    "Patch Apply Patch Critical Checksum Mismatch",         // 141
    "Patch Apply Patch Set Checksum Mismatch",              // 142
    "Patch Enable ID does not Exist",                       // 143
    "Patch Disable ID does not Exist",                      // 144
    "Missed Time 0",                                        // 145
    "Serial Mag Buffer Overflow",                           // 146
    "Level 6 INT Disable Failed",                           // 147
    "Level 5 INT Disable Failed",                           // 148
    "Level 4 INT Disable Failed",                           // 149
    "Task Spawn Error",                                     // 150
    "Already in Standby",                                   // 151
    "Already in Calibration",                               // 152
    "Already in Receive Only",                              // 153
    "Already in Wind Observation",                          // 154
    "Patched ID Already Enabled",                           // 155
    "Patched ID Already Disabled",                          // 156
    "POR Reset",                                            // 157
    "Watchdog Reset",                                       // 158
    "CDS Commanded Reset",                                  // 159
    "Hard Reset Due to Mult Soft",                          // 160
    "TWTA 33 Duty Cycle Violation",                         // 161
    "Pulse Busy",                                           // 162
    "VxWorks Trap",                                         // 163
    "Missed Time 1",                                        // 164
    "Missed Spacecraft Time",                               // 165
    "TWTA Low Drive Power",                                 // 166
    "TWTA Low Drive Power Fault",                           // 167
    "MTLM No TWTA 1 Drive Power",                           // 168
    "MTLM No TWTA 2 Drive Power"                            // 169
};
const int Type1ErrMsgMapSize = ElementNumber(type1_error_msg_map);

const char *type2_error_msg_map[] =
{
    "Critical Variable Voting Failed",                      // 0
    "Critical Varaible Only 2 out of 3 OK",                 // 1
    "Illegal Value for Critical Variable",                  // 2
    "Critical Variables Incompatible",                      // 3
    "Table Checksum Bad",                                   // 4
    "SAS Bad Antenna Delta",                                // 5
    "A2D was busy",                                         // 6
    "Equator Crossing Missed",                              // 7
    "PBI Bad State",                                        // 8
    "PBI Time Stamp Message Error",                         // 9
    "PBI Housekeeping TLM Message Error",                   // 10
    "PBI Command Message Error",                            // 11
    "PBI Bad Memory Write",                                 // 12
    "PBI R2 MOD",                                           // 13
    "PBI BCRTF",                                            // 14
    "PBI WP Viltn",                                         // 15
    "PBI WD Xpird",                                         // 16
    "PBI HK Timeout"                                        // 17
};
const int Type2ErrMsgMapSize = ElementNumber(type2_error_msg_map);

const char *cds_fsw_crit_var_obj_id[] =
{
    "Soft Reset Flag",                                      // 0
    "Operational Mode",                                     // 1
    "Packet Count",                                         // 2
    "Instrument Time",                                      // 3
    "CDS Count",                                            // 4
    "Prev Reset",                                           // 5
    "Active Ext Standby PRF Table Ptr",                     // 6
    "Active Ext Wind Obs PRF Table Ptr",                    // 7
    "Active Ext Cal PRF Table Ptr",                         // 8
    "Active Ext RO PRF Table Ptr",                          // 9
    "Inactive Ext Standby PRF Table Ptr",                   // 10
    "Inactive Ext Wind Obs PRF Table Ptr",                  // 11
    "Inactive Ext Cal PRF Table Ptr",                       // 12
    "Inactive Ext RO PRF Table Ptr",                        // 13
    "Active Int Standby PRF Table Ptr",                     // 14
    "Active Int Wind Obs PRF Table Ptr",                    // 15
    "Active Int Cal PRF Table Ptr",                         // 16
    "Active Int RO PRF Table Ptr",                          // 17
    "Inactive Int Standby PRF Table Ptr",                   // 18
    "Inactive Int Wind Obs PRF Table Ptr",                  // 19
    "Inactive Int Cal PRF Table Ptr",                       // 20
    "Inactive Int RO PRF Table Ptr",                        // 21
    "Active Int PRF Table Ptr",                             // 22
    "Inactive Int PRF Table Ptr",                           // 23
    "Active A Doppler Table Ptr",                           // 24
    "Inactive A Doppler Table Ptr",                         // 25
    "Active B Doppler Table Ptr",                           // 26
    "Inactive B Doppler Table Ptr",                         // 27
    "F Active A Doppler Table Ptr",                         // 28
    "F Inactive A Doppler Table Ptr",                       // 29
    "F Active B Doppler Table Ptr",                         // 30
    "F Inactive B Doppler Table Ptr",                       // 31
    "Active A Range Table Ptr",                             // 32
    "Inactive A Range Table Ptr",                           // 33
    "Active B Range Table Ptr",                             // 34
    "Inactive B Range Table Ptr",                           // 35
    "Active All SES Param Table Ptr",                       // 36
    "Active Standby Param Table Ptr",                       // 37
    "Active Wind Obs Param Table Ptr",                      // 38
    "Active Cal Param Table Ptr",                           // 39
    "Active RO Param Table Ptr",                            // 40
    "Inactive All SES Param Table Ptr",                     // 41
    "Inactive Standby Param Table Ptr",                     // 42
    "Inactive Winds Obs Param Table Ptr",                   // 43
    "Inactive Cal Param Table Ptr",                         // 44
    "Inactive RO Param Table Ptr",                          // 45
    "Active SES Param Table Ptr",                           // 46
    "Inactive SES Param Table Ptr",                         // 47
    "Active Serial Status Table Ptr",                       // 48
    "Inactive Serial Status Table Ptr",                     // 49
    "Active Serial Eng Table Ptr",                          // 50
    "Inactive Serial Eng Table Ptr",                        // 51
    "An Crossing Instrument Time",                          // 52
    "Orbit Timer Ticks",                                    // 53
    "Cal Pulse Period",                                     // 54
    "Instrument Time Sync Interval",                        // 55
    "TWTA Trip Monitor Enable",                             // 56
    "TWTA Trip Monitor Timeout Period",                     // 57
    "Supp Heater Mode Control Enable",                      // 58
    "Multi SES Data Loss Resp Enable",                      // 59
    "Packet SES Cyclical MRO Address",                      // 60
    "Packet CDS Cyclical MRO Address",                      // 61
    "New Command CDS MRO Start Address",                    // 62
    "Command CDS MRO Start Address",                        // 63
    "New Command CDS MRO Upper Limit",                      // 64
    "New Command CDS MRO Lower Limit",                      // 65
    "CDS MRO Upper Limit",                                  // 66
    "CDS MRO Lower Limit",                                  // 67
    "New Command Table RO Start Type",                      // 68
    "Command Table RO Start Type",                          // 69
    "Convert Standby PRF Table",                            // 70
    "Convert Cal PRF Table",                                // 71
    "Convert RO PRF Table",                                 // 72
    "Convert Wind Obs PRF Table",                           // 73
    "Convert A Doppler Table",                              // 74
    "Convert B Doppler Table",                              // 75
    "Convert A Range Table",                                // 76
    "Convert B Range Table",                                // 77
    "Convert All SES Param Table",                          // 78
    "Convert Standby SES Param Table",                      // 79
    "Convert Cal SES Param Table",                          // 80
    "Convert RO SES Param Table",                           // 81
    "Convert Wind Obs SES Param Table",                     // 82
    "Convert STLM Status Table",                            // 83
    "Convert STLM Eng Table",                               // 84
    "Multi SAS Data Loss Resp Enable",                      // 85
    "Command Hist Temp",                                    // 86
    "Command Hist Queue Count",                             // 87
    "Init Table RO",                                        // 88
    "Error Message History",                                // 89
    "Error Message History Queue Count",                    // 90
    "Old Doppler Orbit Step",                               // 91
    "Doppler Second Step",                                  // 92
    "End of Doppler and No A N",                            // 93
    "Last A Range Delay",                                   // 94
    "Last B Range Delay",                                   // 95
    "Table Bytes Written",                                  // 96
    "Table Readout Ptr",                                    // 97
    "Table Ptr",                                            // 98
    "Next Table",                                           // 99
    "Error Message",                                        // 100
    "Error Flags",                                          // 101
    "Running Error Count",                                  // 102
    "Change Flags",                                         // 103
    "Valid Command Count",                                  // 104
    "Invalid Command Count",                                // 105
    "SAS A Offset",                                         // 106
    "SAS B Offset",                                         // 107
    "Beam A Offset",                                        // 108
    "Beam B Offset",                                        // 109
    "Reset Reload Period",                                  // 110
    "TWTA Trip",                                            // 111
    "TWTA Trip Monitor Counter",                            // 112
    "TWTA Trip Fault",                                      // 113
    "Got Low TWTA Drive Power",                             // 114
    "TWTA Low Drive Power Check Enabled",                   // 115
    "TWTA Low Drive Power Limit",                           // 116
    "Low TWTA Drive Power Expired",                         // 117
    "Orbital Counter"                                       // 118
};
const int CdsFswCritVarObjIdSize = ElementNumber(cds_fsw_crit_var_obj_id);

const char *cds_fsw_obj_id[] =
{
    "Active Ext Standby PRF Table ID",                      // 0
    "Active Ext Wind Obs PRF Table ID",                     // 1
    "Active Ext Cal PRF Table ID",                          // 2
    "Active Ext RO PRF Table ID",                           // 3
    "Active All SES Param Table ID",                        // 4
    "Active Standby Param Table ID",                        // 5
    "Active Wind Obs Param Table ID",                       // 6
    "Active Cal Param Table ID",                            // 7
    "Active RO Param Table ID",                             // 8
    "Active Serial Eng Table ID",                           // 9
    "Active Serial Status Table ID",                        // 10
    "Active A Doppler Table ID",                            // 11
    "Active B Doppler Table ID",                            // 12
    "Active A Range Table ID",                              // 13
    "Active B Range Table ID",                              // 14
    "CDS Const Table ID",                                   // 15
    "Inactive Ext Standby PRF Table ID",                    // 16
    "Inactive Ext Wind Obs PRF Table ID",                   // 17
    "Inactive Ext Cal PRF Table ID",                        // 18
    "Inactive Ext RO PRF Table ID",                         // 19
    "Inactive All SES Param Table ID",                      // 20
    "Inactive Ext Standby Param Table ID",                  // 21
    "Inactive Ext Wind Obs Param Table ID",                 // 22
    "Inactive Ext Cal Param Table ID",                      // 23
    "Inactive Ext RO Param Table ID",                       // 24
    "Inactive Serial Eng Table ID",                         // 25
    "Inactive Serial Status Table ID",                      // 26
    "Inactive A Doppler Table ID",                          // 27
    "Inactive B Doppler Table ID",                          // 28
    "Inactive A Range Table ID",                            // 29
    "Inactive B Range Table ID"                             // 30
};
const int CdsFswObjIdSize = ElementNumber(cds_fsw_obj_id);

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
