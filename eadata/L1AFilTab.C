//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.5   04 May 1998 10:52:46   sally
// added HK2 filters
// 
//    Rev 1.4   01 May 1998 16:45:16   sally
// add some more filters, and changed some names, per Lee Poulsen
// 
//    Rev 1.3   24 Mar 1998 15:57:04   sally
// de-warn for GNU
// 
//    Rev 1.2   24 Mar 1998 09:26:52   sally
// de-warn for GNU C++
// 
//    Rev 1.1   20 Feb 1998 10:58:06   sally
// L1 to L1A
// 
//    Rev 1.0   04 Feb 1998 14:15:52   daffer
// Initial checking
// Revision 1.3  1998/01/30 22:29:09  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

static const char rcs_id_L1AFilTab_C[] =
    "@(#) $Header$";

#include "Filter.h"
#include "FilterFunc.h"

const FilTabEntry L1AFilTab[] =
{
    { "sbm", "Standby Mode", Filter_Standby_Mode, 1,
        {
            { SOURCE_L1A, OPERATIONAL_MODE, UNIT_MAP }
        }
    },
    { "rom", "Receive Only Mode", Filter_Rx_Only_Mode, 1,
        {
            { SOURCE_L1A, OPERATIONAL_MODE, UNIT_MAP }
        }
    },
    { "cbm", "Calibration Mode", Filter_Calibration_Mode, 1,
        {
            { SOURCE_L1A, OPERATIONAL_MODE, UNIT_MAP }
        }
    },
    { "wom",   "Wind Observation Mode", Filter_Wind_Obs_Mode, 1,
        {
            { SOURCE_L1A, OPERATIONAL_MODE, UNIT_MAP }
        }
    },
    { "noncal", "Non-calibration Pulse Frame", Filter_NonCal_Frame, 1,
        {
            { SOURCE_L1A, TRUE_CAL_PULSE_POS, UNIT_DN }
        }
    },
    { "cal", "Calibration Pluse Frame", Filter_Cal_Frame, 1,
        {
            { SOURCE_L1A, TRUE_CAL_PULSE_POS, UNIT_DN }
        }
    },
    { "twtOn", "TWT On", Filter_TWT_On, 2,
        {
            { SOURCE_L1A, K9_TWTA_POWER, UNIT_MAP },
            { SOURCE_L1A, K10_TWTA_POWER, UNIT_MAP }
        }
    },
    { "twtOff", "TWT Off", Filter_TWT_Off, 2,
        {
            { SOURCE_L1A, K9_TWTA_POWER, UNIT_MAP },
            { SOURCE_L1A, K10_TWTA_POWER, UNIT_MAP }
        }
    },
    {"beamA", "Beam Select: Xmit A/Rx B", Filter_Beam_A, 1,
        {
            { SOURCE_L1A, DISCRETE_STATUS_1_00, UNIT_MAP }
        }
    },
    { "beamB", "Beam Select: Xmit B/Rx A", Filter_Beam_B, 1,
        {
            { SOURCE_L1A, DISCRETE_STATUS_1_00, UNIT_MAP }
        }
    },
    { "twta1", "TWTA 1", Filter_TWTA_1, 2,
        {
            { SOURCE_L1A, K11_TWTA_SELECT, UNIT_MAP },
            { SOURCE_L1A, K12_TWTA_SELECT, UNIT_MAP }
        }
    },
    { "twta2", "TWTA 2", Filter_TWTA_2, 2,
        {
            { SOURCE_L1A, K11_TWTA_SELECT, UNIT_MAP },
            { SOURCE_L1A, K12_TWTA_SELECT, UNIT_MAP }
        }
    },
    { "sasa", "SAS A", Filter_SAS_A, 2,
        {
            { SOURCE_L1A, K19_SAS_SELECT, UNIT_MAP },
            { SOURCE_L1A, K20_SAS_SELECT, UNIT_MAP }
        }
    },
    { "sasb", "SAS B", Filter_SAS_B, 2,
        {
            { SOURCE_L1A, K19_SAS_SELECT, UNIT_MAP },
            { SOURCE_L1A, K20_SAS_SELECT, UNIT_MAP }
        }
    },
    { "sesa", "SES A", Filter_SES_A, 2,
        {
            { SOURCE_L1A, K15_SES_SELECT, UNIT_MAP },
            { SOURCE_L1A, K16_SES_SELECT, UNIT_MAP }
        }
    },
    { "sesb", "SES B", Filter_SES_B, 2,
        {
            { SOURCE_L1A, K15_SES_SELECT, UNIT_MAP },
            { SOURCE_L1A, K16_SES_SELECT, UNIT_MAP }
        }
    },
    { "modOn", "Modulation On", Filter_Modulation_On, 1,
        {
            { SOURCE_L1A, SES_CONFIG_FLAGS_03, UNIT_MAP }
        }
    },
    { "modOff", "Modulation Off", Filter_Modulation_Off, 1,
        {
            { SOURCE_L1A, SES_CONFIG_FLAGS_03, UNIT_MAP }
        }
    },
    { "rxProtOn", "Receive Protect On", Filter_Rx_Protect_On, 1,
        {
            { SOURCE_L1A, DISCRETE_STATUS_2_03, UNIT_MAP }
        }
    },
    { "rxProtOff", "Receive Protect Off", Filter_Rx_Protect_Off, 1,
        {
            { SOURCE_L1A, DISCRETE_STATUS_2_03, UNIT_MAP }
        }
    },
    { "gridNorm", "Grid Normal", Filter_Grid_Normal, 1,
        {
            { SOURCE_L1A, SES_CONFIG_FLAGS_00, UNIT_MAP }
        }
    },
    { "gridDsbl", "Grid Disable", Filter_Grid_Dsbl, 1,
        {
            { SOURCE_L1A, SES_CONFIG_FLAGS_00, UNIT_MAP }
        }
    },
    { "sasAspin19_8", "SAS-A Spin Rate 19.8 RPM", Filter_SAS_A_Spin19_8, 1,
        {
            { SOURCE_L1A, SAS_A_SPIN_RATE, UNIT_MAP }
        }
    },
    { "sasAspin18_0", "SAS-A Spin Rate 18.0 RPM", Filter_SAS_A_Spin18_0, 1,
        {
            { SOURCE_L1A, SAS_A_SPIN_RATE, UNIT_MAP }
        }
    },
    { "sasBspin19_8", "SAS-B Spin Rate 19.8 RPM", Filter_SAS_B_Spin19_8, 1,
        {
            { SOURCE_L1A, SAS_B_SPIN_RATE, UNIT_MAP }
        }
    },
    { "sasBspin18_0", "SAS-B Spin Rate 18.0 RPM", Filter_SAS_B_Spin18_0, 1,
        {
            { SOURCE_L1A, SAS_B_SPIN_RATE, UNIT_MAP }
        }
    },
    { 0, 0, 0, 0,
        {
            { SOURCE_UNKNOWN, PARAM_UNKNOWN, UNIT_UNKNOWN }
        }
    }
};
