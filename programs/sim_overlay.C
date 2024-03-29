//==============================================================//
// Copyright (C) 1997-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    sim_overlay
//
// SYNOPSIS
//    sim_overlay <sim_config_file>
//
// DESCRIPTION
//    Simulates the SeaWinds 1b instrument based on the parameters
//    in the simulation configuration file.
//    the LON_OF_ASC_NODE Keyword is interpreted differently than in
//    sim. In this case it means the ascending equator crossing longitude.
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operand is supported:
//      <sim_config_file>  The sim_config_file needed listing all
//                           input parameters, input files, and
//                           output files.
//
// EXAMPLES
//    An example of a command line is:
//      % sim sws1b.cfg
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
// BROUGHT TO YOU BY
//    The QSCAT Sim Team
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
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include "ConfigList.h"
#include "Meas.h"
#include "Wind.h"
#include "Tracking.h"
#include "Spacecraft.h"
#include "SpacecraftSim.h"
#include "ConfigSim.h"
#include "QscatConfig.h"
#include "QscatSim.h"
#include "List.h"
#include "BufferedList.h"

//-----------//
// TEMPLATES //
//-----------//
// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<AngleInterval>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<WindVectorPlus>;
template class List<long>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

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

//--------------------//
// Report handler     //
// runs if SIGUSR1 is //
// recieved.          //
//--------------------//

float sim_time = 0.0;

//------------------------------------------//
// Debugging flag turns on print statements //
//------------------------------------------//

int debug_flag = 0;

