//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		generate_rgc
//
// SYNOPSIS
//		generate_rgc <sim_config_file> <RGC_file>
//
// DESCRIPTION
//		Generates a set of Receiver Gate Constants based upon the
//		parameters in the simulation configuration file.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operands are supported:
//		<sim_config_file>	The sim_config_file needed listing
//								all input parameters.
//
//		<RGC_file>			The RGC output file.
//
// EXAMPLES
//		An example of a command line is:
//			% generate_constants sws1b.cfg rgc.dat
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

#define RANGE_ORBIT_STEPS		256
#define RANGE_AZIMUTH_STEPS		90		// used for fitting

#define EQX_TIME_TOLERANCE		0.1

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

const char* usage_array[] = { "<sim_config_file>", "<RGC_file>", 0 };

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

	int arg_idx = 1;
	const char* config_file = argv[arg_idx++];
	const char* rgc_file = argv[arg_idx++];

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

	//----------------------------------//
	// force RGC and DTC to not be read //
	//----------------------------------//

	config_list.StompOrAppend(USE_RGC_KEYWORD, "0");
	config_list.StompOrAppend(USE_DTC_KEYWORD, "0");

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
	spacecraft_sim.LocationToOrbit(0.0, 0.0, 1);

	//-----------------------------------------------//
	// create an instrument and instrument simulator //
	//-----------------------------------------------//

	Instrument instrument;
	if (! ConfigInstrument(&instrument, &config_list))
	{
		fprintf(stderr, "%s: error configuring instrument\n", command);
		exit(1);
	}
	Antenna* antenna = &(instrument.antenna);

	InstrumentSim instrument_sim;
	if (! ConfigInstrumentSim(&instrument_sim, &config_list))
	{
		fprintf(stderr, "%s: error configuring instrument simulator\n",
			command);
		exit(1);
	}

	//------------------------//
	// allocate range tracker //
	//------------------------//

	RangeTracker range_tracker;
	if (! range_tracker.Allocate(antenna->numberOfBeams, RANGE_ORBIT_STEPS))
	{
		fprintf(stderr, "%s: error allocating range tracker\n", command);
		exit(1);
	}

	//----------------//
	// allocate terms //
	//----------------//

	// terms are [0] = amplitude, [1] = phase, [2] = bias
	double*** terms;
	terms = (double ***)make_array(sizeof(double), 3,
		antenna->numberOfBeams, RANGE_ORBIT_STEPS, 3);

	//------------------------------//
	// start at an equator crossing //
	//------------------------------//

	double start_time =
		spacecraft_sim.FindNextArgOfLatTime(spacecraft_sim.GetEpoch(),
			EQX_ARG_OF_LAT, EQX_TIME_TOLERANCE);
	instrument.SetEqxTime(start_time);

	//------------//
	// initialize //
	//------------//

	if (! instrument_sim.Initialize(&(instrument.antenna)))
	{
		fprintf(stderr, "%s: error initializing instrument simulator\n",
			command);
		exit(1);
	}

	if (! spacecraft_sim.Initialize(start_time))
	{
		fprintf(stderr, "%s: error initializing spacecraft simulator\n",
			command);
		exit(1);
	}

	//--------------------//
	// loop through orbit //
	//--------------------//

	double orbit_period = spacecraft_sim.GetPeriod();
	double orbit_step_size = orbit_period / (double)RANGE_ORBIT_STEPS;
	double azimuth_step_size = two_pi / (double)RANGE_AZIMUTH_STEPS;

	for (int orbit_step = 0; orbit_step < RANGE_ORBIT_STEPS; orbit_step++)
	{
		//--------------------//
		// calculate the time //
		//--------------------//

		// addition of 0.5 centers on orbit_step
		double time = start_time +
			orbit_step_size * ((double)orbit_step + 0.5);

		//-----------------------//
		// locate the spacecraft //
		//-----------------------//

		spacecraft_sim.UpdateOrbit(time, &spacecraft);

		//--------------------//
		// step through beams //
		//--------------------//

		for (int beam_idx = 0; beam_idx < antenna->numberOfBeams;
			beam_idx++)
		{
			antenna->currentBeamIdx = beam_idx;

			//----------------------//
			// step through azimuth //
			//----------------------//

			double rtt[RANGE_AZIMUTH_STEPS];

			for (int azimuth_step = 0; azimuth_step < RANGE_AZIMUTH_STEPS;
				azimuth_step++)
			{
				antenna->azimuthAngle = azimuth_step_size *
					(double)azimuth_step;

				//-------------------------------------//
				// calculate the ideal round trip time //
				//-------------------------------------//

				rtt[azimuth_step] = IdealRtt(&spacecraft, &instrument);
			}

			//--------------------//
			// fit rtt parameters //
			//--------------------//

			double a, p, c;
			azimuth_fit(RANGE_AZIMUTH_STEPS, rtt, &a, &p, &c);
			*(*(*(terms + beam_idx) + orbit_step) + 0) = a;
			*(*(*(terms + beam_idx) + orbit_step) + 1) = p;
			*(*(*(terms + beam_idx) + orbit_step) + 2) = c;
		}
	}

	//-----------//
	// set delay //
	//-----------//

	range_tracker.SetRoundTripTime(terms);

	//--------------//
	// set duration //
	//--------------//

	for (int beam_idx = 0; beam_idx < antenna->numberOfBeams; beam_idx++)
	{
		antenna->currentBeamIdx = beam_idx;
		Beam* beam = antenna->GetCurrentBeam();
		range_tracker.SetDuration(beam_idx, beam->receiverGateWidth);
	}

	//---------------------//
	// set ticks per orbit //
	//---------------------//

	unsigned int ticks_per_orbit = instrument.TimeToOrbitTicks(orbit_period);
	range_tracker.SetTicksPerOrbit(ticks_per_orbit);

	//---------------------------------------//
	// write out the receiver gate constants //
	//---------------------------------------//

	if (! range_tracker.WriteBinary(rgc_file))
	{
		fprintf(stderr, "%s: error writing RGC file %s\n", command, rgc_file);
		exit(1);
	}

	return (0);
}
