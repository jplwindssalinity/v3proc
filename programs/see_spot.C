//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		see_spot
//
// SYNOPSIS
//		see_spot [ -s ] <sim_config_file> <output_file>
//
// DESCRIPTION
//		Simulates the SeaWinds 1b instrument based on the parameters
//		in the simulation configuration file.  Generates a binary
//		vector graphics files for the spots.
//
// OPTIONS
//		[ -s ]		Show slices instead of spots
//
// OPERANDS
//		The following operand is supported:
//		<sim_config_file>		The sim_config_file needed listing
//								all input parameters, input files, and
//								output files.
//		<output_file>			The binary vector graphics output file.
//
// EXAMPLES
//		An example of a command line is:
//			% see_spot sws1b.cfg spot.bvg
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

#define OPTSTRING	"s"

unsigned char slice_opt = 0;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -s ]", "<sim_config_file>",
	"<output_file>", 0};

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

	int c;
	while ((c = getopt(argc, argv, OPTSTRING)) != -1)
	{
		switch(c)
		{
		case 's':
			slice_opt = 1;
			break;
		case '?':
			usage(command, usage_array, 1);
			break;
		}
	}
	if (argc != optind + 2)
		usage(command, usage_array, 1);

	const char* config_file = argv[optind++];
	const char* output_file = argv[optind++];

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

	//-------------------------------//
	// determine start and end times //
	//-------------------------------//

	double instrument_start_time, instrument_end_time;
	if (! config_list.GetDouble(INSTRUMENT_START_TIME_KEYWORD,
		&instrument_start_time))
	{
		fprintf(stderr, "%s: error getting instrument start time\n", command);
		exit(1);
	}
	if (! config_list.GetDouble(INSTRUMENT_END_TIME_KEYWORD,
		&instrument_end_time))
	{
		fprintf(stderr, "%s: error getting instrument end time\n", command);
		exit(1);
	}

	//------------------//
	// open output file //
	//------------------//

	FILE* output_fp = fopen(output_file, "w");
	if (output_fp == NULL)
	{
		fprintf(stderr, "%s: error opening output file %s\n", command,
			output_file);
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

	InstrumentEvent instrument_event;
	int instrument_done = 0;

	//-------------------------//
	// start with first events //
	//-------------------------//

	instrument_sim.DetermineNextEvent(&(instrument.antenna),
		&instrument_event);

	//---------------------//
	// loop through events //
	//---------------------//

	for (;;)
	{
		//---------------------------------------//
		// process instrument event if necessary //
		//---------------------------------------//

		if (! instrument_done)
		{
			if (instrument_event.time > instrument_end_time)
			{
				instrument_done = 1;
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
				if (slice_opt)
				{
					//--------//
					// slices //
					//--------//

					instrument_sim.LocateSlices(instrument_event.time,
						&spacecraft, &instrument, &meas_spot);
					for (Meas* meas = meas_spot.slices.GetHead(); meas;
						meas = meas_spot.slices.GetNext())
					{
						double alt, lat, lon;
						meas->center.GetAltLonGDLat(&alt, &lon, &lat);
						LonLat lon_lat;
						lon_lat.longitude = lon;
						lon_lat.latitude = lat;
						lon_lat.WriteBvg(output_fp);
						meas->outline.WriteBvg(output_fp);
					}
				}
				else
				{
					//------//
					// spot //
					//------//

					instrument_sim.LocateSpot(instrument_event.time,
						&spacecraft, &instrument, &meas_spot);
					Meas* meas = meas_spot.slices.GetHead();
					meas->outline.WriteBvg(output_fp);
				}
				instrument_sim.DetermineNextEvent(&(instrument.antenna),
					&instrument_event);
				break;
			default:
				fprintf(stderr, "%s: unknown instrument event\n", command);
				exit(1);
				break;
			}
		}

		//---------------//
		// check if done //
		//---------------//

		if (instrument_done)
			break;
	}

	fclose(output_fp);

	return (0);
}
