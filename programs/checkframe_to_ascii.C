//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		checkframe_to_ascii
//
// SYNOPSIS
//		checkframe_to_ascii <checkfile> <output_file>
//
// DESCRIPTION
//		Reads in a checkframe file and writes out the data in ascii form
//
// OPTIONS
//		None.
//
// EXAMPLES
//		An example of a command line is:
//			% checkframe_to_ascii simcheck.dat simcheck.txt
//
// ENVIRONMENT
//		Not environment dependent.
//
// EXIT STATUS
//		The following exit values are returned:
//		1	Program executed successfully
//		>0	Program had an error
//
// NOTES
//		None.
//
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
#include "CheckFrame.h"
#include "Meas.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"

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

//-----------//
// CONSTANTS //
//-----------//

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<checkfile>", "<output_file>", 0};

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
	if (argc != 3)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* checkfile = argv[clidx++];
	const char* output_file = argv[clidx++];

	//---------------------//
	// open the check file //
	//---------------------//

	FILE* check_fp = fopen(checkfile,"r");
	if (check_fp == NULL)
	{
		fprintf(stderr, "%s: error opening check file %s\n", command,
			checkfile);
		exit(1);
	}

	//------------------//
	// open output file //
	//------------------//

	FILE* output_fp = fopen(output_file, "w");
	if (output_fp == NULL)
	{
		fprintf(stderr, "%s: error opening output file %s\n", command,
			output_file);
		exit(1);
	}

	//-------------------//
	// Setup check frame //
	//-------------------//

	CheckFrame cf;
	if (! cf.Allocate(12))
	{
		fprintf(stderr, "%s: error allocating check frame\n", command);
		exit(1);
	}

	//----------------//
	// loop and write //
	//----------------//

	while (cf.ReadDataRec(check_fp))
	{
		cf.WriteDataRecAscii(output_fp);
		fprintf(output_fp,"\n");
	}

	//-----------------//
	// close the files //
	//-----------------//

	fclose(output_fp);
	fclose(check_fp);

	return (0);
}






