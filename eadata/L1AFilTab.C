//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
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
#include "L1AFilter.h"

const FilTabEntry L1AFilTab[] =
{
    { "sbm", "Standby Mode", L1AF_Standby_Mode, 1,
        {
            { SOURCE_L1A, OPERATIONAL_MODE, UNIT_MAP }
        }
    },
    { "rom", "Receive Only Mode", L1AF_Rx_Only_Mode, 1,
        {
            { SOURCE_L1A, OPERATIONAL_MODE, UNIT_MAP }
        }
    },
    { "cbm", "Calibration Mode", L1AF_Calibration_Mode, 1,
        {
            { SOURCE_L1A, OPERATIONAL_MODE, UNIT_MAP }
        }
    },
    { "wom",   "Wind Observation Mode", L1AF_Wind_Obs_Mode, 1,
        {
            { SOURCE_L1A, OPERATIONAL_MODE, UNIT_MAP }
        }
    },
    { "sci", "Science Frame", L1AF_Sci_Frame, 1,
        {
            { SOURCE_L1A, TRUE_CAL_PULSE_POS, UNIT_DN }
        }
    },
    { "cal", "Calibration Frame", L1AF_Cal_Frame, 1,
        {
            { SOURCE_L1A, TRUE_CAL_PULSE_POS, UNIT_DN }
        }
    },
    { "hvOn", "HVPS On", L1AF_HVPS_On, 1,
        {
            { SOURCE_L1A, L1A_FRAME_INST_STATUS_08, UNIT_MAP }
        }
    },
    { "hvOff", "HVPS Off", L1AF_HVPS_Off, 1,
        {
            { SOURCE_L1A, L1A_FRAME_INST_STATUS_08, UNIT_MAP }
        }
    },
    {"antA", "Xmit A/Rx B", L1AF_Ant_A, 1,
        {
            { SOURCE_L1A, DISCRETE_STATUS_1_00, UNIT_MAP }
        }
    },
    { "antB",  "Xmit B/Rx A", L1AF_Ant_B, 1,
        {
            { SOURCE_L1A, DISCRETE_STATUS_1_00, UNIT_MAP }
        }
    },
    { "twta1", "TWTA 1", L1AF_TWTA_1, 2,
        {
            { SOURCE_L1A, K11_TWTA_SELECT, UNIT_MAP },
            { SOURCE_L1A, K12_TWTA_SELECT, UNIT_MAP }
        }
    },
    { "twta2", "TWTA 2", L1AF_TWTA_2, 2,
        {
            { SOURCE_L1A, K11_TWTA_SELECT, UNIT_MAP },
            { SOURCE_L1A, K12_TWTA_SELECT, UNIT_MAP }
        }
    },
    { "sasa", "SAS A", L1AF_SAS_A, 2,
        {
            { SOURCE_L1A, K19_SAS_SELECT, UNIT_MAP },
            { SOURCE_L1A, K20_SAS_SELECT, UNIT_MAP }
        }
    },
    { "sasb", "SAS B", L1AF_SAS_B, 2,
        {
            { SOURCE_L1A, K19_SAS_SELECT, UNIT_MAP },
            { SOURCE_L1A, K20_SAS_SELECT, UNIT_MAP }
        }
    },
    { "sesa", "SES A", L1AF_SES_A, 2,
        {
            { SOURCE_L1A, K15_SES_SELECT, UNIT_MAP },
            { SOURCE_L1A, K16_SES_SELECT, UNIT_MAP }
        }
    },
    { "sesb", "SES B", L1AF_SES_B, 2,
        {
            { SOURCE_L1A, K15_SES_SELECT, UNIT_MAP },
            { SOURCE_L1A, K16_SES_SELECT, UNIT_MAP }
        }
    },
    { 0, 0, 0, 0,
        {
            { SOURCE_UNKNOWN, PARAM_UNKNOWN, UNIT_UNKNOWN }
        }
    }
};
