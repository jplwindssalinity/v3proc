//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.3   23 Feb 1999 11:13:16   sally
// L2A array size chaned from 810 to 3240
// 
//    Rev 1.2   17 Apr 1998 16:48:54   sally
// add L2A and L2B parameter tables
// 
//    Rev 1.1   03 Apr 1998 16:17:56   sally
// fixed keyword
// 
//    Rev 1.0   30 Mar 1998 15:12:42   sally
// Initial revision.
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
//     defines a giant table to hold all L2A parameters.
//     Each parameter has up to 5 unit entries
//     which defines how to extract and interpret the data according
//     the unit type.
//-********************************************************************

static const char rcs_id_L2AParTab_C[] = "@(#) $Header$";

const ParTabEntry L2AParTab[] =
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
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_3240,
                          0, ExtractData2D_3240_int2_float, pr_3240float4_6 },
      { UNIT_RADIANS, "radians", DATA_FLOAT4_3240,
                        0, ExtractData2D_3240_int2_float_dtr, pr_3240float4_6 }
    }
  },
  { CELL_LON, "Cell Longtitude", SOURCE_L2A, MEAS_DATA, "cell_lon", 2, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_3240,
                          0, ExtractData2D_3240_uint2_float, pr_3240float4_6 },
      { UNIT_RADIANS, "radians", DATA_FLOAT4_3240,
                       0, ExtractData2D_3240_uint2_float_dtr, pr_3240float4_6 }
    }
  },
  { CELL_AZIMUTH, "Cell Azimuth", SOURCE_L2A, MEAS_DATA, "cell_azimuth", 2, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_3240,
                          0, ExtractData2D_3240_uint2_float, pr_3240float4_6 },
      { UNIT_RADIANS, "radians", DATA_FLOAT4_3240,
                       0, ExtractData2D_3240_uint2_float_dtr, pr_3240float4_6 }
    }
  },
  { CELL_INCIDENCE, "Cell Incidence", SOURCE_L2A, MEAS_DATA,
               "cell_incidence", 2, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_3240,
                          0, ExtractData2D_3240_int2_float, pr_3240float4_6 },
      { UNIT_RADIANS, "radians", DATA_FLOAT4_3240,
                          0, ExtractData2D_3240_int2_float_dtr, pr_3240float4_6 }
    }
  },
  { SIGMA0, "Sigma0", SOURCE_L2A, MEAS_DATA, "sigma0", 1, {
      { UNIT_DB, "dB", DATA_FLOAT4_3240,
                          0, ExtractData2D_3240_int2_float, pr_3240float4_6 }
    }
  },
  { SIGMA0_ATTN_AMSR, "Sigma Attenuation from colocated Tb",
                  SOURCE_L2A, MEAS_DATA, "sigma0_attn_amsr", 1, {
      { UNIT_DB, "dB", DATA_FLOAT4_3240,
                          0, ExtractData2D_3240_int2_float, pr_3240float4_6 }
    }
  },
  { SIGMA0_ATTN_MAP, "Sigma Attenuation from climate map",
                  SOURCE_L2A, MEAS_DATA, "sigma0_attn_map", 1, {
      { UNIT_DB, "dB", DATA_FLOAT4_3240,
                          0, ExtractData2D_3240_int2_float, pr_3240float4_6 }
    }
  },
  { KP_ALPHA, "KP Alpha", SOURCE_L2A, MEAS_DATA, "kp_alpha", 1, {
      { UNIT_DN, "dn", DATA_FLOAT4_3240,
                          0, ExtractData2D_3240_int2_float, pr_3240float4_6 }
    }
  },
  { KP_BETA, "KP Beta", SOURCE_L2A, MEAS_DATA, "kp_alpha", 1, {
      { UNIT_DN, "dn", DATA_FLOAT4_3240,
                          0, ExtractData2D_3240_int2_float, pr_3240float4_6 }
    }
  },
  { KP_GAMMA, "KP Gamma", SOURCE_L2A, MEAS_DATA, "kp_gamma", 1, {
      { UNIT_DN, "dn", DATA_FLOAT4_3240, 0, ExtractData2D_3240, pr_3240float4_6 }
    }
  },
  { SIGMA0_QUAL_FLAG, "Sigma0 Quality Flag",
                 SOURCE_L2A, MEAS_DATA, "sigma0_qual_flag", 1, {
      { UNIT_DN, "dn", DATA_UINT2_3240, 0, ExtractData2D_3240, pr_uint2_3240 }
    }
  },
  { SIGMA0_MODE_FLAG, "Sigma0 Mode Flag",
                 SOURCE_L2A, MEAS_DATA, "sigma0_mode_flag", 1, {
      { UNIT_DN, "dn", DATA_UINT2_3240, 0, ExtractData2D_3240, pr_uint2_3240 }
    }
  },
  { SURFACE_FLAG, "Surface Flag",
                 SOURCE_L2A, MEAS_DATA, "surface_flag", 1, {
      { UNIT_DN, "dn", DATA_UINT2_3240, 0, ExtractData2D_3240, pr_uint2_3240 }
    }
  },
  { CELL_INDEX, "Cell Index", SOURCE_L2A, MEAS_DATA, "cell_index", 1, {
      { UNIT_DN, "dn", DATA_INT1_3240, 0, ExtractData2D_3240, pr_int1_3240 }
    }
  }
};

const int L2AParTabSize = ElementNumber(L2AParTab);
