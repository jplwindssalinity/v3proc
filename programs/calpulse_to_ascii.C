//==========================================================//
// Copyright (C) 1998, California Institute of Technology.  //
// U.S. Government sponsorship acknowledged.                //
//==========================================================//

//----------------------------------------------------------------------
// NAME
//      calpulse_to_ascii
//
// SYNOPSIS
//        calpulse_to_ascii <input_file> <output_file> <start_frame> <end_frame>
//
// DESCRIPTION
//          Reads frames start_frame through end_frame from a calpulse file and
//          writes them to an ASCII file
//      OPTIONS
//          Last two arguments are optional
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
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "ConfigList.h"
#include "Spacecraft.h"
#include "ConfigSim.h"
#include "Array.h"
#include "Meas.h"
#include "Ephemeris.h"
#include "Wind.h"
#include "Kpm.h"
#include "Tracking.h"
#include "Tracking.C"
#include "L1AGSFrame.h"

#define OPTSTRING               "a"

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
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

const char* usage_array[] = { "<config_file>","<input_file>", "<output_file>",
                  "<start_frame>(OPT)",
                  "<end_frame>(OPT)",0};

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int     argc,
    char*   argv[])
{
    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);
    int clidx = 1;

    if (argc != 6 && argc != 4)
        usage(command, usage_array, 1);

    const char* config_file = argv[clidx++];
    const char* cal_pulse_file_name = argv[clidx++];
    const char* output_name = argv[clidx++];

    int start_frame = -1;
    int end_frame = 2;
    if (argc == 5)
    {
      start_frame=atoi(argv[clidx++]);
      end_frame=atoi(argv[clidx++]);
    }

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


    //------------------------//
    // Create L1A object      //
    //------------------------//

    L1A l1a;
  
    if (! ConfigL1A(&l1a, &config_list))
    {
        fprintf(stderr, "%s: error configuring Level 1A Product\n", command);
        exit(1);
    }

    //-------------------------------//
    // Open the calpulse ascii file  //
    //-------------------------------//

    if (! l1a.OpenCalPulseForWriting(output_name))
    {
        fprintf(stderr, "%s: error creating Cal Pulse file %s\n", command,
            output_name);
        exit(1);
    }

    //-------------------------------//
    // Open the calpulse input file  //
    //-------------------------------//

    FILE* cal_pulse_file = fopen(cal_pulse_file_name,"r");
    if (cal_pulse_file == NULL)
    {
      fprintf(stderr,"Error opening cal pulse file %s\n",cal_pulse_file_name);
      exit(1);
    }

    //--------------------------//
    // Translate desired frames //
    //--------------------------//

    int frame_number=1;
    while (l1a.ReadGSCalPulseRec(cal_pulse_file) && frame_number <= end_frame)
    {
      if(frame_number >= start_frame)
      {
        if (l1a.WriteGSCalPulseRecAscii() == 0)
        {
          fprintf(stderr,
          "%s: error writing Cal Pulse record\n", command);
          exit(1);
        }
      }
      if(start_frame >= 0) frame_number++;
    }

    //----------------------//
    // close files and exit //
    //----------------------//

    l1a.Close();
    fclose(cal_pulse_file);

    return(0);
}