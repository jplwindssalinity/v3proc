//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    psim
//
// SYNOPSIS
//    psim <sim_config_file>
//
// DESCRIPTION
//    Simulates a polarimetric scatterometer based on the parameters
//    in the simulation configuration file.
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
//      % psim polscat.cfg
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
//    The Simulation Team
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
#include <signal.h>
#include "List.h"
#include "List.C"
#include "AngleInterval.h"
#include "ConfigList.h"
#include "Meas.h"
#include "PMeas.h"
#include "Tracking.h"
#include "Tracking.C"
#include "Spacecraft.h"
#include "ConfigSim.h"
#include "Pscat.h"
#include "PscatConfig.h"
#include "PscatSim.h"
#include "PscatL1A.h"
#include "BufferedList.h"
#include "BufferedList.C"

//-----------//
// TEMPLATES //
//-----------//
// Class declarations needed for templates
// eliminates need to include the entire header file

//class AngleInterval;
template class List<AngleInterval>;
template class List<StringPair>;
template class List<Meas>;
template class List<PMeas>;
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

//-----------------//
// Report handler  //
// runs if SIGUSR1 //
// is recieved.    //
//-----------------//

float sim_time = 0.0;
void
report(
    int  sig_num)
{
    sig_num = sig_num;
    fprintf(stderr, "psim: Current simulation time %g\n", sim_time);
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

    sigset(SIGUSR1, &report);

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

    //--------------------------------------//
    // create a PSCAT and a PSCAT simulator //
    //--------------------------------------//

    Pscat pscat;
    if (! ConfigPscat(&pscat, &config_list))
    {
        fprintf(stderr, "%s: error configuring PSCAT\n", command);
        exit(1);
    }

    PscatSim pscat_sim;
    if (! ConfigPscatSim(&pscat_sim, &config_list))
    {
        fprintf(stderr, "%s: error configuring PSCAT simulator\n", command);
        exit(1);
    }

    //---------------------------------//
    // create a PSCAT Level 1A product //
    //---------------------------------//

    PscatL1A l1a;
    if (! ConfigPscatL1A(&l1a, &config_list))
    {
        fprintf(stderr, "%s: error configuring PSCAT Level 1A\n", command);
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
        fprintf(stderr, "%s: error creating ephemeris file %s\n", command,
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
    pscat_sim.startTime = instrument_start_time;
    // Set spacecraft start time to an integer multiple of ephemeris period.
    spacecraft_start_time = spacecraft_sim.GetEphemerisPeriod() *
      ((int)(spacecraft_start_time / spacecraft_sim.GetEphemerisPeriod()));

    //------------//
    // initialize //
    //------------//

    if (! pscat_sim.Initialize(&pscat))
    {
        fprintf(stderr, "%s: error initializing PSCAT simulator\n", command);
        exit(1);
    }

    if (! spacecraft_sim.Initialize(spacecraft_start_time))
    {
        fprintf(stderr, "%s: error initializing spacecraft simulator\n",
            command);
        exit(1);
    }

    if (! pscat.sas.antenna.Initialize(instrument_start_time))
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
    pscat.cds.SetEqxTime(eqx_time);

    //----------------------//
    // cycle through events //
    //----------------------//

    SpacecraftEvent spacecraft_event;
    spacecraft_event.time = spacecraft_start_time;

    PscatEvent pscat_event;
    pscat_event.eventTime = instrument_start_time;

    int spacecraft_done = 0;
    int instrument_done = 0;

    //-------------------------//
    // start with first events //
    //-------------------------//

    spacecraft_sim.DetermineNextEvent(&spacecraft_event);
    pscat_sim.DetermineNextEvent(&pscat, &pscat_event);

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
            if (spacecraft_event.time <= pscat_event.eventTime ||
                instrument_done)
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
                    break;
                case SpacecraftEvent::EQUATOR_CROSSING:
                    pscat.cds.SetEqxTime(spacecraft_event.time);
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
            if (pscat_event.eventTime > instrument_end_time)
            {
                instrument_done = 1;
                continue;
            }
            if (pscat_event.eventTime <= spacecraft_event.time ||
                spacecraft_done)
            {
                //------------------------------//
                // process the instrument event //
                //------------------------------//

                sim_time = pscat_event.eventTime;

                switch(pscat_event.eventId)
                {
                case PscatEvent::VV_SCAT_EVENT:
                case PscatEvent::HH_SCAT_EVENT:
                case PscatEvent::VV_HV_SCAT_EVENT:
                case PscatEvent::HH_VH_SCAT_EVENT:

                    // process spacecraft stuff
                    spacecraft_sim.UpdateOrbit(pscat_event.eventTime,
                        &spacecraft);
                    spacecraft_sim.UpdateAttitude(pscat_event.eventTime,
                        &spacecraft);

                    // process instrument stuff
                    pscat.cds.SetTime(pscat_event.eventTime);
                    pscat.cds.currentBeamIdx = pscat_event.beamIdx;

                    // antenna
                    pscat.sas.antenna.UpdatePosition(pscat_event.eventTime);
                    pscat.SetOtherAzimuths(&spacecraft);

                    pscat_sim.ScatSim(&spacecraft, &pscat, &pscat_event,
                        &windfield, &gmf, &kp, &kpmField, &(l1a.frame));
                    pscat_sim.DetermineNextEvent(&pscat, &pscat_event);
                    break;
                case PscatEvent::LOOPBACK_EVENT:

                    // process spacecraft stuff
                    spacecraft_sim.UpdateOrbit(pscat_event.eventTime,
                        &spacecraft);
                    spacecraft_sim.UpdateAttitude(pscat_event.eventTime,
                        &spacecraft);

                    // process instrument stuff
                    pscat.cds.SetTime(pscat_event.eventTime);
                    pscat.cds.currentBeamIdx = pscat_event.beamIdx;
                    pscat.sas.antenna.UpdatePosition(pscat_event.eventTime);
                    pscat.SetOtherAzimuths(&spacecraft);
                    pscat_sim.LoopbackSim(&spacecraft, &pscat, &(l1a.frame));
                    pscat_sim.DetermineNextEvent(&pscat, &pscat_event);
                    break;
                case PscatEvent::LOAD_EVENT:

                    // process spacecraft stuff
                    spacecraft_sim.UpdateOrbit(pscat_event.eventTime,
                        &spacecraft);
                    spacecraft_sim.UpdateAttitude(pscat_event.eventTime,
                        &spacecraft);

                    // process instrument stuff
                    pscat.cds.SetTime(pscat_event.eventTime);
                    pscat.cds.currentBeamIdx = pscat_event.beamIdx;
                    pscat.sas.antenna.UpdatePosition(pscat_event.eventTime);
                    pscat.SetOtherAzimuths(&spacecraft);
                    pscat_sim.LoadSim(&spacecraft, &pscat, &(l1a.frame));
                    pscat_sim.DetermineNextEvent(&pscat, &pscat_event);
                    break;
                default:
                    fprintf(stderr, "%s: unknown instrument event (%d)\n",
                        command, pscat_event.eventId);
                    exit(1);
                    break;
                }
            }

            //----------------------------------//
            // write Level 1A data if necessary //
            //----------------------------------//

            if (pscat_sim.l1aFrameReady)
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

    if(pscat_sim.createXtable)
    {
        pscat_sim.xTable.Write();
    }

    return (0);
}
