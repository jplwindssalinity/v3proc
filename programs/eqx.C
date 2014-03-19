//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		eqx
//
// SYNOPSIS
//		eqx <sim_config_file> <time>
//
// DESCRIPTION
//		Prints the times of the two nearest equator crossing times
//		to a specified time.  Also prints the orbit period.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operands are supported:
//		<sim_config_file>	The sim_config_file needed listing
//								all input parameters, input files, and
//								output files.
//
//		<time>				The reference time for equator crossings.
//
// EXAMPLES
//		An example of a command line is:
//			% eqx sws1b.cfg 1000.0
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
#include "ConfigList.h"
#include "SpacecraftSim.h"
#include "ConfigSim.h"
#include "Meas.h"
#include "List.h"
#include "BufferedList.h"
#include "Tracking.h"

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<AngleInterval>;
template class List<Meas>;
template class List<long>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class List<OffsetList>;
template class List<StringPair>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;
template class List<EarthPosition>;
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

const char* usage_array[] = { "<sim_config_file>", "<time>", 0};

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

	int arg_idx = 1;
	const char* config_file = argv[arg_idx++];
	float time = atof(argv[arg_idx++]);

	//--------------------------------//
	// read in simulation config file //
	//--------------------------------//

	ConfigList config_list;
	if (! config_list.Read(config_file))
	{
		fprintf(stderr, "%s: error reading sim config file %s\n",
			command, config_file);
		exit(1);
	}

	//-------------------------------//
	// create a spacecraft simulator //
	//-------------------------------//

	SpacecraftSim spacecraft_sim;
	if (! ConfigSpacecraftSim(&spacecraft_sim, &config_list))
	{
		fprintf(stderr, "%s: error configuring spacecraft simulator\n",
			command);
		exit(1);
	}

	//-----------------------------//
	// find equator crossing times //
	//-----------------------------//

	double eqx_1 =
		spacecraft_sim.FindPrevArgOfLatTime(time, EQX_ARG_OF_LAT,
		EQX_TIME_TOLERANCE);

	double eqx_2 =
		spacecraft_sim.FindNextArgOfLatTime(time, EQX_ARG_OF_LAT,
		EQX_TIME_TOLERANCE);

	//--------//
	// output //
	//--------//

	printf(" EQX 1: %g\n", eqx_1);
	printf(" EQX 2: %g\n", eqx_2);
	printf("Period: %g\n", spacecraft_sim.GetPeriod());

	return (0);
}
