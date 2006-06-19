//==============================================================//
// Copyright (C) 1997-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    l2a_to_l2b
//
// SYNOPSIS
//    l2a_to_l2b <sim_config_file>
//
// DESCRIPTION
//    Simulates the SeaWinds 1b ground processing of Level 2A to
//    Level 2B data.  This program retrieves wind from measurements.
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operand is supported:
//      <sim_config_file>  The sim_config_file needed listing
//                         all input parameters, input files, and
//                         output files.
//
// EXAMPLES
//    An example of a command line is:
//      % l2a_to_l2b sws1b.cfg
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
//    James N. Huddleston (James.N.Huddleston@jpl.nasa.gov)
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
#include <string.h>
#include <signal.h>
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "ConfigList.h"
#include "L2A.h"
#include "ConfigSim.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Tracking.h"
#include "Tracking.C"

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<AngleInterval>;
template list<string>;
template map<string,string,Options::ltstr>;

//-----------//
// CONSTANTS //
//-----------//

#define MAX_ALONG_TRACK_BINS  1624

//-------//
// HACKS //
//-------//

//#define LATLON_LIMIT_HACK

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

const char* usage_array[] = { "<sim_config_file>", "[ignore_bad_l2a]",0};

//--------------------//
// Report handler     //
// runs if SIGUSR1 is //
// received.          //
//--------------------//

int global_frame_number = 0;
void
report(
    int  sig_num)
{
    fprintf(stderr,"l2a_to_l2b: Starting frame number %d\n",
        global_frame_number);
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

    int ignore_bad_l2a=0;
    const char* command = no_path(argv[0]);
    if (argc != 2 && argc!=3 )
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* config_file = argv[clidx++];
    if(argc==3) ignore_bad_l2a =atoi(argv[clidx++]);

    //------------------------//
    // tell how far you have  //
    // gotten if you recieve  //
    // the siguser1 signal    //
    //------------------------//

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

    L2A l2a;
    if (! ConfigL2A(&l2a, &config_list))
    {
        fprintf(stderr, "%s: error configuring Level 2A Product\n", command);
        exit(1);
    }

    L2B l2b;
    if (! ConfigL2B(&l2b, &config_list))
    {
        fprintf(stderr, "%s: error configuring Level 2B Product\n", command);
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

    //------------------------------------//
    // create and configure the converter //
    //------------------------------------//

    L2AToL2B l2a_to_l2b;
    if (! ConfigL2AToL2B(&l2a_to_l2b, &config_list))
    {
        fprintf(stderr, "%s: error configuring L2AToL2B\n", command);
        exit(1);
    }

    //------------//
    // open files //
    //------------//

    l2a.OpenForReading();
    l2b.OpenForWriting();

    //---------------------------------//
    // read the header to set up swath //
    //---------------------------------//

    if (! l2a.ReadHeader())
    {
        fprintf(stderr, "%s: error reading Level 2A header\n", command);
        exit(1);
    }

    int along_track_bins =
        (int)(two_pi * r1_earth / l2a.header.alongTrackResolution + 0.5);
 
    if (! l2b.frame.swath.Allocate(l2a.header.crossTrackBins,
        along_track_bins))
    {
        fprintf(stderr, "%s: error allocating wind swath\n", command);
        exit(1);
    }

    //-----------------------------------------//
    // transfer information to level 2B header //
    //-----------------------------------------//

    l2b.header.crossTrackResolution = l2a.header.crossTrackResolution;
    l2b.header.alongTrackResolution = l2a.header.alongTrackResolution;
    l2b.header.zeroIndex = l2a.header.zeroIndex;

    //-----------------//
    // conversion loop //
    //-----------------//

    for (;;)
    {
        global_frame_number++;

        //-----------------------------//
        // read a level 2A data record //
        //-----------------------------//

        if (! l2a.ReadDataRec())
        {
            switch (l2a.GetStatus())
            {
            case L2A::OK:        // end of file
                break;
            case L2A::ERROR_READING_FRAME:
                fprintf(stderr, "%s: error reading Level 2A data\n", command);
                if(!ignore_bad_l2a) exit(1);
                break;
            case L2A::ERROR_UNKNOWN:
                fprintf(stderr, "%s: unknown error reading Level 2A data\n",
                    command);
                if(!ignore_bad_l2a) exit(1);
                break;
            default:
                fprintf(stderr, "%s: unknown status\n", command);
                if(!ignore_bad_l2a) exit(1);
            }
            break;        // done, exit do loop
        }

        //---------//
        // convert //
        //---------//

#ifdef LATLON_LIMIT_HACK
        // start hack
        Meas* tstmeas = l2a.frame.measList.GetHead();
        double alt, lat, lon;
        if (! tstmeas)
        {
            printf("NULL MeasList \n");
            continue;
        }
        else
        {
            tstmeas->centroid.GetAltLonGDLat(&alt, &lon, &lat);
            lon*=rtd;
            lat*=rtd;
        }
        if (lat < 23.80 && lat > 23.78 && lon < 296.06 && lon > 296.04)
        {
        // end hack
#endif

        int retval = l2a_to_l2b.ConvertAndWrite(&l2a, &gmf, &kp, &l2b);

#ifdef LATLON_LIMIT_HACK
        // start hack
        if (retval != 1)
        {
            printf("%g lat %g lon   retval=%d\n", lat, lon, retval);
        }
        // end hack
#endif

        switch (retval)
        {
        case 1:
            break;
        case 2:
            break;
        case 4:
        case 5:
            break;
        case 0:
            fprintf(stderr, "%s: error converting Level 2A to Level 2B\n",
                command);
            exit(1);
            break;
        }

#ifdef LATLON_LIMIT_HACK
        // start hack
        }
        // end hack
#endif

    }

    l2a_to_l2b.InitFilterAndFlush(&l2b);

    l2a.Close();
    l2b.Close();

    return (0);
}
