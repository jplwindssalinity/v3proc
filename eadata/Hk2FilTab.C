//=========================================================
// Copyright  (C)1996, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
// CM Log
// $Log$
// 
//    Rev 1.0   04 May 1998 10:54:16   sally
// Initial revision.
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

static const char rcs_id_HK2FilTab_C[] =
    "@(#) $Header$";

#include "Filter.h"
#include "FilterFunc.h"

const FilTabEntry HK2FilTab[] =
{
    { "sbm", "Standby Mode", Filter_Standby_Mode, 1,
        {
            { SOURCE_HK2, OPERATIONAL_MODE, UNIT_DN }
        }
    },
    { "rom", "Receive Only Mode", Filter_Rx_Only_Mode, 1,
        {
            { SOURCE_HK2, OPERATIONAL_MODE, UNIT_DN }
        }
    },
    { "cbm", "Calibration Mode", Filter_Calibration_Mode, 1,
        {
            { SOURCE_HK2, OPERATIONAL_MODE, UNIT_DN }
        }
    },
    { "wom",   "Wind Observation Mode", Filter_Wind_Obs_Mode, 1,
        {
            { SOURCE_HK2, OPERATIONAL_MODE, UNIT_DN }
        }
    },
    { "twtOn", "TWT On", Filter_TWT_On, 2,
        {
            { SOURCE_HK2, K9_TWTA_POWER, UNIT_MAP },
            { SOURCE_HK2, K10_TWTA_POWER, UNIT_MAP }
        }
    },
    { "twtOff", "TWT Off", Filter_TWT_Off, 2,
        {
            { SOURCE_HK2, K9_TWTA_POWER, UNIT_MAP },
            { SOURCE_HK2, K10_TWTA_POWER, UNIT_MAP }
        }
    },
    {"beamA", "Beam Select: Xmit A/Rx B", Filter_Beam_A, 1,
        {
            { SOURCE_HK2, BEAM_SELECT, UNIT_MAP }
        }
    },
    { "beamB", "Beam Select: Xmit B/Rx A", Filter_Beam_B, 1,
        {
            { SOURCE_HK2, BEAM_SELECT, UNIT_MAP }
        }
    },
    { "twta1", "TWTA 1", Filter_TWTA_1, 2,
        {
            { SOURCE_HK2, K11_TWTA_SELECT, UNIT_MAP },
            { SOURCE_HK2, K12_TWTA_SELECT, UNIT_MAP }
        }
    },
    { "twta2", "TWTA 2", Filter_TWTA_2, 2,
        {
            { SOURCE_HK2, K11_TWTA_SELECT, UNIT_MAP },
            { SOURCE_HK2, K12_TWTA_SELECT, UNIT_MAP }
        }
    },
    { "sasa", "SAS A", Filter_SAS_A, 2,
        {
            { SOURCE_HK2, K19_SAS_SELECT, UNIT_MAP },
            { SOURCE_HK2, K20_SAS_SELECT, UNIT_MAP }
        }
    },
    { "sasb", "SAS B", Filter_SAS_B, 2,
        {
            { SOURCE_HK2, K19_SAS_SELECT, UNIT_MAP },
            { SOURCE_HK2, K20_SAS_SELECT, UNIT_MAP }
        }
    },
    { "sesa", "SES A", Filter_SES_A, 2,
        {
            { SOURCE_HK2, K15_SES_SELECT, UNIT_MAP },
            { SOURCE_HK2, K16_SES_SELECT, UNIT_MAP }
        }
    },
    { "sesb", "SES B", Filter_SES_B, 2,
        {
            { SOURCE_HK2, K15_SES_SELECT, UNIT_MAP },
            { SOURCE_HK2, K16_SES_SELECT, UNIT_MAP }
        }
    },
    { "rxProtOn", "Receive Protect On", Filter_Rx_Protect_On, 1,
        {
            { SOURCE_HK2, SES_RX_PROTECT, UNIT_MAP }
        }
    },
    { "rxProtOff", "Receive Protect Off", Filter_Rx_Protect_Off, 1,
        {
            { SOURCE_HK2, SES_RX_PROTECT, UNIT_MAP }
        }
    },
    { "gridNorm", "Grid Normal", Filter_Grid_Normal, 1,
        {
            { SOURCE_HK2, GRID_INHIBIT, UNIT_MAP }
        }
    },
    { "gridDsbl", "Grid Disable", Filter_Grid_Dsbl, 1,
        {
            { SOURCE_HK2, GRID_INHIBIT, UNIT_MAP }
        }
    },
    { "sasAspin19_8", "SAS-A Spin Rate 19.8 RPM", Filter_SAS_A_Spin19_8, 1,
        {
            { SOURCE_HK2, SAS_A_SPIN_RATE, UNIT_MAP }
        }
    },
    { "sasAspin18_0", "SAS-A Spin Rate 18.0 RPM", Filter_SAS_A_Spin18_0, 1,
        {
            { SOURCE_HK2, SAS_A_SPIN_RATE, UNIT_MAP }
        }
    },
    { "sasBspin19_8", "SAS-B Spin Rate 19.8 RPM", Filter_SAS_B_Spin19_8, 1,
        {
            { SOURCE_HK2, SAS_B_SPIN_RATE, UNIT_MAP }
        }
    },
    { "sasBspin18_0", "SAS-B Spin Rate 18.0 RPM", Filter_SAS_B_Spin18_0, 1,
        {
            { SOURCE_HK2, SAS_B_SPIN_RATE, UNIT_MAP }
        }
    },
    { 0, 0, 0, 0,
        {
            { SOURCE_UNKNOWN, PARAM_UNKNOWN, UNIT_UNKNOWN }
        }
    }
};
