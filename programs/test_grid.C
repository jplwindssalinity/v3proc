//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		test_grid
//
// SYNOPSIS
//		test_grid <sim_config_file>
//
// DESCRIPTION
//
// OPTIONS
//
// OPERANDS
//		The following operand is supported:
//		<sim_config_file>		The sim_config_file needed listing
//								all input parameters, input files, and
//								output files.
//
// EXAMPLES
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
#include "InstrumentGeom.h"
#include "EarthPosition.h"
#include "SpacecraftSim.h"

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

#define OPTSTRING	"cd:s"

unsigned char centroid_opt = 0;
unsigned char slice_opt = 0;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<sim_config_file>", 0};

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

	if (argc != 2)
		usage(command, usage_array, 1);

	const char* config_file = argv[optind++];

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

	//--------------------------//
	// create an ephemeris file //
	//--------------------------//

	char* epehemeris_filename;
	epehemeris_filename = config_list.Get(EPHEMERIS_FILE_KEYWORD);
	if (! epehemeris_filename)
	{
		fprintf(stderr, "%s: error getting ephemeris filename\n", command);
		exit(1);
	}
	FILE* eph_fp = fopen(epehemeris_filename, "w");
	if (eph_fp == NULL)
	{
		fprintf(stderr, "%s: error opening ephemeris file %s\n", command,
			epehemeris_filename);
		exit(1);
	}

	//----------------------------------------------//
	// create a spacecraft and spacecraft simulator //
	//----------------------------------------------//

	Spacecraft spacecraft;

	SpacecraftSim spacecraft_sim;
	if (! ConfigSpacecraftSim(&spacecraft_sim, &config_list))
	{
		fprintf(stderr, "%s: error configuring spacecraft simulator\n",
			command);
		exit(1);
	}

	//---------------------//
	// configure the times //
	//---------------------//
 
	double grid_start_time, grid_end_time;
	double instrument_start_time, instrument_end_time;
	double spacecraft_start_time, spacecraft_end_time;
 
	if (! ConfigControl(&spacecraft_sim, &config_list,
		&grid_start_time, &grid_end_time,
		&instrument_start_time, &instrument_end_time,
		&spacecraft_start_time, &spacecraft_end_time))
	{
		fprintf(stderr, "%s: error configuring simulation times\n", command);
		exit(1);
	}

	grid_end_time = grid_start_time + 151*60;
	printf("grid start,end time = %g %g\n",grid_start_time,grid_end_time);

	//---------------------//
	// make ephemeris file //
	//---------------------//

	double time;
	printf("making ephemeris file...");
	for (time = grid_start_time-120.0; time < grid_end_time+600.0; time += 60)
	{
		// process spacecraft stuff
		spacecraft_sim.UpdateOrbit(time,
			&spacecraft);
		spacecraft.orbitState.Write(eph_fp);
	}
	fclose(eph_fp);
	printf("done\n");

    //--------------------------------//
    // create and configure ephemeris //
    //--------------------------------//

    Ephemeris ephemeris;
    if (! ConfigEphemeris(&ephemeris, &config_list))
    {
        fprintf(stderr, "%s: error configuring ephemeris\n", command);
        exit(1);
    }

	EarthPosition start_position;
    if (ephemeris.GetPosition(grid_start_time,EPHEMERIS_INTERP_ORDER,
        &start_position) == 0)
    {
        printf("Error: Grid start time is out of ephemeris range\n");
        exit(-1);
    }

	//---------------------//
	// loop through events //
	//---------------------//

	EarthPosition rold = start_position;

	for (time = grid_start_time; time < grid_end_time; time += 5)
	{
		// process spacecraft stuff
		spacecraft_sim.UpdateOrbit(time,
			&spacecraft);

		EarthPosition r = spacecraft.orbitState.rsat.Nadir();
		float atd,ctd;
		ephemeris.GetSubtrackCoordinates(r,start_position,grid_start_time,
			time,&ctd,&atd);
		float arclen = rold.surface_distance(r);
		printf("%g %g %g %g\n",time,ctd,atd,arclen);
		rold = r;
				
	}

	return (0);
}
