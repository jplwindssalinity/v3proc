//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

#ifndef L1AFRAME_H
#define L1AFRAME_H

static const char rcs_id_l1aframe_h[] =
    "@(#) $Id$";

#include "Attitude.h"
#include "L1AGSFrame.h"

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

    GSL1AStatus    status;
    GSL1AEngData   engdata;
    GSL1AEu        in_eu;

	//----------------------------//
	// L1A Status and Error Flags //
	//----------------------------//

    unsigned int    frame_inst_status;
    unsigned int    frame_err_status;
    unsigned short  frame_qual_flag;
    unsigned char   pulse_qual_flag[13];

	//--------------------------------------------------------------------//
	// L1A character time needed by GS
	//--------------------------------------------------------------------//

    char frame_time[24];

	//-------------------------//
	// informational variables //
	//-------------------------//

	int		antennaCyclesPerFrame;
	int		spotsPerFrame;
	int		slicesPerSpot;
	int		slicesPerFrame;
};

#endif
