//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		et_estimator
//
// SYNOPSIS
//		et_estimator <sim_config_file> <output_file>
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
//		<output_file>			The output file.
//
// EXAMPLES
//		An example of a command line is:
//			% et_estimator sws1b.cfg out.txt
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
#include "InstrumentGeom.h"

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

#define BEAMS				2
#define PULSES				3000
#define AZIMUTH_STEP_SIZE	5.0

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int estimate(Spacecraft* spacecraft, Instrument* instrument,
		unsigned int orbit_step, Ephemeris* ephemeris);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<sim_config_file>", "<output_file>", 0};

OrbitState		orbit_state[BEAMS][PULSES];
Attitude		reported_attitude[BEAMS][PULSES];
unsigned int	antenna_encoder[BEAMS][PULSES];
float			f_bb_peak[BEAMS][PULSES];
int				pulse_count[BEAMS] = { 0, 0};

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

	int clidx = 1;
	const char* config_file = argv[clidx++];
	const char* output_file = argv[clidx++];

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

	//-------------------//
	// set up conversion //
	//-------------------//

	L1AToL1B l1a_to_l1b;
	if (! ConfigL1AToL1B(&l1a_to_l1b, &config_list))
	{
		fprintf(stderr,
			"%s: error configuring Level 1A to Level 1B converter.\n",
			command);
		exit(1);
	}

	//------------//
	// open files //
	//------------//

	l1a.OpenForReading();

	//-----------------//
	// conversion loop //
	//-----------------//

	unsigned int last_orbit_step = 0;

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

		//--------//
		// unpack //
		//--------//

		frame->Unpack(l1a.buffer);
		unsigned int orbit_ticks = frame->orbitTicks;
		unsigned char pri_of_orbit_tick_change = frame->priOfOrbitTickChange;

		//--------------------------//
		// loop for each beam cycle //
		//--------------------------//

		for (int beam_cycle = 0; beam_cycle < frame->antennaCyclesPerFrame;
			beam_cycle++)
		{
			//--------------------//
			// loop for each beam //
			//--------------------//

			for (int beam_idx = 0; beam_idx < antenna->numberOfBeams;
				beam_idx++)
			{
				int spot_idx = beam_cycle * antenna->numberOfBeams + beam_idx;
				int base_slice_idx = spot_idx * frame->slicesPerSpot;

				//--------------------------------//
				// check for completed orbit step //
				//--------------------------------//

				antenna->currentBeamIdx = beam_idx;
				Beam* beam = antenna->GetCurrentBeam();
				if (spot_idx == pri_of_orbit_tick_change)
					orbit_ticks++;
				unsigned int orbit_step =
					beam->dopplerTracker.OrbitTicksToStep(orbit_ticks,
					instrument.orbitTicksPerOrbit);

				if (orbit_step != last_orbit_step)
				{
					estimate(&spacecraft, &instrument, last_orbit_step,
						&ephemeris);
					pulse_count[0] = 0;
					pulse_count[1] = 0;
				}
				last_orbit_step = orbit_step;

				//-------------------------//
				// calculate the spot time //
				//-------------------------//
 
				double time = frame->time + beam_cycle * antenna->priPerBeam +
					beam->timeOffset;

				//---------------------------//
				// calculate the orbit state //
				//---------------------------//

				OrbitState os;
				if (! ephemeris.GetOrbitState(time, EPHEMERIS_INTERP_ORDER,
					&os))
				{
					return(0);
				}

				//-----------------------//
				// get the azimuth angle //
				//-----------------------//

				unsigned int encoder =
					frame->antennaPosition[spot_idx];

				//----------------------------------//
				// calculate the signal only energy //
				//----------------------------------//

				double En = frame->spotNoise[spot_idx];
				double Es = 0.0;
				for (int slice_idx = 0; slice_idx < frame->slicesPerSpot;
					slice_idx++)
				{
					Es += frame->science[base_slice_idx + slice_idx];
				}

				double npsd = ((En / (Gn * Bn)) - (Es / (Ge * Bn))) /
					(1.0 - Be / Bn);

				for (int slice_idx = 0; slice_idx < frame->slicesPerSpot;
					slice_idx++)
				{
					signal_energy[slice_idx] =
						frame->science[base_slice_idx + slice_idx] -
						Ge * Bs * npsd;
				}

				//----------------------------------//
				// fit a quadratic to find the peak //
				//----------------------------------//

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

				int near_slice_idx = (int)(peak_slice + 0.5);
				float f1, bw;
				instrument.GetSliceFreqBw(near_slice_idx, &f1, &bw);
				float f_bb_data = f1 + bw * (peak_slice -
					(float)near_slice_idx + 0.5);

				//------------//
				// accumulate //
				//------------//

				unsigned int pc = pulse_count[beam_idx];
				orbit_state[beam_idx][pc] = os;
				antenna_encoder[beam_idx][pc] = encoder;
				f_bb_peak[beam_idx][pc] = f_bb_data;
				reported_attitude[beam_idx][pc] = frame->attitude;
				pulse_count[beam_idx]++;
			}
		}
	} while (1);

	estimate(&spacecraft, &instrument, last_orbit_step, &ephemeris);

	l1a.Close();

	return (0);
}

//----------//
// estimate //
//----------//

int
estimate(
	Spacecraft*		spacecraft,
	Instrument*		instrument,
	unsigned int	orbit_step,
	Ephemeris*		ephemeris)
{
	if (pulse_count[0] == 0 && pulse_count[1] == 0)
		return(1);

	//---------------------------------------//
	// create a spacecraft and an instrument //
	//---------------------------------------//

	Antenna* antenna = &(instrument->antenna);

	//------------------//
	// for each beam... //
	//------------------//

	for (int beam_idx = 0; beam_idx < 2; beam_idx++)
	{
		//-------------------//
		// for each pulse... //
		//-------------------//

		for (int pulse_idx = 0; pulse_idx < pulse_count[beam_idx]; pulse_idx++)
		{
			//--------------------------------------//
			// set up the spacecraft and instrument //
			//--------------------------------------//

			spacecraft->orbitState = orbit_state[beam_idx][pulse_idx];
			antenna->SetAzimuthWithEncoder(
				antenna_encoder[beam_idx][pulse_idx]);
			antenna->currentBeamIdx = beam_idx;
			Beam* beam = antenna->GetCurrentBeam();
			instrument->orbitTicks = beam->rangeTracker.OrbitStepToTicks(
				orbit_step, instrument->orbitTicksPerOrbit);

			SetRangeAndDoppler(spacecraft, instrument);

			//-----------------------//
			// use reported attitude //
			//-----------------------//

			CoordinateSwitch antenna_frame_to_gc =
				AntennaFrameToGC(&(spacecraft->orbitState),
				&(reported_attitude[beam_idx][pulse_idx]), antenna);

			double center_look, center_azim;
			if (! GetTwoWayPeakGain2(&antenna_frame_to_gc, spacecraft, beam,
				instrument->antenna.actualSpinRate, &center_look,
				&center_azim))
			{
				return(0);
			}
 
			Vector3 vector;
			vector.SphericalSet(1.0, center_look, center_azim);
			TargetInfoPackage tip;
			if (! TargetInfo(&antenna_frame_to_gc, spacecraft, instrument,
				vector, &tip))
			{
				return(0);
			}
			double f_bb_expected = tip.basebandFreq;

			printf("%d %g %g\n", antenna_encoder[beam_idx][pulse_idx],
				f_bb_peak[beam_idx][pulse_idx], f_bb_expected);
		}
printf("&\n");
	}
	printf("&\n");
	return(1);
}
