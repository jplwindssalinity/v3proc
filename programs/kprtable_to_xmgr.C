//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		kpr_table_to_xmgr
//
// SYNOPSIS
//	        kpr_table_to_xmgr <kprtable_file> <output_file>
//
// DESCRIPTION
//             Read kprtable file writes to block data file for xmgr.
//             (Output values in dB)
//
// OPTIONS
//		None.
//
// OPERANDS
//  
//
// EXAMPLES
//		
//
// ENVIRONMENT
//		Not environment dependent.
//
// EXIT STATUS
//		The following exit values are returned:
//		0	Program executed successfully
//		>0	Program had an error
//
// NOTES
//		None.
//
// AUTHOR
//		Bryan W. Stiles
//		bstiles@warukaze.jpl.nasa.gov
//----------------------------------------------------------------------


//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
	"@(#) $Id$";

//----------------------//
// Includes             //
//----------------------//

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Kpr.h"
#include "Meas.h"
#include "L00.h"
#include "Misc.h"
#include "Tracking.h"
#include "Tracking.C"

//-----------------------//
// Templates             //
//-----------------------//

template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<long>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

//-----------//
// CONSTANTS //
//-----------//

#define NUMBER_OF_AZIMUTH_BINS 100
#define SLICES_PER_SPOT 6
#define NUMBER_OF_BEAMS 2
#define SLICE_BANDWIDTH 8300.0
#define MIN_NUM_SAMPLES 1
#define SMOOTH_FILTER_SIZE 7
#define ANTENNA_CYCLES_PER_FRAME 10

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<kprtablefile>", 
			      "<output_file>","<composite_size>",0};

int main(int argc, char* argv[]){	

        //------------------------//
        // parse the command line //
	//------------------------//

	const char* command = no_path(argv[0]);
	if (argc != 4)
		usage(command, usage_array, 1);


	Kprs kpr;
        if(!kpr.Read(argv[1])) exit(1);
        if(!kpr.WriteXmgr(argv[2],atoi(argv[3]))) exit(1);

	exit(0);
}

