//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		resample_ephem
//
// SYNOPSIS
//		resample_ephem ephemeris_file output_file spacing
//
// DESCRIPTION
//		Reads in an ephemeris file and outputs another ephemeris file
//		interpolated onto a new time spacing.
//
// OPTIONS
//		None.
//
// OPERANDS
//		ephemeris_file = name of the ephemeris file to resample.
//		output_file = name of the file to put the resampled ephemeris in.
//		spacing = the new time spacing to use.
//
// ENVIRONMENT
//		Not environment dependent.
//
// EXIT STATUS
//		The following exit values are returned:
//		0	Program executed successfully
//		>0	Program had an error
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
#include <fcntl.h>
#include "Ephemeris.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "ConfigList.h"
#include "Spacecraft.h"
#include "ConfigSim.h"
#include "Array.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<Meas>;
template class List<LonLat>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
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

const char* usage_array[] = { "ephemeris_file", "output_file", "spacing", 0};

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
	const char* ephemeris_filename = argv[clidx++];
	const char* output_filename = argv[clidx++];
	const double spacing = atof(argv[clidx++]);

	//---------------------//
	// create an ephemeris //
	//---------------------//

	Ephemeris ephemeris;
    ephemeris.SetInputFile(ephemeris_filename);
	ephemeris.SetMaxNodes(50);

	FILE* output_ephemeris = fopen(output_filename,"w");
	if (output_ephemeris == NULL)
	{
		printf("Error writing to %s\n",output_filename);
		exit(-1);
	}

	OrbitState os;
//	OrbitState *os = ephemeris.GetOrReadNext();
//	double t = os->time;
	double t = 0;

	//------------------------------------------------//
	// Resample the ephemeris data at the new spacing //
	//------------------------------------------------//

	while (1)
	{
		if (ephemeris.GetOrbitState(t,EPHEMERIS_INTERP_ORDER,&os) == 0)
		{
			break;
		}

		if (os.Write(output_ephemeris) == 0)
		{
			printf("Error writing to %s\n",output_filename);
			exit(-1);
		}

		t += spacing;
	}

	//-------------//
	// close files //
	//-------------//

	fclose(output_ephemeris);
	return (0);
}
