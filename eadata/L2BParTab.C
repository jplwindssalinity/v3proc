//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.2   20 Apr 1998 10:22:04   sally
// change for WindSwatch
// 
//    Rev 1.1   17 Apr 1998 16:48:56   sally
// add L2A and L2B parameter tables
// 
//    Rev 1.0   03 Apr 1998 16:54:58   sally
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
//     defines a giant table to hold all LBA parameters.
//     Each parameter has up to 5 unit entries
//     which defines how to extract and interpret the data according
//     the unit type.
//-********************************************************************

static const char rcs_id_L2BParTab_C[] = "@(#) $Header$";

const ParTabEntry L2BParTab[] =
{
  { WVC_ROW, "WVC Row", SOURCE_L2B, MEAS_DATA, "wvc_row", 1, {
      { UNIT_DN, "dn", DATA_INT2, 0, ExtractData1D, pr_int2 }
    }
  },
  { WVC_LAT, "WVC Latitude", SOURCE_L2B, MEAS_DATA, "wvc_lat", 1, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_76, 0,
                            ExtractData2D_76_int2_float, pr_76float4_6 }
    }
  },
  { WVC_LON, "WVC Longtitude", SOURCE_L2B, MEAS_DATA, "wvc_lon", 1, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_76, 0,
                            ExtractData2D_76_uint2_float, pr_76float4_6 }
    }
  },
  { WVC_INDEX, "WVC Index", SOURCE_L2B, MEAS_DATA, "wvc_index", 1, {
      { UNIT_DN, "dn", DATA_INT1_76, 0, ExtractData2D_76, pr_int1_76 }
    }
  },
  { NUM_IN_FORE, "Number in Fore", SOURCE_L2B, MEAS_DATA, "num_in_fore", 1, {
      { UNIT_DN, "dn", DATA_INT1_76, 0, ExtractData2D_76, pr_int1_76 }
    }
  },
  { NUM_IN_AFT, "Number in Aft", SOURCE_L2B, MEAS_DATA, "num_in_aft", 1, {
      { UNIT_DN, "dn", DATA_INT1_76, 0, ExtractData2D_76, pr_int1_76 }
    }
  },
  { NUM_OUT_FORE, "Number Out Fore", SOURCE_L2B, MEAS_DATA, "num_out_fore", 1, {
      { UNIT_DN, "dn", DATA_INT1_76, 0, ExtractData2D_76, pr_int1_76 }
    }
  },
  { NUM_OUT_AFT, "Number Out Aft", SOURCE_L2B, MEAS_DATA, "num_out_aft", 1, {
      { UNIT_DN, "dn", DATA_INT1_76, 0, ExtractData2D_76, pr_int1_76 }
    }
  },
  { WVC_QUALITY_FLAG, "WVC Quality Flag", SOURCE_L2B, MEAS_DATA,
                   "wvc_quality_flag", 1, {
      { UNIT_DN, "dn", DATA_UINT2_76, 0, ExtractData2D_76, pr_uint2_76 }
    }
  },
  { ATTEN_CORR, "Atten Corr", SOURCE_L2B, MEAS_DATA,
                   "atten_corr", 1, {
      { UNIT_DB, "dB", DATA_FLOAT4_76, 0,
                           ExtractData2D_76_int2_float, pr_76float4_6 }
    }
  },
  { MODEL_SPEED, "Model Speed", SOURCE_L2B, MEAS_DATA, "model_speed", 1, {
      { UNIT_MS, "ms", DATA_FLOAT4_76, 0,
                           ExtractData2D_76_int2_float, pr_76float4_6 }
    }
  },
  { MODEL_DIR, "Model Dir", SOURCE_L2B, MEAS_DATA, "model_dir", 1, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_76, 0,
                           ExtractData2D_76_uint2_float, pr_76float4_6 }
    }
  },
  { NUM_AMBIGS, "Number Ambigs", SOURCE_L2B, MEAS_DATA, "num_ambigs", 1, {
      { UNIT_DN, "dn", DATA_INT1_76, 0, ExtractData2D_76, pr_int1_76 }
    }
  },
  { WIND_SPEED, "Wind Speed", SOURCE_L2B, MEAS_DATA, "wind_speed", 1, {
      { UNIT_MS, "ms", DATA_FLOAT4_76_4, 0,
                           ExtractData3D_76_4_int2_float, pr_76_4_float4_6 }
    }
  },
  { WIND_DIR, "Wind Direction", SOURCE_L2B, MEAS_DATA, "wind_dir", 1, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_76_4, 0,
                            ExtractData3D_76_4_uint2_float, pr_76_4_float4_6 }
    }
  },
  { WIND_SPEED_ERR, "Wind Speed Error", SOURCE_L2B, MEAS_DATA,
                           "wind_speed_err", 1, {
      { UNIT_MS, "ms", DATA_FLOAT4_76_4, 0,
                            ExtractData3D_76_4_int2_float, pr_76_4_float4_6}
    }
  },
  { WIND_DIR_ERR, "Wind Dir Err", SOURCE_L2B, MEAS_DATA, "wind_dir_err", 1, {
      { UNIT_DEGREES, "degrees", DATA_FLOAT4_76_4, 0,
                            ExtractData3D_76_4_int2_float, pr_76_4_float4_6 }
    }
  },
  { MAX_LIKELIHOOD_EST, "Max Likelihood Est (MLE)", SOURCE_L2B, MEAS_DATA,
                           "max_likelihood_est", 1, {
      { UNIT_DN, "dn", DATA_FLOAT4_76_4, 0,
                            ExtractData3D_76_4_int2_float, pr_76_4_float4_6 }
    }
  },
  { WVC_SELECTION, "WVC Selection", SOURCE_L2B, MEAS_DATA,
                           "wvc_selection", 1, {
      { UNIT_DN, "dn", DATA_INT1_76, 0, ExtractData2D_76, pr_int1_76 }
    }
  },
};

const int L2BParTabSize = ElementNumber(L2BParTab);
