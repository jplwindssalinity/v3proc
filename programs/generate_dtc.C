//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		generate_dtc
//
// SYNOPSIS
//		generate_dtc <sim_config_file> <RGC_file> <DTC_file>
//
// DESCRIPTION
//		Generates a set of Doppler Tracking Constants based upon the
//		parameters in the simulation configuration file and the given
//		Receiver Gate Constants.
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
//		<DTC_file>			The DTC output file.
//
// EXAMPLES
//		An example of a command line is:
//			% generate_constants sws1b.cfg rgc.dat dtc.dat
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
template class List<OrbitState>;
template class BufferedList<OrbitState>;

//-----------//
// CONSTANTS //
//-----------//

#define DOPPLER_ORBIT_STEPS		256
#define DOPPLER_AZIMUTH_STEPS	360		// used for fitting

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
	"<DTC_FILE>", 0};

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
	const char* dtc_file = argv[arg_idx++];

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

	//--------------------//
	// read range tracker //
	//--------------------//

	RangeTracker range_tracker;
	if (! range_tracker.ReadBinary(rgc_file))
	{
		fprintf(stderr, "%s: error reading RGC file %s\n", command,
			rgc_file);
		exit(1);
	}

	//--------------------------//
	// allocate Doppler tracker //
	//--------------------------//

	DopplerTracker doppler_tracker;
	if (! doppler_tracker.Allocate(antenna->numberOfBeams,
		DOPPLER_ORBIT_STEPS))
	{
		fprintf(stderr, "%s: error allocating Doppler tracker\n",
			command);
	}

	//----------------//
	// allocate terms //
	//----------------//

	// terms are [0] = amplitude, [1] = phase, [2] = bias
	double*** terms;
	terms = (double ***)make_array(sizeof(double), 3,
		antenna->numberOfBeams, DOPPLER_ORBIT_STEPS, 3);

	//-----------------------//
	// mark equator crossing //
	//-----------------------//

	double start_time = spacecraft_sim.GetEpoch();
	double orbit_period = spacecraft_sim.GetPeriod();

	start_time += orbit_period / 2.0;
	start_time = spacecraft_sim.NextEqxTime(start_time, EQX_TIME_TOLERANCE);
	instrument.Eqx(start_time);

	//------------//
	// initialize //
	//------------//

	if (! instrument_sim.Initialize(&(instrument.antenna)))
	{
		fprintf(stderr, "%s: error initializing instrument simulator\n",
			command);
		exit(1);
	}
	instrument.Eqx(start_time);

	if (! spacecraft_sim.Initialize(start_time))
	{
		fprintf(stderr, "%s: error initializing spacecraft simulator\n",
			command);
		exit(1);
	}

	//-----------//
	// variables //
	//-----------//

	OrbitState* orbit_state = &(spacecraft.orbitState);
	Attitude* attitude = &(spacecraft.attitude);
	Vector3 vector;

	//--------------------//
	// loop through orbit //
	//--------------------//

	double orbit_step_size = orbit_period / (double)DOPPLER_ORBIT_STEPS;
	double azimuth_step_size = two_pi / (double)DOPPLER_AZIMUTH_STEPS;

	for (int orbit_step = 0; orbit_step < DOPPLER_ORBIT_STEPS; orbit_step++)
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
			Beam* beam = antenna->GetCurrentBeam();

			//--------------------------------//
			// calculate baseband frequencies //
			//--------------------------------//

			double dop_com[DOPPLER_AZIMUTH_STEPS];

			for (int azimuth_step = 0; azimuth_step < DOPPLER_AZIMUTH_STEPS;
				azimuth_step++)
			{
				antenna->azimuthAngle = azimuth_step_size *
					((double)azimuth_step + 0.5);

				CoordinateSwitch antenna_frame_to_gc =
					AntennaFrameToGC(orbit_state, attitude, antenna);

				double look, azimuth;
				if (! GetTwoWayPeakGain2(&antenna_frame_to_gc, &spacecraft,
					beam, antenna->spinRate, &look, &azimuth))
				{
					fprintf(stderr, "%s: error finding two-way peak gain\n",
						command);
					exit(1);
				}
				vector.SphericalSet(1.0, look, azimuth);

				//------------------------------//
				// calculate receiver gate info //
				//------------------------------//

				range_tracker.SetInstrument(&instrument);

				//--------------------------------//
				// calculate corrective frequency //
				//--------------------------------//

				IdealCommandedDoppler(&spacecraft, &instrument);

				// constants store Doppler to correct for (ergo -)
				dop_com[azimuth_step] = -instrument.commandedDoppler;
			}

			//------------------------//
			// fit doppler parameters //
			//------------------------//

			double a, p, c;
			azimuth_fit(DOPPLER_AZIMUTH_STEPS, dop_com, &a, &p, &c);
			*(*(*(terms + beam_idx) + orbit_step) + 0) = a;
			*(*(*(terms + beam_idx) + orbit_step) + 1) = p;
			*(*(*(terms + beam_idx) + orbit_step) + 2) = c;
		}
	}

	//---------------------------//
	// set the doppler constants //
	//---------------------------//

	doppler_tracker.Set(terms);
	if (dtc_file)
	{
		free_array((void *)terms, 3, antenna->numberOfBeams,
			DOPPLER_ORBIT_STEPS, 3);
	}

	//----------------------//
	// set ticks per orbits //
	//----------------------//

	unsigned int ticks_per_orbit = instrument.TimeToOrbitTicks(orbit_period);
	doppler_tracker.SetTicksPerOrbit(ticks_per_orbit);

	//------------------------------------------//
	// write out the Doppler tracking constants //
	//------------------------------------------//

	if (dtc_file)
	{
		if (! doppler_tracker.WriteBinary(dtc_file))
		{
			fprintf(stderr, "%s: error writing DTC file %s\n", command,
				dtc_file);
			exit(1);
		}
	}

	return (0);
}
