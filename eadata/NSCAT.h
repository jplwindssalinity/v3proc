//=========================================================
// Copyright  (C)1995, California Institute of Technology. 
// U.S. Government sponsorship under 
// NASA Contract NAS7-1260 is acknowledged
//
//
// CM Log
// $Log$
// 
//    Rev 1.0   04 Feb 1998 14:16:28   daffer
// Initial checking
// Revision 1.2  1998/01/30 22:28:32  daffer
// Added pvcs keywords, fixed headers, fixed rcsid strings - whd
//
// $Date$
// $Revision$
// $Author$
//
//
//=========================================================

#ifndef NSCAT_H
#define NSCAT_H

static const char rcs_nscat_h[]="@(#) $Header$";

// sampling frequencies in Hz
#define SAMPLING_FREQUENCY_CH1 480588.0
#define SAMPLING_FREQUENCY_CH2 263548.0
#define SAMPLING_FREQUENCY_CH3 136167.0
#define SAMPLING_FREQUENCY_CH4 121940.0

// bin widths in Hz
#define BIN_WIDTH_CH1 SAMPLING_FREQUENCY_CH1/512.0
#define BIN_WIDTH_CH2 SAMPLING_FREQUENCY_CH2/512.0
#define BIN_WIDTH_CH3 SAMPLING_FREQUENCY_CH3/512.0
#define BIN_WIDTH_CH4 SAMPLING_FREQUENCY_CH4/512.0

#endif
