//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		rgc_format
//
// SYNOPSIS
//		rgc_format <input_format> <input_rgc_file>
//			<output_format> <output_rgc_file>
//
// DESCRIPTION
//		Converts RGC files among various formats.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operands are supported:
//		<input_format>		Used to indicate the format of the input RGC file.
//								Can be h (hex), b (binary)
//		<input_rgc_file>	The RGC input file.
//		<output_format>		Used to indicate the format of the output RGC file.
//								Can be h (hex), b (binary), c (code)
//		<output_rgc_file>	The RGC input file.
//
// EXAMPLES
//		An example of a command line is:
//			% rgc_format h rgc.1.hex b rgc.1.bin
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
#include "Tracking.h"
#include "Tracking.C"

//-----------//
// TEMPLATES //
//-----------//

/*
template class List<EarthPosition>;
*/

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

const char* usage_array[] = { "<input_format>", "<input_rgc_file>",
	"<output_format>", "<output_rgc_file>", 0 };

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

	int arg_idx = 1;
	const char input_format = argv[arg_idx++][0];
	const char* input_file = argv[arg_idx++];
	const char output_format = argv[arg_idx++][0];
	const char* output_file = argv[arg_idx++];

	//------------------//
	// read in RGC file //
	//------------------//

	RangeTracker range_tracker;
	switch (input_format)
	{
	case 'h':
		if (! range_tracker.ReadHex(input_file))
		{
			fprintf(stderr, "%s: error reading hex RGC file %s\n", command,
				input_file);
			exit(1);
		}
		break;
	case 'b':
		if (! range_tracker.ReadBinary(input_file))
		{
			fprintf(stderr, "%s: error reading binary RGC file %s\n", command,
				input_file);
			exit(1);
		}
		break;
	}

	//--------------------//
	// write out RGC file //
	//--------------------//

	switch (output_format)
	{
	case 'h':
		if (! range_tracker.WriteHex(output_file))
		{
			fprintf(stderr, "%s: error writing hex RGC file %s\n", command,
				output_file);
			exit(1);
		}
		break;
	case 'b':
		if (! range_tracker.WriteBinary(output_file))
		{
			fprintf(stderr, "%s: error writing binary RGC file %s\n", command,
				output_file);
			exit(1);
		}
		break;
	case 'c':
		if (! range_tracker.WriteCode(output_file))
		{
			fprintf(stderr, "%s: error writing code RGC file %s\n", command,
				output_file);
			exit(1);
		}
		break;
	}

	return (0);
}
