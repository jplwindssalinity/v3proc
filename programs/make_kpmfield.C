//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		make_kpmfield
//
// SYNOPSIS
//		make_kpmfield <correlation length> <output_file>
//
// DESCRIPTION
//		Builds a kpm field and stores it in a file.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//		<correlation_length>	The gaussian corr length (km) to use.
//		<output_file>			The file name to output to.
//
// EXAMPLES
//		An example of a command line is:
//			% make_kpmfield 250 corr_kpm.250
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
#include <fcntl.h>
#include <math.h>
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "ConfigList.h"
#include "Spacecraft.h"
#include "ConfigSim.h"
#include "Array.h"
#include "Meas.h"
#include "Ephemeris.h"
#include "Wind.h"
#include "Kpm.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<WindVectorPlus>;
template class List<long>;
template class List<OffsetList>;

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

const char* usage_array[] = { "<corr_length>", "<output_filename>", 0};

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
	const float corr_length = atof(argv[clidx++]);
	char* output_file = argv[clidx++];

	//------------------//
	// Setup a KpmField //
	//------------------//

	KpmField kpmField;

	printf("Building correlated Kpm field (corr_len = %g km) ...\n",
    	corr_length);
	if (! kpmField.Build(corr_length))
	{
		printf("Error building the KpmField\n");
		return(0);
	}
	printf("... Done\n");

	if (! kpmField.corr.Write(output_file))
	{
		printf("Error writing Kpm field to %s\n",output_file);
		return(0);
	}

	return (0);
}
