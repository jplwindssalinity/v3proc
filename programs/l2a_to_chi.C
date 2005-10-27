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
template class List<long>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<AngleInterval>;

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

const char* usage_array[] = { "<sim_config_file>", 0};

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

    const char* command = no_path(argv[0]);
    if (argc != 2)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* config_file = argv[clidx++];

    //------------------------//
    // tell how far you have  //
    // gotten if you recieve  //
    // the siguser1 signal    //
    //------------------------//

    //sigset(SIGUSR1, &report);

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
    // create and configure L2A //
    //-------------------------------------//

    L2A l2a;
    if (! ConfigL2A(&l2a, &config_list))
    {
        fprintf(stderr, "%s: error configuring Level 2A Product\n", command);
        exit(1);
    }

    //-------------------------------------//
    // create and configure wind field     //
    //-------------------------------------//

    WindField wind_field;
    if (! ConfigWindField(&wind_field, &config_list))
    {
        fprintf(stderr, "%s: error configuring Wind Field\n", command);
        exit(1);
    }
  
  
    //------------//
    // open files //
    //------------//

    l2a.OpenForReading();
  
    //---------------------------------//
    // read the header to set up swath //
    //---------------------------------//

    if (! l2a.ReadHeader())
    {
        fprintf(stderr, "%s: error reading Level 2A header\n", command);
        exit(1);
    }

 
 
    //-----------------//
    // loop through data//
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
                exit(1);
                break;
            case L2A::ERROR_UNKNOWN:
                fprintf(stderr, "%s: unknown error reading Level 2A data\n",
                    command);
                exit(1);
                break;
            default:
                fprintf(stderr, "%s: unknown status\n", command);
                exit(1);
            }
            break;        // done, exit do loop
        }

        //------------//
        // compute chi//
        //------------//
	int ati=l2a.frame.ati;
        int cti=l2a.frame.cti;
        for(Meas* meas=l2a.frame.measList.GetHead();meas; meas=l2a.frame.measList.GetNext()){ 
	  double alt, lat, lon;
	  if (! meas->centroid.GetAltLonGDLat(&alt, &lon, &lat))
	       continue;
	  LonLat lon_lat;
	  lon_lat.longitude = lon;
	  lon_lat.latitude = lat;
	  WindVector wv;
	  if (! wind_field.InterpolatedWindVector(lon_lat, &wv))
            {
	      continue;
            }
	  float chi = wv.dir - meas->eastAzimuth + pi;
	  printf("%d %d %d %g\n",meas->beamIdx, ati, cti, chi*rtd);
	}
        


    }
    return (0);
}
