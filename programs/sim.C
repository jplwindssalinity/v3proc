//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		sim
//
// SYNOPSIS
//		sim <sim_config_file>
//
// DESCRIPTION
//		Simulates the SeaWinds 1b instrument based on the parameters
//		in the simulation configuration file.
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
//			% sim sws1b.cfg
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
#include <fcntl.h>
#include "Misc.h"
#include "ConfigList.h"
#include "Spacecraft.h"
#include "ConfigSim.h"
/*
#include "Instrument.h"
#include "L00File.h"
#include "L00Frame.h"
*/

//-----------//
// CONSTANTS //
//-----------//

// these probably belong somewhere else
#define SCATTEROMETER_BEAM_A_INDEX		0
#define SCATTEROMETER_BEAM_B_INDEX		1

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
	config_list.LogErrors();
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
	if (! ConfigSpacecraft(&spacecraft, &config_list))
	{
		fprintf(stderr, "%s: error configuring spacecraft\n", command);
		exit(1);
	}

	SpacecraftSim spacecraft_sim;
	if (! ConfigSpacecraftSim(&spacecraft_sim, &config_list))
	{
		fprintf(stderr, "%s: error configuring spacecraft simulator\n",
			command);
		exit(1);
	}

	//-----------------------------------------------//
	// create an instrument and instrument simulator //
	//-----------------------------------------------//

	Instrument instrument;
	if (! ConfigInstrument(&instrument, &config_list))
	{
		fprintf(stderr, "%s: error configuring instrument\n", command);
		exit(1);
	}

	InstrumentSim instrument_sim;
	if (! ConfigInstrumentSim(&instrument_sim, &config_list))
	{
		fprintf(stderr, "%s: error configuring instrument simulator\n",
			command);
		exit(1);
	}

	//-------------------------//
	// create a Level 0.0 file //
	//-------------------------//

	L00File l00_file;
	if (! ConfigL00File(&l00_file, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 0.0\n", command);
		exit(1);
	}
	l00_file.OpenForOutput();

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

	//--------------------//
	// read the windfield //
	//--------------------//

	WindField windfield;
	if (! ConfigWindField(&windfield, &config_list))
	{
		fprintf(stderr, "%s: error configuring wind field\n", command);
		exit(1);
	}

	//-------------------------------------//
	// read the geophysical model function //
	//-------------------------------------//

	GMF gmf;
	if (! ConfigGMF(&gmf, &config_list))
	{
		fprintf(stderr, "%s: error configuring GMF\n", command);
		exit(1);
	}

	//----------------------//
	// cycle through events //
	//----------------------//

	L00Frame l00_frame;
	char l00_buffer[MAX_L00_BUFFER_SIZE];

	SpacecraftEvent spacecraft_event;
	InstrumentEvent instrument_event;
	int need_spacecraft_event  = 1;
	int need_instrument_event  = 1;

	OrbitState* orbit_state;

	while (instrument_event.time < 120.0)
	{
		//--------------------------------------//
		// determine the next appropriate event //
		//--------------------------------------//

		if (need_spacecraft_event)
		{
			spacecraft_sim.DetermineNextEvent(&spacecraft_event);
			need_spacecraft_event = 0;
		}
		if (need_instrument_event)
		{
			instrument_sim.DetermineNextEvent(&instrument_event);
			need_instrument_event = 0;
		}

		//--------------------------------------//
		// select earliest event for processing //
		//--------------------------------------//

		if (spacecraft_event.time <= instrument_event.time)
		{
			//------------------------------//
			// process the spacecraft event //
			//------------------------------//

			switch(spacecraft_event.eventId)
			{
			case SpacecraftEvent::UPDATE_STATE:
				spacecraft_sim.UpdateOrbit(instrument_event.time,
					&spacecraft);
				spacecraft.orbitState.Write(eph_fp);
				break;
			default:
				fprintf(stderr, "%s: unknown spacecraft event\n", command);
				exit(1);
				break;
			}

			need_spacecraft_event = 1;
		}
		else
		{
			//------------------------------//
			// process the instrument event //
			//------------------------------//

			switch(instrument_event.eventId)
			{
			case InstrumentEvent::SCATTEROMETER_BEAM_A_MEASUREMENT:
				orbit_state = &(spacecraft.orbitState);
				spacecraft_sim.UpdateOrbit(instrument_event.time,
					&spacecraft);
				instrument_sim.ScatSim(instrument_event.time, orbit_state,
					&instrument_sim, &instrument, SCATTEROMETER_BEAM_A_INDEX,
					&windfield, &gmf);
				break;
			case InstrumentEvent::SCATTEROMETER_BEAM_B_MEASUREMENT:
				orbit_state = &(spacecraft.orbitState);
				spacecraft_sim.UpdateOrbit(instrument_event.time,
					&spacecraft);
				instrument_sim.ScatSim(instrument_event.time, orbit_state,
					&instrument_sim, &instrument, SCATTEROMETER_BEAM_A_INDEX,
					&windfield, &gmf);
				break;
			default:
				fprintf(stderr, "%s: unknown instrument event\n", command);
				exit(1);
				break;
			}

			//----------------------//
			// write Level 0.0 data //
			//----------------------//

			if (instrument_sim.l00FrameReady)
			{
				int size = instrument_sim.l00Frame.Pack(l00_buffer);
				l00_file.Write(l00_buffer, size);
			}

			need_instrument_event = 1;
		}
	}

	//----------------------//
	// close Level 0.0 file //
	//----------------------//

	l00_file.Close();

	return (0);
}