void
report(
    int  sig_num)
{
    sig_num = sig_num;
    fprintf(stderr, "sim: Current simulation time %g\n", sim_time);
    return;
}

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
    if (argc != 2)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* config_file = argv[clidx++];

    //-----------------------//
    // tell how far you have //
    // gotten if you recieve //
    // the siguser1 signal   //
    //-----------------------//

    // sigset(SIGUSR1,&report);

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

    double long_of_asc_node;
    if (! config_list.GetDouble(LONG_OF_ASC_NODE_KEYWORD, &long_of_asc_node))
        return(0);

    spacecraft_sim.LocationToOrbit(long_of_asc_node,0.0,1);
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

    //----------------------------//
    // create a Level 1A product //
    //----------------------------//

    L1A l1a;
    if (! ConfigL1A(&l1a, &config_list))
    {
        fprintf(stderr, "%s: error configuring Level 0\n", command);
        exit(1);
    }
    l1a.OpenForWriting();

    //--------------------------//
    // create an ephemeris file //
    //--------------------------//

    char* ephemeris_filename;
    ephemeris_filename = config_list.Get(EPHEMERIS_FILE_KEYWORD);
    if (! ephemeris_filename)
    {
        fprintf(stderr, "%s: error getting ephemeris filename\n", command);
        exit(1);
    }

    FILE* eph_fp = fopen(ephemeris_filename, "w");
    if (eph_fp == NULL)
    {
        fprintf(stderr, "%s: error opening ephemeris file %s\n", command,
            ephemeris_filename);
        exit(1);
    }

    //-------------------------//
    // create an attitude file //
    //-------------------------//

    char* att_filename;
    att_filename = config_list.Get(ATTITUDE_FILE_KEYWORD);
    if (! att_filename)
    {
        fprintf(stderr, "%s: error getting attitude filename\n", command);
        exit(1);
    }
    FILE* att_fp = fopen(att_filename, "w");
    if (att_fp == NULL)
    {
        fprintf(stderr, "%s: error opening attitude file %s\n", command,
            att_filename);
        exit(1);
    }

    //--------------------//
    // read the windfield //
    //--------------------//

    WindField windfield;
    if (! ConfigWindField(&windfield, &config_list))
    {
        fprintf(stderr, "%s: error configuring wind field\n", command);
        exit(1);
    }

    //-----------------------//
    // read the sigma-0 maps //
    //-----------------------//

    Sigma0Map inner_map, outer_map;
    Sigma0Map* inner_map_ptr = NULL;
    Sigma0Map* outer_map_ptr = NULL;
    if (ConfigSigma0Maps(&inner_map, &outer_map, &config_list))
    {
        inner_map_ptr = &inner_map;
        outer_map_ptr = &outer_map;
    }

    //-----------------------------------------------------//
    // configure topographic map and frequency shift table //
    //-----------------------------------------------------//

    Topo topo;
    Topo* topo_ptr = NULL;
    Stable stable;
    Stable* stable_ptr = NULL;

    int use_topomap;
    config_list.ExitForMissingKeywords();
    config_list.GetInt(USE_TOPOMAP_KEYWORD, &use_topomap);
    if (use_topomap)
    {
        char* topomap_file = config_list.Get(TOPOMAP_FILE_KEYWORD);
        if (! topo.Read(topomap_file))
        {
            fprintf(stderr, "%s: error reading topographic map %s\n",
                command, topomap_file);
            exit(1);
        }
        topo_ptr = &topo;

        char* stable_file = config_list.Get(STABLE_FILE_KEYWORD);
        if (! stable.Read(stable_file))
        {
            fprintf(stderr, "%s: error reading S Table %s\n", command,
                stable_file);
            exit(1);
        }
        int stable_mode_id;
        config_list.GetInt(STABLE_MODE_ID_KEYWORD, &stable_mode_id);
        stable.SetModeId(stable_mode_id);
        stable_ptr = &stable;
    }

    //-------------------------------------//
    // read the geophysical model function //
    //-------------------------------------//

    GMF gmf;
    if (! ConfigGMF(&gmf, &config_list))
    {
        fprintf(stderr, "%s: error configuring GMF\n", command);
        exit(1);
    }

    //--------------//
    // configure Kp //
    //--------------//

    Kp kp;
    if (! ConfigKp(&kp, &config_list))
    {
        fprintf(stderr, "%s: error configuring Kp\n", command);
        exit(1);
    }

    //------------------//
    // Setup a KpmField //
    //------------------//

    KpmField kpmField;
    if (! ConfigKpmField(&kpmField, &config_list))
    {
        fprintf(stderr, "%s: error configuring KpmField\n", command);
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
    // Set spacecraft start time to an integer multiple of ephemeris period.
    spacecraft_start_time = spacecraft_sim.GetEphemerisPeriod() *
      ((int)(spacecraft_start_time / spacecraft_sim.GetEphemerisPeriod()));

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

    if (! qscat.sas.antenna.Initialize(instrument_start_time))
    {
        fprintf(stderr, "%s: error initializing antenna\n", command);
        exit(1);
    }

    //---------------------------//
    // set the previous Eqx time //
    //---------------------------//

    double eqx_time =
        spacecraft_sim.FindPrevArgOfLatTime(instrument_start_time,
        EQX_ARG_OF_LAT, EQX_TIME_TOLERANCE);
    qscat.cds.SetEqxTime(eqx_time);

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
    qscat_sim.DetermineNextEvent(l1a.frame.spotsPerFrame, &qscat, &qscat_event);

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
            if (spacecraft_event.time <= qscat_event.time || instrument_done)
            {
                //------------------------------//
                // process the spacecraft event //
                //------------------------------//

                sim_time = spacecraft_event.time;
                Attitude attitude;  // for recording in att file only

                switch(spacecraft_event.eventId)
                {
                case SpacecraftEvent::UPDATE_STATE:
                    spacecraft_sim.UpdateOrbit(spacecraft_event.time,
                        &spacecraft);
                    spacecraft.orbitState.Write(eph_fp);
                    spacecraft_sim.UpdateAttitude(spacecraft_event.time,
                        &spacecraft);
                    spacecraft_sim.ReportAttitude(spacecraft_event.time,
                      &spacecraft, &attitude);
                    spacecraft_sim.DetermineNextEvent(&spacecraft_event);
                    attitude.GSWrite(att_fp,spacecraft_event.time);
                    break;
                case SpacecraftEvent::EQUATOR_CROSSING:
                    qscat.cds.SetEqxTime(spacecraft_event.time);
                    spacecraft_sim.DetermineNextEvent(&spacecraft_event);
                    break;
                default:
                    fprintf(stderr, "%s: unknown spacecraft event\n", command);
                    exit(1);
                    break;
                }
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

                sim_time=qscat_event.time;
/*
                double mid_tx_pulse_time = qscat_event.time +
                   0.5*qscat.ses.txPulseWidth;
*/

                switch(qscat_event.eventId)
                {
                case QscatEvent::SCAT_EVENT:

                    // process spacecraft stuff
                    spacecraft_sim.UpdateOrbit(qscat_event.time,
                        &spacecraft);
                    spacecraft_sim.UpdateAttitude(qscat_event.time,
                        &spacecraft);

                    // process instrument stuff
                    qscat.cds.SetTime(qscat_event.time);
                    qscat.cds.currentBeamIdx = qscat_event.beamIdx;

                    // antenna
                    qscat.sas.antenna.UpdatePosition(qscat_event.time);
                    qscat.SetOtherAzimuths(&spacecraft);

                    qscat_sim.ScatSim(&spacecraft, &qscat, &windfield,
                        inner_map_ptr, outer_map_ptr, &gmf, &kp, &kpmField,
                        topo_ptr, stable_ptr, &(l1a.frame));
                    qscat_sim.DetermineNextEvent(l1a.frame.spotsPerFrame,
                                                 &qscat, &qscat_event);
                    break;
                case QscatEvent::LOOPBACK_EVENT:

                    // process spacecraft stuff
                    spacecraft_sim.UpdateOrbit(qscat_event.time,
                        &spacecraft);
                    spacecraft_sim.UpdateAttitude(qscat_event.time,
                        &spacecraft);

                    // process instrument stuff
                    qscat.cds.SetTime(qscat_event.time);
                    qscat.cds.currentBeamIdx = qscat_event.beamIdx;
                    qscat.sas.antenna.UpdatePosition(qscat_event.time);
                    qscat.SetOtherAzimuths(&spacecraft);
                    qscat_sim.LoopbackSim(&spacecraft, &qscat, &(l1a.frame));
                    qscat_sim.DetermineNextEvent(l1a.frame.spotsPerFrame,
                                                 &qscat, &qscat_event);
                    break;
                case QscatEvent::LOAD_EVENT:

                    // process spacecraft stuff
                    spacecraft_sim.UpdateOrbit(qscat_event.time,
                        &spacecraft);
                    spacecraft_sim.UpdateAttitude(qscat_event.time,
                        &spacecraft);

                    // process instrument stuff
                    qscat.cds.SetTime(qscat_event.time);
                    qscat.cds.currentBeamIdx = qscat_event.beamIdx;
                    qscat.sas.antenna.UpdatePosition(qscat_event.time);
                    qscat.SetOtherAzimuths(&spacecraft);
                    qscat_sim.LoadSim(&spacecraft, &qscat, &(l1a.frame));
                    qscat_sim.DetermineNextEvent(l1a.frame.spotsPerFrame,
                                                 &qscat, &qscat_event);
                    break;
                default:
                    fprintf(stderr, "%s: unknown instrument event\n", command);
                    exit(1);
                    break;
                }
            }

            //-----------------------------------//
            // write Level 1A data if necessary //
            //-----------------------------------//

            if (qscat_sim.l1aFrameReady)
            {
                // Report Latest Attitude Measurement
                // + Knowledge Error
                spacecraft_sim.ReportAttitude(sim_time, &spacecraft,
                    &(l1a.frame.attitude));

                int size = l1a.frame.Pack(l1a.buffer);
                l1a.Write(l1a.buffer, size);
            }
        }

        //---------------//
        // check if done //
        //---------------//

        if (instrument_done && spacecraft_done)
            break;
    }

    //----------------------//
    // close Level 1A file //
    //----------------------//

    l1a.Close();

    //--------------------------//
    // If createXtable is set    //
    // write XTABLE file        //
    //--------------------------//

    if(qscat_sim.createXtable)
    {
        qscat_sim.xTable.Write();
    }

    return (0);
}
