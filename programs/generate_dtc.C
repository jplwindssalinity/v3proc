//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		generate_dtc
//
// SYNOPSIS
//		generate_dtc <sim_config_file> <DTC_base>
//
// DESCRIPTION
//		Generates a set of Doppler Tracking Constants for each beam,
//		based upon the parameters in the simulation configuration
//		file and the given Receiver Gate Constants.
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
//		<DTC_base>			The DTC output base.
//
// EXAMPLES
//		An example of a command line is:
//			% generate_constants sws1b.cfg dtc.dat
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

#define DOPPLER_ORBIT_STEPS		256
#define DOPPLER_AZIMUTH_STEPS	90		// used for fitting

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

const char* usage_array[] = { "<sim_config_file>", "<DTC_base>", 0};

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
	const char* dtc_base = argv[arg_idx++];

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

	//-----------------------------------//
	// force RGC to be read, but not DTC //
	//-----------------------------------//

	config_list.StompOrAppend(USE_RGC_KEYWORD, "1");
	config_list.StompOrAppend(USE_DTC_KEYWORD, "0");
	config_list.StompOrAppend(USE_KFACTOR_KEYWORD, "0");

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

	//----------------//
	// allocate terms //
	//----------------//

	// terms are [0] = amplitude, [1] = phase, [2] = bias
	double** terms;
	terms = (double **)make_array(sizeof(double), 2, DOPPLER_ORBIT_STEPS, 3);

	//-----------//
	// variables //
	//-----------//

	OrbitState* orbit_state = &(spacecraft.orbitState);
	Attitude* attitude = &(spacecraft.attitude);
	Vector3 vector;

	double orbit_period = spacecraft_sim.GetPeriod();
	double orbit_step_size = orbit_period / (double)DOPPLER_ORBIT_STEPS;
	double azimuth_step_size = two_pi / (double)DOPPLER_AZIMUTH_STEPS;

	//-----------------//
	// loop over beams //
	//-----------------//

	for (int beam_idx = 0; beam_idx < antenna->numberOfBeams; beam_idx++)
	{
		//--------------------------//
		// allocate Doppler tracker //
		//--------------------------//

		antenna->currentBeamIdx = beam_idx;
		Beam* beam = antenna->GetCurrentBeam();
		if (! beam->dopplerTracker.Allocate(DOPPLER_ORBIT_STEPS))
		{
			fprintf(stderr, "%s: error allocating Doppler tracker\n", command);
			exit(1);
		}

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

			//-------------------------//
			// set the instrument time //
			//-------------------------//

			instrument.SetTime(time);

			//--------------------------------//
			// calculate baseband frequencies //
			//--------------------------------//

			double dop_com[DOPPLER_AZIMUTH_STEPS];

			for (int azimuth_step = 0; azimuth_step < DOPPLER_AZIMUTH_STEPS;
				azimuth_step++)
			{
				antenna->azimuthAngle = azimuth_step_size *
					(double)azimuth_step;

				CoordinateSwitch antenna_frame_to_gc =
					AntennaFrameToGC(orbit_state, attitude, antenna);

				double look, azimuth;
				if (! GetTwoWayPeakGain2(&antenna_frame_to_gc, &spacecraft,
					beam, antenna->actualSpinRate, &look, &azimuth))
				{
					fprintf(stderr, "%s: error finding two-way peak gain\n",
						command);
					exit(1);
				}
				vector.SphericalSet(1.0, look, azimuth);

				//------------------------------//
				// calculate receiver gate info //
				//------------------------------//

				float residual_delay_error = 0.0;
				beam->rangeTracker.SetInstrument(&instrument,
					&residual_delay_error);

				//--------------------------------------------------------//
				// hack in ideal delay by removing the quantization error //
				//--------------------------------------------------------//

				instrument.commandedRxGateDelay += residual_delay_error;

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
			*(*(terms + orbit_step) + 0) = a;
			*(*(terms + orbit_step) + 1) = p;
			*(*(terms + orbit_step) + 2) = c;
		}

		//-------------//
		// set Doppler //
		//-------------//

		beam->dopplerTracker.Set(terms);

		//------------------------------------------//
		// write out the doppler tracking constants //
		//------------------------------------------//

		char filename[1024];
		sprintf(filename, "%s.%d", dtc_base, beam_idx + 1);
		if (! beam->dopplerTracker.WriteBinary(filename))
		{
			fprintf(stderr, "%s: error writing DTC file %s\n", command,
				filename);
			exit(1);
		}
	}

	//----------------//
	// free the array //
	//----------------//

	free_array((void *)terms, 2, DOPPLER_ORBIT_STEPS, 3);

	return (0);
}
