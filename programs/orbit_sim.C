//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		orbit_sim
//
// SYNOPSIS
//		orbit_sim <orbit_config_file>
//
// DESCRIPTION
//		Simulates an orbit based on the configuration file.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//		<orbit_config_file>		The orbital parameters
//
// EXAMPLES
//		An example of a command line is:
//			% orbit_sim orbit.cfg
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
#include "List.h"
#include "List.C"
#include "ConfigList.h"
#include "Spacecraft.h"
#include "SpacecraftSim.h"
#include "ConfigSim.h"
#include "BufferedList.h"
#include "BufferedList.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<Meas>;
template class List<LonLat>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;

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

const char* usage_array[] = { "<orbit_config_file>", 0};

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

	int clidx = 1;
	const char* config_file = argv[clidx++];

	//---------------------------//
	// read in orbit config file //
	//---------------------------//

	ConfigList config_list;
	config_list.LogErrors();
	if (! config_list.Read(config_file))
	{
		fprintf(stderr, "%s: error reading config file %s\n",
			command, config_file);
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

	//--------------------------//
	// create an ephemeris file //
	//--------------------------//

/*
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
*/

	//----------------------//
	// cycle through events //
	//----------------------//

	SpacecraftEvent spacecraft_event;

	for (;;)
	{
		spacecraft_sim.DetermineNextEvent(&spacecraft_event);

		switch(spacecraft_event.eventId)
		{
		case SpacecraftEvent::UPDATE_STATE:
			spacecraft_sim.UpdateOrbit(spacecraft_event.time,
				&spacecraft);
			double alt, lat, lon;
			spacecraft.orbitState.rsat.GetAltLatLon(EarthPosition::GEOCENTRIC,
				&alt,&lat,&lon);
			printf("%g %g\n", spacecraft.orbitState.time, alt);
//			spacecraft.orbitState.Write(eph_fp);
			break;
		default:
			fprintf(stderr, "%s: unknown spacecraft event\n", command);
			exit(1);
			break;
		}
		if (spacecraft.orbitState.time > 6300.0)
			break;
	}

	return (0);
}
