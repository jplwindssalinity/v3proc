//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		gen_ephem
//
// SYNOPSIS
//		gen_ephem <sim_config_file>
//
// DESCRIPTION
//		Generates an ephemeris file based on the information in a
//		simulation configuration file.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//		<sim_config_file>		The sim_config_file needed listing
//								all input parameters, input files, and
//								output files.
//
// EXAMPLES
//		An example of a command line is:
//			% gen_ephem sws1b.cfg
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
#include "ConfigList.h"
#include "Spacecraft.h"
#include "SpacecraftSim.h"
#include "ConfigSim.h"
#include "List.h"
#include "BufferedList.h"
#include "AngleInterval.h"
#include "Meas.h"
#include "Wind.h"
#include "Tracking.h"

//-----------//
// TEMPLATES //
//-----------//


template class List<AngleInterval>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<WindVectorPlus>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;

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

	int clidx = 1;
	const char* config_file = argv[clidx++];

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

    if (! ConfigControl(&spacecraft_sim, &config_list, &grid_start_time,
        &grid_end_time, &instrument_start_time, &instrument_end_time,
        &spacecraft_start_time, &spacecraft_end_time))
    {
        fprintf(stderr, "%s: error configuring simulation times\n", command);
        exit(1);
    }

    // Set spacecraft start time to an integer multiple of ephemeris period.
    spacecraft_start_time = spacecraft_sim.GetEphemerisPeriod() *
      ((int)(spacecraft_start_time / spacecraft_sim.GetEphemerisPeriod()));


    //------------//
    // initialize //
    //------------//


    if (! spacecraft_sim.Initialize(spacecraft_start_time))
    {
        fprintf(stderr, "%s: error initializing spacecraft simulator\n",
            command);
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

	//----------------------//
	// cycle through events //
	//----------------------//

	SpacecraftEvent spacecraft_event;
	int need_spacecraft_event  = 1;

	for (;;)
	{
		//--------------------------------------//
		// determine the next appropriate event //
		//--------------------------------------//

		if (need_spacecraft_event)
		{
			spacecraft_sim.DetermineNextEvent(&spacecraft_event);
			need_spacecraft_event = 0;
		}

		//------------------------------//
		// process the spacecraft event //
		//------------------------------//

		switch(spacecraft_event.eventId)
		{
		case SpacecraftEvent::UPDATE_STATE:
			spacecraft_sim.UpdateOrbit(spacecraft_event.time,
				&spacecraft);
			spacecraft.orbitState.Write(eph_fp);
			break;
		case SpacecraftEvent::EQUATOR_CROSSING:
		        spacecraft_sim.DetermineNextEvent(&spacecraft_event);
			break;
		default:
			fprintf(stderr, "%s: unknown spacecraft event\n", command);
			exit(1);
			break;
		}

		need_spacecraft_event = 1;

		//-----------------------------//
		// check for end of simulation //
		//-----------------------------//

		if (spacecraft_event.time > spacecraft_end_time)
			break;
	}

	return (0);
}
