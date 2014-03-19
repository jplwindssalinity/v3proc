//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		smooth_kprtable
//
// SYNOPSIS
//	  smooth_kprtable <input_file> <output_file> filter_width num_passes
//
// DESCRIPTION
//             Read kprtable file , smooths, writes to output file
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
#include "BufferedList.h"
#include "Kpr.h"
#include "Meas.h"
#include "L00.h"
#include "Misc.h"
#include "Tracking.h"

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


//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<input_file>", 
			      "<output_file>","<filter_width>"
			      "<num_passes>",0};

int main(int argc, char* argv[]){	

        //------------------------//
        // parse the command line //
	//------------------------//

	const char* command = no_path(argv[0]);
	if (argc != 5)
		usage(command, usage_array, 1);


	Kprs kpr;
        if(!kpr.Read(argv[1])) exit(1);
        int num_passes=atoi(argv[4]);
        for(int c=0;c<num_passes;c++){
	  if(!kpr.Smooth(atoi(argv[3]))) exit(1);
	}
        if(!kpr.Write(argv[2])) exit(1);

	exit(0);
}

