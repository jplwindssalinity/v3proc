//==============================================================//
// Copyright (C) 1999-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef MUDH_H
#define MUDH_H

#define MAX_SHORT  65535

// these are for the compressed table
#define NBD_DIM  32
#define SPD_DIM  32
#define DIR_DIM  32
#define MLE_DIM  32

#define NBD_MIN   -4.0
#define NBD_MAX    5.0
#define SPD_MIN    0.0
#define SPD_MAX   26.0
#define DIR_MIN    0.0
#define DIR_MAX   90.0
#define MLE_MIN   -8.0
#define MLE_MAX    0.0

#define AT_WIDTH  1624
#define CT_WIDTH  76

//-----------//
// PCA stuff //
//-----------//

#define DIM       32
#define PC_COUNT  4

enum { NBD_IDX = 0, SPD_IDX, DIR_IDX, MLE_IDX, ENOF_IDX, QUAL_IDX, TBH_IDX,
    TBV_IDX, TBH_STD_IDX, TBV_STD_IDX, TRANS_IDX, PARAM_COUNT };

enum { INNER_CLEAR = 0, INNER_RAIN, INNER_UNKNOWN, OUTER_CLEAR, OUTER_RAIN,
    OUTER_UNKNOWN, NO_WIND, UNKNOWN, MUDH_CLASS_COUNT };

//-------------------//
// for transmittance //
//-------------------//

const double alpha1 = 1.0;
const double alpha2 = 1.0;
const double beta1 = 0.0;
const double beta2 = 0.0;
const double a0 = -0.6202;
const double a1 = -0.0499;
const double a2 = 0.3283;
const double a3 = 0.00132;
const double a4 = -0.4368;
const double a5 = 0.2895;

#endif
