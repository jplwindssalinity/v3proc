//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		plot_delay
//
// SYNOPSIS
//		plot_delay <sim_config_file> <RGC_file> <delay_file>
//
// DESCRIPTION
//		Plots the range gate delay from a set of RGC.
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
//		<delay_file>		The delay output file.
//
// EXAMPLES
//		An example of a command line is:
//			% plot_delay sws1b.cfg rgc.dat delay.out
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
#include "Instrument.h"
#include "Tracking.h"
#include "ConfigSim.h"
#include "List.h"
#include "List.C"
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

#define RANGE_AZIMUTH_STEPS		360

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
	"<delay_file>", 0};

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
	const char* delay_file = argv[arg_idx++];

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

	//----------------------//
	// create an instrument //
	//----------------------//

	Instrument instrument;
	if (! ConfigInstrument(&instrument, &config_list))
	{
		fprintf(stderr, "%s: error configuring instrument\n", command);
		exit(1);
	}
	Antenna* antenna = &(instrument.antenna);

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

	//-----------//
	// variables //
	//-----------//

	int range_steps = range_tracker.GetRangeSteps();
	int beams = range_tracker.GetNumberOfBeams();

	//----------------------//
	// open the output file //
	//----------------------//

	FILE* ofp = fopen(delay_file, "w");
	if (ofp == NULL)
	{
		fprintf(stderr, "%s: error opening delay file %s\n", command,
			delay_file);
		exit(1);
	}

	//--------------------------//
	// step through range steps //
	//--------------------------//

	for (int range_step = 0; range_step < range_steps; range_step++)
	{
		//-----------------------------//
		// step through azimuth angles //
		//-----------------------------//

		for (int azimuth_step = 0; azimuth_step < RANGE_AZIMUTH_STEPS;
			azimuth_step++)
		{
			double fstep = (double)range_step +
				(double)azimuth_step / (double)RANGE_AZIMUTH_STEPS;
			fprintf(ofp, "%g ", fstep);

			//--------------------//
			// step through beams //
			//--------------------//

			for (int beam_idx = 0; beam_idx < beams; beam_idx++)
			{
				antenna->currentBeamIdx = beam_idx;
				Beam* beam = antenna->GetCurrentBeam();

				//------------------------------//
				// calculate receiver gate info //
				//------------------------------//

				float delay, duration;
				range_tracker.GetDelayAndDuration(beam_idx, range_step,
					beam->pulseWidth, azimuth_step, RANGE_AZIMUTH_STEPS,
					&delay, &duration);

				//-------//
				// print //
				//-------//

				fprintf(ofp, " %g", delay * S_TO_MS);
			}

			fprintf(ofp, "\n");
		}
	}

	//-----------------------//
	// close the output file //
	//-----------------------//

	fclose(ofp);

	return (0);
}
