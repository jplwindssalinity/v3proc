//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    dtc_freq
//
// SYNOPSIS
//    dtc_freq <dtc_file> <freq_file>
//
// DESCRIPTION
//    Generates plottable error file showing the baseband frequency
//    error between the actual peak 2-way gain baseband frequency and
//    the algorithmic frequency.
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operands are supported:
//      <sim_config_file>    The sim_config_file needed listing all
//                             input parameters, input files, and
//                             output files.
//
//      <error_file>         A plottable error file.
//
// EXAMPLES
//    An example of a command line is:
//      % baseband_errors qscat.cfg baseband.err
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
#include "List.h"
#include "List.C"
#include "Spacecraft.h"
#include "ConfigSim.h"
#include "Tracking.h"
#include "InstrumentGeom.h"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"
#include "QscatConfig.h"

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<AngleInterval>;
template class List<EarthPosition>;
template class List<StringPair>;
template class List<Meas>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class List<long>;
template class List<OffsetList>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;
template class TrackerBase<unsigned char>;
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

const char* usage_array[] = { "<sim_config_file>", "<error_file>", 0};

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
    const char* error_file = argv[arg_idx++];

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
        fprintf(stderr, "%s: error configuring QSCAT simulator\n", command);
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

    double eqx_time =
        spacecraft_sim.FindPrevArgOfLatTime(instrument_start_time,
            EQX_ARG_OF_LAT, EQX_TIME_TOLERANCE);
    qscat.cds.SetEqxTime(eqx_time);

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

        //---------------------------------------//
        // process instrument event if necessary //
        //---------------------------------------//

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
                //------------------------------//
                // process the instrument event //
                //------------------------------//

                Antenna* antenna = &(qscat.sas.antenna);
                Beam* beam;
                OrbitState* orbit_state;
                Attitude* attitude;
                CoordinateSwitch antenna_frame_to_gc;
                Vector3 rlook_antenna;
                QscatTargetInfo qti;

                switch(qscat_event.eventId)
                {
                case QscatEvent::SCAT_EVENT:
                case QscatEvent::LOOPBACK_EVENT:
                case QscatEvent::LOAD_EVENT:

                    // process spacecraft stuff
                    spacecraft_sim.UpdateOrbit(qscat_event.time,
                        &spacecraft);
                    spacecraft_sim.UpdateAttitude(qscat_event.time,
                        &spacecraft);

                    // process instrument stuff
                    qscat.cds.SetTime(qscat_event.time);
                    qscat.sas.antenna.UpdatePosition(qscat_event.time);
                    qscat.SetOtherAzimuths(&spacecraft);

                    qscat.cds.currentBeamIdx = qscat_event.beamIdx;
                    beam = qscat.GetCurrentBeam();

                    //-------------------------------//
                    // do range and Doppler tracking //
                    //-------------------------------//

                    qscat.cds.useRgc = 1;
                    qscat.cds.useDtc = 1;
                    SetOrbitStepDelayAndFrequency(&spacecraft, &qscat);

                    //----------------------------------//
                    // calculate the baseband frequency //
                    //----------------------------------//

                    orbit_state = &(spacecraft.orbitState);
                    attitude = &(spacecraft.attitude);

                    antenna_frame_to_gc = AntennaFrameToGC(orbit_state,
                        attitude, antenna, antenna->txCenterAzimuthAngle);

                    double look, azimuth;
                    if (! GetPeakSpatialResponse2(&antenna_frame_to_gc,
                        &spacecraft, beam, antenna->spinRate, &look, &azimuth))
                    {
                        fprintf(stderr,
                          "%s: error calculating the peak spatial response\n",
                          command);
                        exit(1);
                    }

                    rlook_antenna.SphericalSet(1.0, look, azimuth);
                    qscat.TargetInfo(&antenna_frame_to_gc, &spacecraft,
                        rlook_antenna, &qti);

                    //------------------------------//
                    // calculate the baseband error //
                    //------------------------------//

                    // any deviation from zero Hz is undesirable
                    fprintf(error_fp, "%.6f %.6f %.6f\n", qscat_event.time,
                        qti.basebandFreq, qscat.ses.txDoppler);

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

    fclose(error_fp);

    return (0);
}
