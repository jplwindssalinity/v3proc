//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		baseband_errors
//
// SYNOPSIS
//		baseband_errors <sim_config_file> <RGC_file> <DTC_file>
//			<error_file>
//
// DESCRIPTION
//		Generates plottable error file showing the baseband frequency
//		error between the actual peak 2-way gain baseband frequency
//		and the algorithmic frequency.
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
//		<RGC_file>			The RGC input file.
//
//		<DTC_file>			The DTC input file.
//
//		<error_file>		A plottable error file.
//
// EXAMPLES
//		An example of a command line is:
//			% baseband_errors sws1b.cfg rgc.dat dtc.dat baseband.err
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
#include "List.h"
#include "List.C"
#include "Spacecraft.h"
#include "ConfigSim.h"
#include "Tracking.h"
#include "InstrumentGeom.h"
#include "BufferedList.h"
#include "BufferedList.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<EarthPosition>;
template class List<StringPair>;
template class List<Meas>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class List<long>;
template class List<OffsetList>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;

//-----------//
// CONSTANTS //
//-----------//

#define EQX_TIME_TOLERANCE	0.1

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

const char* usage_array[] = { "<sim_config_file>", "<RGC_file>", "<DTC_file>",
	"<error_file>", 0};

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

	if (argc != 5)
		usage(command, usage_array, 1);

	int arg_idx = 1;
	const char* config_file = argv[arg_idx++];
	const char* rgc_file = argv[arg_idx++];
	const char* dtc_file = argv[arg_idx++];
	const char* error_file = argv[arg_idx++];

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

	//------------------------------------------//
	// create an attitude knowledge error model //
	//------------------------------------------//

	if (! ConfigAttitudeKnowledgeModel(&spacecraft_sim, &config_list))
	{
		fprintf(stderr,
			"%s: error configuring attitude knowledge error model\n", command);
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

	//------------------//
	// set the eqx time //
	//------------------//

	double eqx_time = spacecraft_sim.FindPrevEqxTime(instrument_start_time,
		EQX_TIME_TOLERANCE);
	instrument.Eqx(eqx_time);

	//------------//
	// initialize //
	//------------//

	if (! instrument_sim.Initialize(&(instrument.antenna)))
	{
		fprintf(stderr, "%s: error initializing instrument simulator\n",
			command);
		exit(1);
	}

	if (! spacecraft_sim.Initialize(spacecraft_start_time))
	{
		fprintf(stderr, "%s: error initializing spacecraft simulator\n",
			command);
		exit(1);
	}

	//--------------//
	// read the RGC //
	//--------------//

	RangeTracker range_tracker;
	if (! range_tracker.ReadBinary(rgc_file))
	{
		fprintf(stderr, "%s: error reading RGC file %s\n", command, rgc_file);
		exit(1);
	}

	//--------------//
	// read the DTC //
	//--------------//

	DopplerTracker doppler_tracker;
	if (! doppler_tracker.ReadBinary(dtc_file))
	{
		fprintf(stderr, "%s: error reading DTC file %s\n", command, dtc_file);
		exit(1);
	}

	//----------------------//
	// open the output file //
	//----------------------//

	FILE* error_fp = fopen(error_file, "w");
	if (error_fp == NULL)
	{
		fprintf(stderr, "%s: error opening error file %s\n", command,
			error_file);
		exit(1);
	}

	//----------------------//
	// cycle through events //
	//----------------------//

	SpacecraftEvent spacecraft_event;
	spacecraft_event.time = spacecraft_start_time;

	InstrumentEvent instrument_event;
	instrument_event.time = instrument_start_time;

	int spacecraft_done = 0;
	int instrument_done = 0;

	//-------------------------//
	// start with first events //
	//-------------------------//

	spacecraft_sim.DetermineNextEvent(&spacecraft_event);
	instrument_sim.DetermineNextEvent(&(instrument.antenna),
		&instrument_event);

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
			if (spacecraft_event.time <= instrument_event.time ||
				instrument_done)
			{
				//------------------------------//
				// process the spacecraft event //
				//------------------------------//

				switch(spacecraft_event.eventId)
				{
				case SpacecraftEvent::EQUATOR_CROSSING:
					instrument.Eqx(spacecraft_event.time);
					break;
				default:
					break;
				}
				spacecraft_sim.DetermineNextEvent(&spacecraft_event);
			}
		}

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
			if (instrument_event.time <= spacecraft_event.time ||
				spacecraft_done)
			{
				//------------------------------//
				// process the instrument event //
				//------------------------------//

				Antenna* antenna;
				Beam* beam;
				OrbitState* orbit_state;
				Attitude* attitude;
				CoordinateSwitch antenna_frame_to_gc;
				Vector3 rlook_antenna;
				TargetInfoPackage tip;
				double ideal_dopcom;
				float residual_delay_error;

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
					instrument.antenna.currentBeamIdx =
						instrument_event.beamIdx;

					//---------------------------//
					// set up the range tracking //
					//---------------------------//

					residual_delay_error = 0.0;
					range_tracker.SetInstrument(&instrument,
						&residual_delay_error);

					//---------------------------------------//
					// determine the ideal commanded doppler //
					//---------------------------------------//

					IdealCommandedDoppler(&spacecraft, &instrument);
					ideal_dopcom = instrument.commandedDoppler;

					//-----------------------------//
					// set up the doppler tracking //
					//-----------------------------//

					doppler_tracker.SetInstrument(&instrument,
						residual_delay_error);

					//----------------------------------------//
					// get the beam center baseband frequency //
					//----------------------------------------//

					antenna = &(instrument.antenna);
					beam = antenna->GetCurrentBeam();
					orbit_state = &(spacecraft.orbitState);
					attitude = &(spacecraft.attitude);

					antenna_frame_to_gc = AntennaFrameToGC(orbit_state,
						attitude, antenna);

					double look, azimuth;
					GetTwoWayPeakGain2(&antenna_frame_to_gc, &spacecraft,
						beam, antenna->spinRate, &look, &azimuth);

					rlook_antenna.SphericalSet(1.0, look, azimuth);
					TargetInfo(&antenna_frame_to_gc, &spacecraft, &instrument,
						rlook_antenna, &tip);

					//------------------------------//
					// calculate the baseband error //
					//------------------------------//

					// actually, any deviation from zero Hz is undesirable
					fprintf(error_fp, "%.6f %.6f %.6f %.6f\n",
						instrument_event.time, tip.basebandFreq,
						instrument.commandedDoppler, ideal_dopcom);

					instrument_sim.DetermineNextEvent(&(instrument.antenna),
						&instrument_event);
					break;

				default:
					fprintf(stderr, "%s: unknown instrument event\n", command);
					exit(1);
					break;
				}
			}
		}

		//---------------//
		// check if done //
		//---------------//

		if (instrument_done && spacecraft_done)
			break;
	}

	//-------------------//
	// close error files //
	//-------------------//

	fclose(error_fp);

	return (0);
}
