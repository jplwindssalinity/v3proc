//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    see_spot
//
// SYNOPSIS
//    see_spot [ -d dB ] [ -cs ] <sim_config_file> <output_file>
//
// DESCRIPTION
//    Simulates the SeaWinds 1b instrument based on the parameters
//    in the simulation configuration file.  Generates a binary
//    vector graphics files for the spots.
//
// OPTIONS
//    [ -d dB ]  dB level for contours (defaults to 3.0)
//    [ -c ]     Show the centroid
//    [ -s ]     Show slices instead of spots
//
// OPERANDS
//    The following operand is supported:
//      <sim_config_file>  The sim_config_file needed listing
//                         all input parameters, input files, and
//                         output files.
//      <output_file>o     The binary vector graphics output file.
//
// EXAMPLES
//    An example of a command line is:
//      % see_spot -c sws1b.cfg spot.bvg
//
// ENVIRONMENT
//    Not environment dependent.
//
// EXIT STATUS
//    The following exit values are returned:
//     0  Program executed successfully
//    >0  Program had an error
//
// NOTES
//    None.
//
// AUTHOR
//    James N. Huddleston
//    hudd@casket.jpl.nasa.gov
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
#include <unistd.h>
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
#include "Tracking.h"
#include "Tracking.C"
#include "Qscat.h"
#include "QscatConfig.h"
#include "QscatConfigDefs.h"

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<AngleInterval>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<WindVectorPlus>;
template class List<long>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

//-----------//
// CONSTANTS //
//-----------//

#define DEFAULT_DB_LEVEL  -3.0

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

#define OPTSTRING  "cd:s"

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
    int    argc,
    char*  argv[])
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
	if (! ConfigSpacecraft(&spacecraft, &config_list))
	{
		fprintf(stderr, "%s: error configuring spacecraft simulator\n",
			command);
		exit(1);
	}

	SpacecraftSim spacecraft_sim;
	if (! ConfigSpacecraftSim(&spacecraft_sim, &config_list))
	{
		fprintf(stderr, "%s: error configuring spacecraft simulator\n",
			command);
		exit(1);
	}

    //--------------------------------------//
    // create a QSCAT and a QSCAT simulator //
    //--------------------------------------//

    Qscat qscat;
    if (! ConfigQscat(&qscat, &config_list))
    {
        fprintf(stderr, "%s: error configuring QSCAT\n", command);
        exit(1);
    }

    // hack in the pri because the simulated CDS puts restrictions
    // on the range of available pri's
    float pri;
    if (! config_list.GetFloat(PRI_KEYWORD, &pri))
        return(0);
    qscat.ses.pri = pri;

    QscatSim qscat_sim;
    if (! ConfigQscatSim(&qscat_sim, &config_list))
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
	qscat_sim.startTime = instrument_start_time;

	//------------//
	// initialize //
	//------------//

	if (! qscat_sim.Initialize(&qscat))
	{
		fprintf(stderr, "%s: error initializing QSCAT simulator\n",
			command);
		exit(1);
	}

	if (! spacecraft_sim.Initialize(spacecraft_start_time))
	{
		fprintf(stderr, "%s: error initializing spacecraft simulator\n",
			command);
		exit(1);
	}

	//---------------------------//
	// set the previous Eqx time //
	//---------------------------//

	double eqx_time =
		spacecraft_sim.FindPrevArgOfLatTime(instrument_start_time,
			EQX_ARG_OF_LAT, EQX_TIME_TOLERANCE);
	qscat.cds.SetEqxTime(eqx_time);

	//----------------------//
	// cycle through events //
	//----------------------//

	MeasSpot meas_spot;

	QscatEvent qscat_event;
	int instrument_done = 0;

	//-------------------------//
	// start with first events //
	//-------------------------//

	qscat_sim.DetermineNextEvent(100, &qscat, &qscat_event);

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
			if (qscat_event.time > instrument_end_time)
			{
				instrument_done = 1;
				continue;
			}

			//------------------------------//
			// process the instrument event //
			//------------------------------//

			switch(qscat_event.eventId)
			{
			case QscatEvent::SCAT_EVENT:

				// process spacecraft stuff
				spacecraft_sim.UpdateOrbit(qscat_event.time,
					&spacecraft);
				spacecraft_sim.UpdateAttitude(qscat_event.time,
					&spacecraft);

				// process instrument stuff
				qscat.cds.SetTime(qscat_event.time);
                qscat.cds.currentBeamIdx = qscat_event.beamIdx;

                // antenna
                qscat.sas.antenna.UpdatePosition(qscat_event.time);
                qscat.SetOtherAzimuths(&spacecraft);

				//-----------------------------------------------//
				// command the range delay and Doppler frequency //
				//-----------------------------------------------//

                SetOrbitStepDelayAndFrequency(&spacecraft, &qscat);

				if (slice_opt)
				{
					//--------//
					// slices //
					//--------//

					qscat.LocateSlices(&spacecraft, &meas_spot);
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

					qscat.LocateSpot(&spacecraft, &meas_spot, contour_level);
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
				break;
			default:
//				fprintf(stderr, "%s: unknown instrument event\n", command);
//				exit(1);
				break;
			}
            meas_spot.FreeContents();
            qscat_sim.DetermineNextEvent(100, &qscat, &qscat_event);
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
