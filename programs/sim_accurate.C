//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		sim_accurate
//
// SYNOPSIS
//		sim_accurate <sim_config_file>
//
// DESCRIPTION
//		Simulates the SeaWinds 1b instrument based on the parameters
//		in the simulation configuration file. Performs a very
//		accurate simulation by integrating over each slice to
//		calculate X.
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
//			% sim_accurate sws1b.cfg
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
//		James N. Huddleston and Bryan Stiles
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
#include "QscatConfig.h"
#include "Array.h"
#include "Meas.h"
#include "Ephemeris.h"
#include "Wind.h"
#include "QscatSimAccurate.h"
#include "Tracking.h"
#include "Tracking.C"

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



	//-----------------------------------------------//
	// create an instrument and instrument simulator //
	//-----------------------------------------------//

	Qscat qscat;
	if (! ConfigQscat(&qscat, &config_list))
	{
		fprintf(stderr, "%s: error configuring QSCAT\n", command);
		exit(1);
	}

	QscatSimAccurate qscat_sim;
	if (! ConfigQscatSimAccurate(&qscat_sim, &config_list))
	{
		fprintf(stderr, "%s: error configuring instrument simulator\n",
			command);
		exit(1);
	}

	//----------------------------//
	// create a Level 0.0 product //
	//----------------------------//

	L1A l1a;
	if (! ConfigL1A(&l1a, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 0.0\n", command);
		exit(1);
	}
	l1a.OpenForWriting();

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

	//------------------//
	// set the eqx time //
	//------------------//

	double start_time;
	start_time = spacecraft_sim.FindPrevArgOfLatTime(instrument_start_time,
		EQX_ARG_OF_LAT, EQX_TIME_TOLERANCE);
	qscat.cds.SetEqxTime(start_time);

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

	//----------------------//
	// cycle through events //
	//----------------------//

	SpacecraftEvent spacecraft_event;
	spacecraft_event.time = spacecraft_start_time;

	QscatEvent qscat_event;
	qscat_event.time = instrument_start_time;

	int spacecraft_done = 0;
	int instrument_done = 0;

	//-------------------------//
	// start with first events //
	//-------------------------//

	spacecraft_sim.DetermineNextEvent(&spacecraft_event);
	qscat_sim.DetermineNextEvent(&qscat, &qscat_event);

	//---------------------//
	// loop through events //
	//---------------------//

	for (;;)
	{
		//---------------------------------------//
		// process spacecraft event if necessary //
		//---------------------------------------//

		if (! spacecraft_done)
		{
			if (spacecraft_event.time > spacecraft_end_time)
			{
				spacecraft_done = 1;
				continue;
			}
			if (spacecraft_event.time <= qscat_event.time ||
				instrument_done)
			{
				//------------------------------//
				// process the spacecraft event //
				//------------------------------//

				switch(spacecraft_event.eventId)
				{
				case SpacecraftEvent::UPDATE_STATE:
					spacecraft_sim.UpdateOrbit(spacecraft_event.time,
						&spacecraft);
					spacecraft.orbitState.Write(eph_fp);
					spacecraft_sim.DetermineNextEvent(&spacecraft_event);
					break;			
				case SpacecraftEvent::EQUATOR_CROSSING:
				        qscat.cds.SetEqxTime(spacecraft_event.time);
					spacecraft_sim.DetermineNextEvent(&spacecraft_event);
					break;

				default:
					fprintf(stderr, "%s: unknown spacecraft event\n", command);
					exit(1);
					break;
				}
			}
		}

		//----------------------------------//
		// process QSCAT event if necessary //
		//----------------------------------//

		if (! instrument_done)
		{
			if (qscat_event.time > instrument_end_time)
			{
				instrument_done = 1;
				continue;
			}
			if (qscat_event.time <= spacecraft_event.time ||
				spacecraft_done)
			{
				//-------------------------//
				// process the QSCAT event //
				//-------------------------//

				switch(qscat_event.eventId)
				{
				case QscatEvent::SCATTEROMETER_MEASUREMENT:

					// process spacecraft stuff
					spacecraft_sim.UpdateOrbit(qscat_event.time,
						&spacecraft);
					spacecraft_sim.UpdateAttitude(qscat_event.time,
						&spacecraft);

					// process instrument stuff
					qscat.cds.SetTime(qscat_event.time);
                    qscat.sas.antenna.UpdatePosition(qscat_event.time);
                    qscat.cds.currentBeamIdx = qscat_event.beamIdx;
					qscat_sim.ScatSim(&spacecraft, &qscat,
						&windfield, &gmf, &(l1a.frame));
					qscat_sim.DetermineNextEvent(&qscat, &qscat_event);
					break;
				default:
					fprintf(stderr, "%s: unknown QSCAT event\n", command);
					exit(1);
					break;
				}
			}

			//-----------------------------------//
			// write Level 0.0 data if necessary //
			//-----------------------------------//

			if (qscat_sim.l1aFrameReady)
			{
				// Report Latest Attitude Measurement
				// + Knowledge Error
				spacecraft_sim.ReportAttitude(qscat_event.time,
					&spacecraft, &(l1a.frame.attitude));

				int size = l1a.frame.Pack(l1a.buffer);
				l1a.Write(l1a.buffer, size);
			}
		}

		//---------------//
		// check if done //
		//---------------//

		if (instrument_done && spacecraft_done)
			break;
	}

	//----------------------//
	// close Level 0.0 file //
	//----------------------//

	l1a.Close();

	//--------------------------//
	// If createXtable is set	//
	// write XTABLE file		//
	//--------------------------//

	if(qscat_sim.createXtable)
	{
		qscat_sim.xTable.Write();
	}

	return (0);
}
