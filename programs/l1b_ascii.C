//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		l1b_ascii
//
// SYNOPSIS
//		l1b_ascii <l1b_file> [ output_file ]
//
// DESCRIPTION
//		Reads in a Level 1B file and writes the measurements
//		out in ASCII.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//		<l1b_file>			The Level 1B input file.
//		[ output_file ]		The ASCII output file.
//
// EXAMPLES
//		An example of a command line is:
//			% l1b_ascii l1b.dat l1b.txt
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
// AUTHOR
//		James N. Huddleston
//		hudd@acid.jpl.nasa.gov
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
#include "Misc.h"
#include "L1B.h"
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

const char* usage_array[] = { "<l1b_file>", "[ output_file ]", 0};

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
	if (argc < 2 || argc > 3)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* l1b_file = argv[clidx++];
	const char* output_file = NULL;
	if (argc == 3)
		output_file = argv[clidx++];

	//------------------------//
	// open the Level 1B file //
	//------------------------//

	L1B l1b;
	if (! l1b.OpenForReading(l1b_file))
	{
		fprintf(stderr, "%s: error opening Level 1B file %s\n", command,
			l1b_file);
		exit(1);
	}

	//------------------//
	// open output file //
	//------------------//

	FILE* output_fp = stdout;
	if (output_file)
	{
		output_fp = fopen(output_file, "w");
		if (output_fp == NULL)
		{
			fprintf(stderr, "%s: error opening output file %s\n", command,
				output_file);
			exit(1);
		}
	}

	//----------------//
	// loop and write //
	//----------------//

	while (l1b.ReadDataRec())
	{
		MeasSpotList* msl = &(l1b.frame.spotList);
		for (MeasSpot* ms = msl->GetHead(); ms; ms = msl->GetNext())
		{
			ms->WriteAscii(output_fp);
			fprintf(output_fp, "\n");
		}
	}

	//-----------------//
	// close the files //
	//-----------------//

	if (output_fp != stdout)
		fclose(output_fp);
	l1b.Close();

	return (0);
}
