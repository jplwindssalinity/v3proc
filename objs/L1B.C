//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_l1b_c[] =
    "@(#) $Id$";

#include <memory.h>
#include "L1B.h"


//==========//
// L1BFrame //
//==========//

L1BFrame::L1BFrame()
{
    return;
}

L1BFrame::~L1BFrame()
{
    return;
}

//------------------//
// L1B::readPureHdf //
//------------------//

int
L1BFrame::InitReadPureHdf(char *_hdf_fn)
{
	hdf_fn = _hdf_fn;

    // prepare //
    spotList.FreeContents();

    // start access to the file //
    sd_id = SDstart(hdf_fn, DFACC_READ);
    h_id = Hopen(hdf_fn, DFACC_READ, 0);
    Vstart(h_id);	// why the F*CK is this needed AND why doesn't the documentation tell you its needed!??!
    if (sd_id == FAIL || h_id == FAIL) {
        fprintf(stderr, "L1B::ReadPureHdf: error with SDstart or Hopen\n");
        return(0);
    }
    
    //start

	// initialize variables //
	frame_i = 0;
	num_l1b_frames = 0;
    num_pulses_per_frame = 100;
    num_slices_per_pulse = 8;

    // determine the actual number of rows //
    int32 attr_idx_l1b_frames = SDfindattr(sd_id, "l1b_actual_frames");
    if (attr_idx_l1b_frames == FAIL) {
        fprintf(stderr, "L1B::ReadPureHdf: error with SDfindattr\n");
        return(0);
    }

    char data[1024];
    if (SDreadattr(sd_id, attr_idx_l1b_frames, data) == FAIL) {
        fprintf(stderr, "L1B::ReadPureHdf: error with SDreadattr\n");
        return(0);
    }

    if (sscanf(data, " %*[^\n] %*[^\n] %d", &num_l1b_frames) != 1) {
        fprintf(stderr, "L1B::ReadPureHdf: error parsing header\n");
        return(0);
    }
        
    return(1);
}

int
L1BFrame::ReadPureHdfFrame()
{
	int res = ReadPureHdfFrame(frame_i);
	frame_i++;
	return(res);
}


int
L1BFrame::ReadPureHdfFrame(int _frame_i)
{
	if (_frame_i < 0 || _frame_i >= num_l1b_frames)
		return 0;
	
    
    // the HDF read routine should only access as many dimensions as needed
    int cur_start[3] = { _frame_i, 0, 0 };
    int edges[3] = { num_l1b_frames, num_pulses_per_frame, num_slices_per_pulse };

	return spotList.UnpackL1BHdf(sd_id, h_id, cur_start, edges);
	
//    int32 slice_sigma0_sds_id = SDnametoid(sd_id, "slice_sigma0");
//	    	
//		int16 slice_sigma0[num_pulses_per_frame * num_slices_per_pulse];
//        if (SDreaddata(slice_sigma0_sds_id, generic_start, NULL,
//            generic_edges, (VOIDP)slice_sigma0) == FAIL)
//        {
//            fprintf(stderr,
//                "L1B::ReadPureHdf: error with SDreaddata (slice_sigma0)\n");
//            return(0);
//        }
//        
//        for (int ii = 0; ii < 40; ii++)
//        	printf("%hd\n", slice_sigma0[ii]);
        	

	
}


//=====//
// L1B //
//=====//

L1B::L1B()
:   _status(OK)
{
    return;
}

L1B::L1B(char *filename)
:   _status(OK)
{
	ephemeris_fp = NULL;
	frame.InitReadPureHdf(filename); 
}

L1B::~L1B()
{
    return;
}


//------------------------//
// L1B::WriteDataRecAscii //
//------------------------//

int
L1B::WriteDataRecAscii()
{
    fprintf(_outputFp, "\n");
    fprintf(_outputFp, "###############################################\n");
    fprintf(_outputFp, "#####                                    ######\n");
    fprintf(_outputFp, "#####           L1B Data Record          ######\n");
    fprintf(_outputFp, "#####                                    ######\n");
    fprintf(_outputFp, "###############################################\n");
    fprintf(_outputFp, "\n");
    return(frame.spotList.WriteAscii(_outputFp));
}




