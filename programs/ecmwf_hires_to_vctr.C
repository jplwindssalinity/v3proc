//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		ecmwf_hires_to_bev
//
// SYNOPSIS
//		ecmwf_hires_to_bev <ecmwf_hires_file> <bev_file>
//
// DESCRIPTION
//		Converts a ECMWF HiRes file into a bev (binary earth vector) file
//		file for plotting in IDL.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operands are supported:
//		<ecmwf_hires_file>		The input ECMWF HiRes wind field
//		<bev_file>		The output bev file
//
// EXAMPLES
//		An example of a command line is:
//			% ecmwf_hires_to_bev ecmwf.hires output.bev
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

const char* usage_array[] = { "<ecmwf_hires_file>", "<bev_file>", 0};

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
	const char* ecmwf_hires_file = argv[clidx++];
	const char* bev_file = argv[clidx++];

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
	// write out bev file //
	//--------------------//

	if (! wind_field.WriteBev(bev_file))
	{
		fprintf(stderr, "%s: error writing bev file %s\n", command, bev_file);
		exit(1);
	}

	return (0);
}
