//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.1   06 Jul 1999 10:58:54   sally
// corrected cut and paste error in kp_beta
// 
//    Rev 1.0   25 May 1999 14:05:30   sally
// Initial revision.
// 
// 
// $Date$
// $Revision$
// $Author$
//
//=========================================================

#include "ParTab.h"
#include "L1AExtract.h"
#include "Print.h"

//-********************************************************************
//     defines a giant table to hold all L2Ax parameters.
//     Each parameter has up to 5 unit entries
//     which defines how to extract and interpret the data according
//     the unit type.
//     This table is the L2A format for the first 4 months after launch.
//     Then the table will be switched to the regular L2A.
//-********************************************************************

static const char rcs_id_L2AxParTab_C[] = "@(#) $Header$";

const ParTabEntry L2AxParTab[] =
{
  { WVC_ROW_TIME, "WVC Row Time", SOURCE_L2A, MEAS_TIME, "v:wvc_row_time", 6, {
      { UNIT_AUTOTIME, "(auto)", DATA_ITIME, 0, ExtractL1Time, NULL },
      { UNIT_CODE_A,   "Code A", DATA_ITIME,0,ExtractL1Time,pr_itime_codea},
      { UNIT_DAYS,     "days", DATA_ITIME, 0, ExtractL1Time, pr_itime_d },
      { UNIT_HOURS,    "hours", DATA_ITIME, 0, ExtractL1Time, pr_itime_h },
      { UNIT_MINUTES,  "minutes", DATA_ITIME, 0, ExtractL1Time, pr_itime_m },
      { UNIT_SECONDS,  "seconds", DATA_ITIME, 0, ExtractL1Time, pr_itime_s }
    }
  },
  { ROW_NUMBER, "Row Number", SOURCE_L2A, MEAS_DATA, "row_number", 1, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D, pr_int2 }
    }
  },
  { NUM_SIGMA0, "Number of Sigma0's", SOURCE_L2A, MEAS_DATA, "num_sigma0", 1, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D, pr_int2 }
    }
  },
  { CELL_LAT, "Cell Latitude", SOURCE_L2A, MEAS_DATA, "cell_lat", 2, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_810,
                          0, ExtractData2D_810_int2_float, pr_810float4_6 },
      { UNIT_RADIANS, "radians", DATA_FLOAT4_810,
                        0, ExtractData2D_810_int2_float_dtr, pr_810float4_6 }
    }
  },
  { CELL_LON, "Cell Longtitude", SOURCE_L2A, MEAS_DATA, "cell_lon", 2, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_810,
                          0, ExtractData2D_810_uint2_float, pr_810float4_6 },
      { UNIT_RADIANS, "radians", DATA_FLOAT4_810,
                       0, ExtractData2D_810_uint2_float_dtr, pr_810float4_6 }
    }
  },
  { CELL_AZIMUTH, "Cell Azimuth", SOURCE_L2A, MEAS_DATA, "cell_azimuth", 2, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_810,
                          0, ExtractData2D_810_uint2_float, pr_810float4_6 },
      { UNIT_RADIANS, "radians", DATA_FLOAT4_810,
                       0, ExtractData2D_810_uint2_float_dtr, pr_810float4_6 }
    }
  },
  { CELL_INCIDENCE, "Cell Incidence", SOURCE_L2A, MEAS_DATA,
               "cell_incidence", 2, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_810,
                          0, ExtractData2D_810_int2_float, pr_810float4_6 },
      { UNIT_RADIANS, "radians", DATA_FLOAT4_810,
                          0, ExtractData2D_810_int2_float_dtr, pr_810float4_6 }
    }
  },
  { SIGMA0, "Sigma0", SOURCE_L2A, MEAS_DATA, "sigma0", 1, {
      { UNIT_DB, "dB", DATA_FLOAT4_810,
                          0, ExtractData2D_810_int2_float, pr_810float4_6 }
    }
  },
  { SIGMA0_ATTN_AMSR, "Sigma Attenuation from colocated Tb",
                  SOURCE_L2A, MEAS_DATA, "sigma0_attn_amsr", 1, {
      { UNIT_DB, "dB", DATA_FLOAT4_810,
                          0, ExtractData2D_810_int2_float, pr_810float4_6 }
    }
  },
  { SIGMA0_ATTN_MAP, "Sigma Attenuation from climate map",
                  SOURCE_L2A, MEAS_DATA, "sigma0_attn_map", 1, {
      { UNIT_DB, "dB", DATA_FLOAT4_810,
                          0, ExtractData2D_810_int2_float, pr_810float4_6 }
    }
  },
  { KP_ALPHA, "KP Alpha", SOURCE_L2A, MEAS_DATA, "kp_alpha", 1, {
      { UNIT_DN, "dn", DATA_FLOAT4_810,
                          0, ExtractData2D_810_int2_float, pr_810float4_6 }
    }
  },
  { KP_BETA, "KP Beta", SOURCE_L2A, MEAS_DATA, "kp_beta", 1, {
      { UNIT_DN, "dn", DATA_FLOAT4_810,
                          0, ExtractData2D_810_int2_float, pr_810float4_6 }
    }
  },
  { KP_GAMMA, "KP Gamma", SOURCE_L2A, MEAS_DATA, "kp_gamma", 1, {
      { UNIT_DN, "dn", DATA_FLOAT4_810, 0, ExtractData2D_810, pr_810float4_6 }
    }
  },
  { SIGMA0_QUAL_FLAG, "Sigma0 Quality Flag",
                 SOURCE_L2A, MEAS_DATA, "sigma0_qual_flag", 1, {
      { UNIT_DN, "dn", DATA_UINT2_810, 0, ExtractData2D_810, pr_uint2_810 }
    }
  },
  { SIGMA0_MODE_FLAG, "Sigma0 Mode Flag",
                 SOURCE_L2A, MEAS_DATA, "sigma0_mode_flag", 1, {
      { UNIT_DN, "dn", DATA_UINT2_810, 0, ExtractData2D_810, pr_uint2_810 }
    }
  },
  { SURFACE_FLAG, "Surface Flag",
                 SOURCE_L2A, MEAS_DATA, "surface_flag", 1, {
      { UNIT_DN, "dn", DATA_UINT2_810, 0, ExtractData2D_810, pr_uint2_810 }
    }
  },
  { CELL_INDEX, "Cell Index", SOURCE_L2A, MEAS_DATA, "cell_index", 1, {
      { UNIT_DN, "dn", DATA_INT1_810, 0, ExtractData2D_810, pr_int1_810 }
    }
  }
};

const int L2AxParTabSize = ElementNumber(L2AxParTab);
