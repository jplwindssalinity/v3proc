//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		xtable_to_xmgr
//
// SYNOPSIS
//	        xtable_to_xmgr <xtable_file> <output_file> <orbit_position>
//
// DESCRIPTION
//             Read xtable file writes to block data file for xmgr.
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
#include "XTable.h"
#include "Meas.h"
#include "L00.h"
#include "Misc.h"

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

//-----------//
// CONSTANTS //
//-----------//


//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<xtablefile>", 
			      "<output_file>","<orbit_position>",0};

int main(int argc, char* argv[]){	

        //------------------------//
        // parse the command line //
	//------------------------//

	const char* command = no_path(argv[0]);
	if (argc != 4)
		usage(command, usage_array, 1);


	XTable X;
        if(!X.SetFilename(argv[1])) exit(1);
        if(!X.Read()) exit(1);
        if(!X.WriteXmgr(argv[2],atof(argv[3]))) exit(1);

	exit(0);
}

