//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		dtc_format
//
// SYNOPSIS
//		dtc_format <input_format> <input_dtc_file>
//			<output_format> <output_dtc_file>
//
// DESCRIPTION
//		Converts DTC files among various formats.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operands are supported:
//		<input_format>		Used to indicate the format of the input DTC file.
//								Can be h (hex), b (binary)
//		<input_dtc_file>	The DTC input file.
//		<output_format>		Used to indicate the format of the output DTC file.
//								Can be h (hex), b (binary), c (code)
//		<output_dtc_file>	The DTC input file.
//
// EXAMPLES
//		An example of a command line is:
//			% dtc_format h dtc.1.hex b dtc.1.bin
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

const char* usage_array[] = { "<input_format>", "<input_dtc_file>",
	"<output_format>", "<output_dtc_file>", 0 };

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
	// read in DTC file //
	//------------------//

	DopplerTracker doppler_tracker;
	switch (input_format)
	{
	case 'h':
		if (! doppler_tracker.ReadHex(input_file))
		{
			fprintf(stderr, "%s: error reading hex DTC file %s\n", command,
				input_file);
			exit(1);
		}
		break;
	case 'b':
		if (! doppler_tracker.ReadBinary(input_file))
		{
			fprintf(stderr, "%s: error reading binary DTC file %s\n", command,
				input_file);
			exit(1);
		}
		break;
	}

	//--------------------//
	// write out DTC file //
	//--------------------//

	switch (output_format)
	{
	case 'h':
		if (! doppler_tracker.WriteHex(output_file))
		{
			fprintf(stderr, "%s: error writing hex DTC file %s\n", command,
				output_file);
			exit(1);
		}
		break;
	case 'b':
		if (! doppler_tracker.WriteBinary(output_file))
		{
			fprintf(stderr, "%s: error writing binary DTC file %s\n", command,
				output_file);
			exit(1);
		}
		break;
	case 'c':
		if (! doppler_tracker.WriteCode(output_file))
		{
			fprintf(stderr, "%s: error writing code DTC file %s\n", command,
				output_file);
			exit(1);
		}
		break;
	}

	return (0);
}
