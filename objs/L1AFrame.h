//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef L1AFRAME_H
#define L1AFRAME_H

static const char rcs_id_l1aframe_h[] =
    "@(#) $Id$";

#include "Attitude.h"


//======================================================================
// CLASSES
//    L1AFrame
//======================================================================

//======================================================================
// CLASS
//    L1AFrame
//
// DESCRIPTION
//    The L1AFrame object contains the contents of a Level 1A frame
//    as a structure.
//======================================================================

class L1AFrame
{
public:

    //--------------//
    // construction //
    //--------------//

    L1AFrame();
    ~L1AFrame();

    int  Allocate(int number_of_beams, int antenna_cycles_per_frame,
             int slices_per_spot);
    int  Deallocate();
    int  FrameSize();

    //-------------------//
    // data manipulation //
    //-------------------//

    int  Pack(char* buffer);
    int  Unpack(char* buffer);
    int  WriteAscii(FILE* ofp);

    //-------------------//
    // product variables //
    //-------------------//

    double         time;
    unsigned int   instrumentTicks;
    unsigned int   orbitTicks;
    unsigned char  orbitStep;
    unsigned char  priOfOrbitStepChange;

	//-----------------//
	// S/C information //
	//-----------------//

	float			gcAltitude;
	float			gcLongitude;
	float			gcLatitude;
	float			gcX;
	float			gcY;
	float			gcZ;
	float			velX;
	float			velY;
	float			velZ;
	Attitude		attitude;
	float			ptgr;
    unsigned char   calPosition;

    //----------//
    // cal data //
    //----------//

    float*              loopbackSlices;
    float               loopbackNoise;
    float*              loadSlices;
    float               loadNoise;

	// antenna position
	unsigned short*		antennaPosition;

    //----------------------//
	// science measurements //
    //----------------------//

	float*				science;
	float*				spotNoise;

    //---------------------------------------------------------//
    // Additional data needed to make GS compatible L1A files. //
    //---------------------------------------------------------//

    unsigned short  prf_cycle_time_eu;
    unsigned short  range_gate_delay_inner;
    unsigned short  range_gate_delay_outer;
    unsigned short  range_gate_width_inner;
    unsigned short  range_gate_width_outer;
    unsigned short  transmit_pulse_width;
    char            true_cal_pulse_pos;
    short           precision_coupler_temp_eu;
    short           rcv_protect_sw_temp_eu;
    short           beam_select_sw_temp_eu;
    short           receiver_temp_eu;

	//----------------------------//
	// L1A Status and Error Flags //
	//----------------------------//

    unsigned int    frame_inst_status;
    unsigned int    frame_err_status;
    unsigned short  frame_qual_flag;
    unsigned char   pulse_qual_flag;

	//--------------------------------------------------------------------//
	// L1A Raw Telemetry Values needed to go back to L00 (ie., telemetry) //
    // These values are all in DN's.                                      //
	//--------------------------------------------------------------------//

    char            frame_time[21];
    double          frame_time_secs;
    double          instrument_time;
    unsigned char   prf_count;
    char            specified_cal_pulse_pos;
    unsigned char   prf_cycle_time;
    unsigned char   range_gate_a_delay;
    unsigned char   range_gate_a_width;
    unsigned char   range_gate_b_delay;
    unsigned char   range_gate_b_width;
    unsigned char   pulse_width;
    unsigned char   pred_antenna_pos_count;
    unsigned int    vtcw[2];
    unsigned char   precision_coupler_temp;
    unsigned char   rcv_protect_sw_temp;
    unsigned char   beam_select_sw_temp;
    unsigned char   receiver_temp;

	//-------------------------//
	// informational variables //
	//-------------------------//

	int		antennaCyclesPerFrame;
	int		spotsPerFrame;
	int		slicesPerSpot;
	int		slicesPerFrame;
};

#endif
