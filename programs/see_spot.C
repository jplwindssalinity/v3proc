//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		see_spot
//
// SYNOPSIS
//		see_spot [ -d dB ] [ -cs ] <sim_config_file> <output_file>
//
// DESCRIPTION
//		Simulates the SeaWinds 1b instrument based on the parameters
//		in the simulation configuration file.  Generates a binary
//		vector graphics files for the spots.
//
// OPTIONS
//		[ -d dB ]	dB level for contours (defaults to 3.0)
//		[ -c ]		Show the centroid
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
//			% see_spot -c sws1b.cfg spot.bvg
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
#include "InstrumentGeom.h"

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

#define DEFAULT_DB_LEVEL	-3.0

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
float contour_level = pow(10.0, 0.1 * DEFAULT_DB_LEVEL);

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -d dB ]", "[ -cs ]", "<sim_config_file>",
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
	extern char *optarg;
	extern int optind;

	float db_level;
	int c;
	while ((c = getopt(argc, argv, OPTSTRING)) != -1)
	{
		switch(c)
		{
		case 'c':
			centroid_opt = 1;
			break;
		case 'd':
			db_level = -fabs(atof(optarg));
			contour_level = pow(10.0, 0.1 * db_level);
			break;
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
	if (! config_list.Read(config_file))
	{
		fprintf(stderr, "%s: error reading sim config file %s\n",
			command, config_file);
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
	char* hdr = OTLN_HEADER;
	if (fwrite((void *)hdr, 4, 1, output_fp) != 1)
	{
		fprintf(stderr, "%s: error writing header to output file %s\n",
			command, output_file);
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
	instrument_sim.startTime = instrument_start_time;

	//------------//
	// initialize //
	//------------//

	if (! instrument_sim.Initialize(&(instrument.antenna)))
	{
		fprintf(stderr, "%s: error initializing instrument simulator\n",
			command);
		exit(1);
	}

	//---------------------------//
	// set the previous Eqx time //
	//---------------------------//

	double eqx_time =
		spacecraft_sim.FindPrevArgOfLatTime(instrument_start_time,
			EQX_ARG_OF_LAT, EQX_TIME_TOLERANCE);
	instrument.SetEqxTime(eqx_time);

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

				// process spacecraft stuff
				spacecraft_sim.UpdateOrbit(instrument_event.time,
					&spacecraft);
				spacecraft_sim.UpdateAttitude(instrument_event.time,
					&spacecraft);

				// process instrument stuff
				instrument.SetTime(instrument_event.time);
				instrument_sim.UpdateAntennaPosition(&instrument);
				instrument.antenna.currentBeamIdx = instrument_event.beamIdx;

				//-------------------------------------------------------//
				// command the range delay, width, and Doppler frequency //
				//-------------------------------------------------------//

				SetRangeAndDoppler(&spacecraft, &instrument);

				if (slice_opt)
				{
					//--------//
					// slices //
					//--------//

					LocateSlices(&spacecraft, &instrument, &meas_spot);
					for (Meas* meas = meas_spot.GetHead(); meas;
						meas = meas_spot.GetNext())
					{
						if (centroid_opt)
						{
							double alt, lat, lon;
							meas->centroid.GetAltLonGDLat(&alt, &lon, &lat);
							LonLat lon_lat;
							lon_lat.longitude = lon;
							lon_lat.latitude = lat;
							lon_lat.WriteOtln(output_fp);
						}
						meas->outline.WriteOtln(output_fp);
					}
				}
				else
				{
					//------//
					// spot //
					//------//

					LocateSpot(&spacecraft, &instrument, &meas_spot,
						contour_level);
					Meas* meas = meas_spot.GetHead();
					if (centroid_opt)
					{
						double alt, lat, lon;
						meas->centroid.GetAltLonGDLat(&alt, &lon, &lat);
						LonLat lon_lat;
						lon_lat.longitude = lon;
						lon_lat.latitude = lat;
						lon_lat.WriteOtln(output_fp);
					}
					meas->outline.WriteOtln(output_fp);
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
