//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		vap_to_vctr
//
// SYNOPSIS
//		vap_to_vctr <vap_file> <Vctr_file>
//
// DESCRIPTION
//		Converts a vap file into a Vctr (vector) file for plotting in IDL.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operands are supported:
//		<vap_file>		The input vap wind field
//		<Vctr_file>		The output vctr file
//
// EXAMPLES
//		An example of a command line is:
//			% vap_to_vctr 96interp.bin output.vctr
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

const char* usage_array[] = { "<vap_file>", "<Vctr_file>", 0};

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
	const char* vap_file = argv[clidx++];
	const char* vctr_file = argv[clidx++];

	//------------------//
	// read in vap file //
	//------------------//

	WindField wind_field;
	if (! wind_field.ReadVap(vap_file))
	{
		fprintf(stderr, "%s: error reading vap file %s\n", command, vap_file);
		exit(1);
	}

	//---------------------//
	// write out vctr file //
	//---------------------//

	if (! wind_field.WriteVctr(vctr_file))
	{
		fprintf(stderr, "%s: error writing Vctr file %s\n", command,
			vctr_file);
		exit(1);
	}

	return (0);
}
