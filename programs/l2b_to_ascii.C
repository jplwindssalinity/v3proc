//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l2b_to_ascii
//
// SYNOPSIS
//	      l2b_to_ascii <input_file> <output_file>  <start_frame> <end_frame>
//
// DESCRIPTION
//          Reads frames start_frame through end_frame from a L1b file and
//          writes them to an ASCII file
//      OPTIONS
//		Last two arguments are optional
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
#include "L2B.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"

//-----------//
// TEMPLATES //
//-----------//

class AngleInterval;
template class List<AngleInterval>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<long>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;


const char* usage_array[] = { "<input_file>", "<output_file>",0};

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
	if (argc!=3)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* input_file = argv[clidx++];
	const char* output_file = argv[clidx++];

	//------------------------//
	// create L2B object      //
	//------------------------//
	L2B l2b;

	//------------------------//
	// open the input file    //
	//------------------------//

 	if (! l2b.OpenForReading(input_file))
	{
		fprintf(stderr, "%s: error opening input file %s\n", command,
			input_file);
		exit(1);
	}

	//------------------------//
	// open the output file   //
	//------------------------//

 	if (! l2b.OpenForWriting(output_file))
	{
		fprintf(stderr, "%s: error creating output file %s\n", command,
			input_file);
		exit(1);
	}       



	//---------------------//
	// copy desired frames //
	//---------------------//
        if(! l2b.ReadHeader()){
	  fprintf(stderr, "%s: error reading from input file %s\n", command,
		  input_file);
	  exit(1);
	}
        if(! l2b.ReadDataRec()){
	  fprintf(stderr, "%s: error reading from input file %s\n", command,
		  input_file);
	  exit(1);
	}
	if(! l2b.WriteAscii()){
	  fprintf(stderr, "%s: error writing to output file %s\n", command,
		  output_file);
	  exit(1);
	}

        //----------------------//
        // close files and exit //
        //----------------------//

	l2b.Close();
        return(0);
}

