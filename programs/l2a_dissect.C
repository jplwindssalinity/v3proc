//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l2a_dissect
//
// SYNOPSIS
//		l2a_dissect <input_file> <output_file> <start_frame> <end_frame>
//
// DESCRIPTION
//          Reads frames start_frame through end_frame from a L2a file and
//          writes them along with the header to an output_file
//      OPTIONS
//		None.
// AUTHOR
//		Bryan Stiles
//		bstiles@acid.jpl.nasa.gov
//----------------------------------------------------------------------

//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
	"@(#) $Id$";


//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <stdlib.h>
#include "Misc.h"
#include "L2A.h"
#include "List.h"
#include "BufferedList.h"
#include "Tracking.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<EarthPosition>;
template class List<Meas>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<long>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;


const char* usage_array[] = { "<input_file>", "<output_file>","<start_frame>",
			      "<end_frame>",0};

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
	int		argc,
	char*	argv[])
{
	//------------------------//
	// parse the command line //
	//------------------------//

	const char* command = no_path(argv[0]);
	if (argc != 5)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* input_file = argv[clidx++];
	const char* output_file = argv[clidx++];
        int start_frame=atoi(argv[clidx++]);
	int end_frame=atoi(argv[clidx++]);

	//------------------------//
	// create L2A object      //
	//------------------------//
	L2A l2a;

	//------------------------//
	// open the input file    //
	//------------------------//

 	if (! l2a.OpenForReading(input_file))
	{
		fprintf(stderr, "%s: error opening input file %s\n", command,
			input_file);
		exit(1);
	}

	//------------------------//
	// open the output file   //
	//------------------------//

 	if (! l2a.OpenForWriting(output_file))
	{
		fprintf(stderr, "%s: error creating output file %s\n", command,
			input_file);
		exit(1);
	}       

        //------------------------//
        // Copy Header            //
        //------------------------//
  
	l2a.ReadHeader();
        l2a.WriteHeader();



        int frame_number=1;

	//---------------------//
	// copy desired frames //
	//---------------------//

	while (l2a.ReadDataRec() && frame_number <= end_frame)
	{
	  if(frame_number >= start_frame){
	    l2a.WriteDataRec();
	  }
          frame_number++;
        }

        //----------------------//
        // close files and exit //
        //----------------------//

	l2a.Close();
        return(0);
}
