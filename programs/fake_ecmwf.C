//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		fake_ecmwf
//
// SYNOPSIS
//		fake_ecmwf <spd> <output_file>
//
// DESCRIPTION
//		Generates a fake ECMWF Hi-Res wind field with a fixed wind
//      speed and smoothly varying direction.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operands are supported:
//		<spd>					The fixed wind speed.
//		<output_file>			The output ECMWF file
//
// EXAMPLES
//		An example of a command line is:
//			% fake_ecmwf 5.0 ecmwf.5.hires
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
#include <stdlib.h>
#include "Misc.h"
#include "Wind.h"
#include "List.h"
#include "List.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<EarthPosition>;
template class List<WindVectorPlus>;

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

const char* usage_array[] = { "<spd>", "<output_file>", 0};

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
	float spd = atof(argv[clidx++]);
	const char* output_file = argv[clidx++];

	//----------------//
	// fake the field //
	//----------------//

	WindField wind_field;
	wind_field.FakeEcmwfHiRes(spd);

	//----------------------//
	// write out ECMWF file //
	//----------------------//

	if (! wind_field.WriteEcmwfHiRes(output_file))
	{
		fprintf(stderr, "%s: error writing ECMWF file %s\n", command,
			output_file);
		exit(1);
	}

	return (0);
}
