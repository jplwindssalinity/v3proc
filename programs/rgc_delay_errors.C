//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		rgc_delay_errors
//
// SYNOPSIS
//		rgc_delay_errors <sim_config_file> <RGC_file> <RGC_errs>
//
// DESCRIPTION
//		Generates plottable error files between the actual delay and
//		the delay calculated by the RGC.
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
//		<RGC_errs>			A plottable error file for the RGC.
//
// EXAMPLES
//		An example of a command line is:
//			% constant_errors sws1b.cfg rgc.dat rgc.errs
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

const char* usage_array[] = { "<sim_config_file>", "<RGC_file>",
	"<RGC_errs>", 0};

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

	if (argc != 4)
		usage(command, usage_array, 1);

	int arg_idx = 1;
	const char* config_file = argv[arg_idx++];
	const char* rgc_file = argv[arg_idx++];
	const char* rgc_err_file = argv[arg_idx++];

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

	//----------------------//
	// open the output file //
	//----------------------//

	FILE* rgc_err_fp = fopen(rgc_err_file, "w");
	if (rgc_err_fp == NULL)
	{
		fprintf(stderr, "%s: error opening RGC error file %s\n", command,
			rgc_err_file);
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
				int range_step;
				double ideal_delay, delay_error;
				CoordinateSwitch antenna_frame_to_gc;
				TargetInfoPackage tip;
				Vector3 vector;
				unsigned int antenna_dn, antenna_n;

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

					//------------------------------//
					// get the true round trip time //
					//------------------------------//

					antenna = &(instrument.antenna);
					beam = antenna->GetCurrentBeam();
					orbit_state = &(spacecraft.orbitState);
					attitude = &(spacecraft.attitude);

					antenna_frame_to_gc = AntennaFrameToGC(orbit_state,
						attitude, antenna);

					double look, azim;
					GetTwoWayPeakGain2(&antenna_frame_to_gc, &spacecraft,
						beam, antenna->spinRate, &look, &azim);
					vector.SphericalSet(1.0, look, azim);
					TargetInfo(&antenna_frame_to_gc, &spacecraft, &instrument,
						vector, &tip);

					//---------------------------//
					// calculate the ideal delay //
					//---------------------------//

					ideal_delay = tip.roundTripTime +
						(beam->pulseWidth - beam->receiverGateWidth) / 2.0;

					//-------------------------//
					// calculate the RGC delay //
					//-------------------------//

					range_step = range_tracker.OrbitTicksToRangeStep(
						instrument.orbitTicks);
					float delay, duration;
					antenna_dn = antenna->GetEncoderValue();
					antenna_n = antenna->GetEncoderN();
					float residual_delay_error;
					range_tracker.GetDelayAndDuration(instrument_event.beamIdx,
						range_step, beam->pulseWidth, antenna_dn, antenna_n,
						&delay, &duration, &residual_delay_error);

					//---------------------------//
					// calculate the delay error //
					//---------------------------//

					delay_error = delay - ideal_delay;
					fprintf(rgc_err_fp, "%.6f %.6f %.6f %.6f\n",
						instrument_event.time, delay_error * 1000.0,
						ideal_delay * 1000.0, delay * 1000.0);

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

	fclose(rgc_err_fp);

	return (0);
}
