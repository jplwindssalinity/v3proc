//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    rgc_delay_errors
//
// SYNOPSIS
//    rgc_delay_errors <sim_config_file> <RGC_errs>
//
// DESCRIPTION
//    Generates plottable error files between the actual delay and the
//    delay calculated by the RGC.
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operands are supported:
//      <sim_config_file>  The sim_config_file needed listing all
//                           input parameters, input files, and output
//                           files.
//
//      <RGC_errs>         A plottable error file for the RGC.
//
// EXAMPLES
//    An example of a command line is:
//      % constant_errors sws1b.cfg rgc.dat rgc.errs
//
// ENVIRONMENT
//    Not environment dependent.
//
// EXIT STATUS
//    The following exit values are returned:
//       0  Program executed successfully
//      >0  Program had an error
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
#include "Misc.h"
#include "ConfigList.h"
#include "Spacecraft.h"
#include "ConfigSim.h"
#include "QscatConfig.h"
#include "InstrumentGeom.h"
#include "List.h"
#include "List.C"
#include "Tracking.h"
#include "Tracking.C"
#include "BufferedList.h"
#include "BufferedList.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<Meas>;
template class List<long>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class List<OffsetList>;
template class List<StringPair>;
template class TrackerBase<unsigned char>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;
template class List<EarthPosition>;
template class TrackerBase<unsigned short>;

//-----------//
// CONSTANTS //
//-----------//

#define EQX_TIME_TOLERANCE  0.1

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

const char* usage_array[] = { "<sim_config_file>", "<RGC_errs>", 0};

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

    if (argc != 3)
        usage(command, usage_array, 1);

    int arg_idx = 1;
    const char* config_file = argv[arg_idx++];
    const char* rgc_err_file = argv[arg_idx++];

    //--------------------------------//
    // read in simulation config file //
    //--------------------------------//

    ConfigList config_list;
    if (! config_list.Read(config_file))
    {
        fprintf(stderr, "%s: error reading sim config file %s\n", command,
            config_file);
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

    if (! ConfigControl(&spacecraft_sim, &config_list, &grid_start_time,
        &grid_end_time, &instrument_start_time, &instrument_end_time,
        &spacecraft_start_time, &spacecraft_end_time))
    {
        fprintf(stderr, "%s: error configuring simulation times\n", command);
        exit(1);
    }
    qscat_sim.startTime = instrument_start_time;

    //------------------//
    // set the eqx time //
    //------------------//

    double eqx_time =
        spacecraft_sim.FindPrevArgOfLatTime(instrument_start_time,
        EQX_ARG_OF_LAT, EQX_TIME_TOLERANCE);
    qscat.cds.SetEqxTime(eqx_time);

    //------------//
    // initialize //
    //------------//

    if (! qscat_sim.Initialize(&qscat))
    {
        fprintf(stderr, "%s: error initializing QSCAT simulator\n", command);
        exit(1);
    }

    if (! spacecraft_sim.Initialize(spacecraft_start_time))
    {
        fprintf(stderr, "%s: error initializing spacecraft simulator\n",
            command);
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
				case SpacecraftEvent::EQUATOR_CROSSING:
					qscat.cds.SetEqxTime(spacecraft_event.time);
					break;
				default:
					break;
				}
				spacecraft_sim.DetermineNextEvent(&spacecraft_event);
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

				Antenna* antenna;
				Beam* beam;
				OrbitState* orbit_state;
				Attitude* attitude;
				double ideal_delay, delay_error;
				CoordinateSwitch antenna_frame_to_gc;
				TargetInfoPackage tip;
				Vector3 vector;
				float delay;
                SesBeamInfo* ses_beam_info;

				switch(qscat_event.eventId)
				{
				case QscatEvent::SCATTEROMETER_MEASUREMENT:
				case QscatEvent::LOOPBACK_MEASUREMENT:
				case QscatEvent::LOAD_MEASUREMENT:

					// process spacecraft stuff
					spacecraft_sim.UpdateOrbit(qscat_event.time,
						&spacecraft);
					spacecraft_sim.UpdateAttitude(qscat_event.time,
						&spacecraft);

					// process QSCAT stuff
					qscat.cds.SetTime(qscat_event.time);
                    qscat.sas.antenna.UpdatePosition(qscat_event.time);
					qscat.cds.currentBeamIdx = qscat_event.beamIdx;

					//-------------------------//
					// calculate the RGC delay //
					//-------------------------//

                    SetOrbitStepDelayAndFrequency(&spacecraft, &qscat);
                    delay = qscat.ses.rxGateDelay;

					//------------------------------//
					// get the true round trip time //
					//------------------------------//

					antenna = &(qscat.sas.antenna);
					orbit_state = &(spacecraft.orbitState);
					attitude = &(spacecraft.attitude);

                    // center on the transmit pulse
                    qscat.sas.antenna.TimeRotation(qscat.ses.txPulseWidth /
                        2.0);

					antenna_frame_to_gc = AntennaFrameToGC(orbit_state,
						attitude, antenna);

					double look, azim;
                    beam = qscat.GetCurrentBeam();
					if (! GetPeakSpatialResponse2(&antenna_frame_to_gc,
                        &spacecraft, beam, antenna->spinRate, &look, &azim))
                    {
                        fprintf(stderr,
                          "%s: error calculating the peak spatial response\n",
                          command);
                        exit(1);
                    }
					vector.SphericalSet(1.0, look, azim);
					TargetInfo(&antenna_frame_to_gc, &spacecraft, &qscat,
						vector, &tip);

					//---------------------------//
					// calculate the ideal delay //
					//---------------------------//

                    ses_beam_info = qscat.GetCurrentSesBeamInfo();
					ideal_delay = tip.roundTripTime +
						(qscat.ses.txPulseWidth -
                        ses_beam_info->rxGateWidth) / 2.0;

					//---------------------------//
					// calculate the delay error //
					//---------------------------//

					delay_error = delay - ideal_delay;
					fprintf(rgc_err_fp, "%.6f %.6f %.6f %.6f %d\n",
						qscat_event.time, delay_error * 1000.0,
						ideal_delay * 1000.0, delay * 1000.0,
                        qscat.cds.currentBeamIdx);

					qscat_sim.DetermineNextEvent(&qscat, &qscat_event);
					break;

				default:
					fprintf(stderr, "%s: unknown QSCAT event\n", command);
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
