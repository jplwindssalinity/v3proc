//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		see_slices
//
// SYNOPSIS
//		see_slices <sim_config_file> <slice_file>
//
// DESCRIPTION
//		Simulates the SeaWinds 1b instrument based on the parameters
//		in the simulation configuration file.  Generates a binary
//		vector graphics files for the slices.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//		<sim_config_file>		The sim_config_file needed listing
//								all input parameters, input files, and
//								output files.
//		<slice_file>			The binary vector graphics file for slices.
//
// EXAMPLES
//		An example of a command line is:
//			% see_slices sws1b.cfg slice.bvg
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
#include "List.h"
#include "BufferedList.h"
#include "Misc.h"
#include "ConfigList.h"
#include "Spacecraft.h"
#include "ConfigSim.h"
#include "Array.h"
#include "Meas.h"
#include "Ephemeris.h"
#include "Wind.h"

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

const char* usage_array[] = { "<sim_config_file>", "<slice_file>", 0};

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
	const char* config_file = argv[clidx++];
	const char* slice_file = argv[clidx++];

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

	//-----------------//
	// open slice file //
	//-----------------//

	FILE* slice_fp = fopen(slice_file, "w");
	if (slice_fp == NULL)
	{
		fprintf(stderr, "%s: error opening slice file %s\n", command,
			slice_file);
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

	//----------------------------------------//
	// create an attitude control error model //
	//----------------------------------------//

	if (! ConfigAttitudeControlModel(&spacecraft_sim, &config_list))
	{
		fprintf(stderr, "%s: error configuring attitude control error model\n",
			command);
		fprintf(stderr, "    for spacecraft simulator\n");
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

	//------------//
	// initialize //
	//------------//

	if (! instrument_sim.Initialize(&(instrument.antenna)))
	{
		fprintf(stderr, "%s: error initializing instrument simulator\n",
			command);
		exit(1);
	}

	//----------------------//
	// cycle through events //
	//----------------------//

	MeasSpot meas_spot;

	SpacecraftEvent spacecraft_event;
	InstrumentEvent instrument_event;
	int need_spacecraft_event  = 1;
	int need_instrument_event  = 1;

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
		if (need_instrument_event)
		{
			instrument_sim.DetermineNextEvent(&(instrument.antenna),
				&instrument_event);
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
				spacecraft_sim.UpdateOrbit(spacecraft_event.time,
					&spacecraft);
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

			if (spacecraft_event.time > instrument_sim.endTime)
				break;
		}
		else
		{
			//----------------------------------------//
			// check for end of instrument simulation //
			//----------------------------------------//

			if (instrument_event.time > instrument_sim.endTime)
			{
				// force the processing of one more spacecraft event
				instrument_event.time = spacecraft_event.time + 1.0;
				continue;
			}

			//------------------------------//
			// process the instrument event //
			//------------------------------//

			switch(instrument_event.eventId)
			{
			case InstrumentEvent::SCATTEROMETER_MEASUREMENT:
				spacecraft_sim.UpdateOrbit(instrument_event.time,
					&spacecraft);
				spacecraft_sim.UpdateAttitude(instrument_event.time,
					&spacecraft);
				instrument_sim.UpdateAntennaPosition(instrument_event.time,
					&instrument);
				instrument.antenna.currentBeamIdx = instrument_event.beamIdx;
				instrument_sim.LocateSlices(instrument_event.time,
					&spacecraft, &instrument, &meas_spot);
				for (Meas* meas = meas_spot.slices.GetHead(); meas;
					meas = meas_spot.slices.GetNext())
				{
					meas->outline.WriteBvg(slice_fp);
				}
				break;
			default:
				fprintf(stderr, "%s: unknown instrument event\n", command);
				exit(1);
				break;
			}

			need_instrument_event = 1;
		}
	}

	fclose(slice_fp);

	return (0);
}
