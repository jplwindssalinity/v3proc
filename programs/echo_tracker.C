//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		echo_tracker
//
// SYNOPSIS
//		echo_tracker <sim_config_file>
//
// DESCRIPTION
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
//			% echo_tracker sws1b.cfg
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
#include "L1A.h"
#include "Spacecraft.h"
#include "Instrument.h"
#include "ConfigSim.h"
#include "List.h"
#include "List.C"
#include "Tracking.h"
#include "Tracking.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Matrix.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<Meas>;
template class List<long>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class List<OffsetList>;
template class List<StringPair>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;
template class List<EarthPosition>;

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

	//---------------------//
	// read in config file //
	//---------------------//

	ConfigList config_list;
	if (! config_list.Read(config_file))
	{
		fprintf(stderr, "%s: error reading sim config file %s\n",
			command, config_file);
		exit(1);
	}

	//-------------------------------------//
	// create and configure level products //
	//-------------------------------------//

	L1A l1a;
	if (! ConfigL1A(&l1a, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 1A Product\n", command);
		exit(1);
	}

	//---------------------------------//
	// create and configure spacecraft //
	//---------------------------------//

	Spacecraft spacecraft;
	if (! ConfigSpacecraft(&spacecraft, &config_list))
	{
		fprintf(stderr, "%s: error configuring spacecraft\n", command);
		exit(1);
	}

	//---------------------------------//
	// create and configure instrument //
	//---------------------------------//

	Instrument instrument;
	if (! ConfigInstrument(&instrument, &config_list))
	{
		fprintf(stderr, "%s: error configuring instrument\n", command);
		exit(1);
	}

	//--------------------------------//
	// create and configure ephemeris //
	//--------------------------------//

	Ephemeris ephemeris;
	if (! ConfigEphemeris(&ephemeris, &config_list))
	{
		fprintf(stderr, "%s: error configuring ephemeris\n", command);
		exit(1);
	}

	//-----------//
	// variables //
	//-----------//

	float slice_number[10] = { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0,
		9.0 };
	float signal_energy[10];
	double c[3];
	Vector coefs;
	coefs.Allocate(3);

	//-----------//
	// predigest //
	//-----------//

	Antenna* antenna = &(instrument.antenna);
	L1AFrame* frame = &(l1a.frame);
	double Gn = instrument.noise_receiverGain;
	double Ge = instrument.echo_receiverGain;
	double Bn = instrument.noiseBandwidth;
	double Be = instrument.GetTotalSignalBandwidth();
	double Bs = instrument.scienceSliceBandwidth;

	//------------//
	// open files //
	//------------//

	l1a.OpenForReading();

	//-----------------//
	// conversion loop //
	//-----------------//

	int top_of_file = 1;
	do
	{
		//-----------------------------//
		// read a level 1A data record //
		//-----------------------------//

		if (! l1a.ReadDataRec())
		{
			switch (l1a.GetStatus())
			{
			case L1A::OK:		// end of file
				break;
			case L1A::ERROR_READING_FRAME:
				fprintf(stderr, "%s: error reading Level 1A data\n", command);
				exit(1);
				break;
			case L1A::ERROR_UNKNOWN:
				fprintf(stderr, "%s: unknown error reading Level 1A data\n",
					command);
				exit(1);
				break;
			default:
				fprintf(stderr, "%s: unknown status (???)\n", command);
				exit(1);
			}
			break;		// done, exit do loop
		}

		//-------------------------------//
		// set previous eqx if necessary //
		//-------------------------------//

		if (top_of_file)
		{
			top_of_file = 0;
			SpacecraftSim spacecraft_sim;
			if (! ConfigSpacecraftSim(&spacecraft_sim, &config_list))
			{
				fprintf(stderr, "%s: error configuring spacecraft simulator\n",
					command);
				exit(1);
			}
			l1a.frame.Unpack(l1a.buffer);
		  
			double eqx_time =
				spacecraft_sim.FindPrevArgOfLatTime(l1a.frame.time,
				EQX_ARG_OF_LAT, EQX_TIME_TOLERANCE);
			instrument.SetEqxTime(eqx_time);
		}

		//---------------//
		// find the peak //
		//---------------//

		l1a.frame.Unpack(l1a.buffer);
		int base_slice_idx = 0;
		int spot_idx = 0;
		for (int beam_cycle = 0; beam_cycle < frame->antennaCyclesPerFrame;
			beam_cycle++)
		{
			for (int beam_idx = 0; beam_idx < antenna->numberOfBeams;
				beam_idx++)
			{
				//-----------------------//
				// calculate some things //
				//-----------------------//

				double En = frame->spotNoise[spot_idx];
				double Es = 0.0;
				for (int slice_idx = 0; slice_idx < frame->slicesPerSpot;
					slice_idx++)
				{
					Es += frame->science[base_slice_idx + slice_idx];
				}

				//--------------------------------------------//
				// determine the noise power spectral density //
				//--------------------------------------------//

				double npsd = ((En / (Gn * Bn)) - (Es / (Ge * Bn))) /
					(1.0 - Be / Bn);

				for (int slice_idx = 0; slice_idx < frame->slicesPerSpot;
					slice_idx++)
				{
					signal_energy[slice_idx] =
						frame->science[base_slice_idx + slice_idx] -
						Ge * Bs * npsd;
				}

				// peak
				Matrix u, v;
				Vector w;
				u.SVDFit(slice_number, signal_energy, NULL,
					frame->slicesPerSpot, &coefs, 3, &u, &v, &w);
				coefs.GetElement(0, &(c[0]));
				coefs.GetElement(1, &(c[1]));
				coefs.GetElement(2, &(c[2]));
				float peak_slice = -c[1] / (2.0 * c[2]);
				if (peak_slice < 0.0 || peak_slice > frame->slicesPerSpot)
					continue;

				//------------//
				// accumulate //
				//------------//

				antenna->SetAzimuthWithEncoder(
					frame->antennaPosition[spot_idx]);
				double angle = antenna->azimuthAngle;

				printf("%g %g\n", angle, peak_slice);
				base_slice_idx += frame->slicesPerSpot;
			}
			spot_idx++;
		}
	} while (1);

	l1a.Close();

	return (0);
}
