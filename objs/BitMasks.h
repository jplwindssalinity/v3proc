//=========================================================//
// Copyright (C) 2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

#ifndef BIT_MASKS_H
#define BIT_MASKS_H

static const char rcs_id_bit_masks_h[] =
    "@(#) $Id$";

//======================================================================
// DESCRIPTION
//    A bunch of useful (or not) bit masks
//======================================================================

#define L1A_OPERATIONAL_MODE_WOM         0x0e
#define L1A_OPERATIONAL_MODE_CAL         0x07
#define L1A_OPERATIONAL_MODE_STB         0x70
#define L1A_OPERATIONAL_MODE_ROM         0xe0

#define L1A_FRAME_INST_STATUS_MODE_MASK  0x00000003
#define L1A_FRAME_INST_STATUS_WOM        0x00000000
#define L1A_FRAME_INST_STATUS_CAL        0x00000001
#define L1A_FRAME_INST_STATUS_STB        0x00000002
#define L1A_FRAME_INST_STATUS_ROM        0x00000003

#endif
