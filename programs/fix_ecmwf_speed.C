//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		fix_ecmwf_speed
//
// SYNOPSIS
//		fix_ecmwf_speed <ecmwf_hires_file> <spd> <output_file>
//
// DESCRIPTION
//		Fixes the wind speed in a ECMWF wind field to a constant, but
//		keeps the direction.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operands are supported:
//		<ecmwf_hires_file>		The input ECMWF HiRes wind field
//		<spd>					The fixed wind speed.
//		<output_file>			The output ECMWF file
//
// EXAMPLES
//		An example of a command line is:
//			% fix_ecmwf_speed ecmwf.hires 5.0 ecmwf.5.hires
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

const char* usage_array[] = { "<ecmwf_hires_file>", "<spd>", "<output_file",
				0};

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
	if (argc != 4)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* ecmwf_hires_file = argv[clidx++];
	float spd = atof(argv[clidx++]);
	const char* output_file = argv[clidx++];

	//--------------------------//
	// read in ECMWF HiRes file //
	//--------------------------//

	WindField wind_field;
	if (! wind_field.ReadEcmwfHiRes(ecmwf_hires_file))
	{
		fprintf(stderr, "%s: error reading ECMWF HiRes file %s\n", command,
			ecmwf_hires_file);
		exit(1);
	}

	//--------------------//
	// fix the wind speed //
	//--------------------//

	wind_field.SetAllSpeeds(spd);

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
