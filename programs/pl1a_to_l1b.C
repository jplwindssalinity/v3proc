//==============================================================//
// Copyright (C) 1997-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    pl1a_to_l1b
//
// SYNOPSIS
//    pl1a_to_l1b <config_file>
//
// DESCRIPTION
//    Processes Polarimetric Level 1A data to Level 1B.
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operand is supported:
//    <config_file>  The config_file needed listing all input
//                   parameters, input files, and output files.
//
// EXAMPLES
//    An example of a command line is:
//      % pl1a_to_l1b polscat.cfg
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
// AUTHORS
//    James N. Huddleston (hudd@casket.jpl.nasa.gov)
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
#include "BufferedList.h"
#include "List.h"
#include "Misc.h"
#include "Ephemeris.h"
#include "ConfigList.h"
#include "ConfigSim.h"
#include "L1B.h"
#include "Tracking.h"
#include "PscatL1A.h"
#include "PscatL1AToL1B.h"
#include "PscatConfig.h"

using std::list;
using std::map; 

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<AngleInterval>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class List<WindVectorPlus>;
template class List<long>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;

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

const char* usage_array[] = { "<config_file>", 0};

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

    //---------------------//
    // read in config file //
    //---------------------//

    ConfigList config_list;
    if (! config_list.Read(config_file))
    {
        fprintf(stderr, "%s: error reading sim config file %s\n", command,
            config_file);
        exit(1);
    }

    //-------------------------------------//
    // create and configure level products //
    //-------------------------------------//

    PscatL1A pscat_l1a;
    if (! ConfigPscatL1A(&pscat_l1a, &config_list))
    {
        fprintf(stderr, "%s: error configuring Pscat Level 1A Product\n",
            command);
        exit(1);
    }

    L1B l1b;
    if (! ConfigL1B(&l1b, &config_list))
    {
        fprintf(stderr, "%s: error configuring Level 1B Product\n", command);
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

    //----------------------------//
    // create and configure PSCAT //
    //----------------------------//

    Pscat pscat;
    if (! ConfigPscat(&pscat, &config_list))
    {
        fprintf(stderr, "%s: error configuring PSCAT\n", command);
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

    //------------//
    // open files //
    //------------//

    pscat_l1a.OpenForReading();
    l1b.OpenForWriting();

    //-----------------//
    // conversion loop //
    //-----------------//

    int data_record_number = 1;
    PscatL1AToL1B l1a_to_l1b;
    if (! ConfigPscatL1AToL1B(&l1a_to_l1b, &config_list))
    {
        fprintf(stderr,
            "%s: error configuring Level 1A to Level 1B converter.\n",
            command);
        exit(1);
    }

    int top_of_file = 1;
    do
    {
        //-----------------------------//
        // read a level 1A data record //
        //-----------------------------//

        if (! pscat_l1a.ReadDataRec())
        {
            switch (pscat_l1a.GetStatus())
            {
            case L1A::OK:    // end of file
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
                fprintf(stderr, "%s: unknown status\n", command);
                exit(1);
            }
            break;    // done, exit do loop
        }

        //=======================//
        // FOR FIRST RECORD ONLY //
        //=======================//
        //===========================================================//
        // This part may be omitted if the prev_eqx_time is included //
        // in the l1a file                                           //
        //===========================================================//

        if (top_of_file == 1)
        {
            top_of_file = 0;

            //----------------------------------//
            // Quickly simulate the spacecraft  //
            // to get previous equator crossing //
            // time.                            //
            //----------------------------------//

            SpacecraftSim spacecraft_sim;
            if (! ConfigSpacecraftSim(&spacecraft_sim, &config_list))
            {
                fprintf(stderr, "%s: error configuring spacecraft simulator\n",
                    command);
                exit(1);
            }

            //---------------------------//
            // set the previous Eqx time //
            //---------------------------//

            pscat_l1a.frame.Unpack(pscat_l1a.buffer);

            double eqx_time =
                spacecraft_sim.FindPrevArgOfLatTime(pscat_l1a.frame.time,
                EQX_ARG_OF_LAT, EQX_TIME_TOLERANCE);
            pscat.cds.SetEqxTime(eqx_time);
        }

        //---------//
        // convert //
        //---------//

        if (! l1a_to_l1b.Convert(&pscat_l1a, &spacecraft, &pscat, &ephemeris,
            topo_ptr, stable_ptr, &l1b))
        {
            fprintf(stderr, "%s: error converting data record %d\n", command,
                data_record_number);
        }
        else if (! l1b.WriteDataRec())
        {
            //------------------------------//
            // write a level 1B data record //
            //------------------------------//

            fprintf(stderr, "%s: error writing Level 1B data\n", command);
            exit(1);
        }

        data_record_number++;
    } while (1);

    pscat_l1a.Close();
    l1b.Close();

    return (0);
}
