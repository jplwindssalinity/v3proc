//==========================================================//
// Copyright (C) 1998, California Institute of Technology.  //
// U.S. Government sponsorship acknowledged.                //
//==========================================================//

//----------------------------------------------------------------------
// NAME
//    expand_calpulse
//
// SYNOPSIS
//    expand_calpulse <config_file> <input_file> <output_file>
//        <start_frame> <end_frame>
//
// DESCRIPTION
//    Reads frames start_frame through end_frame from a L1b file and
//    writes them to an ASCII file
//
// OPTIONS
//    Last two arguments are optional
//
// AUTHOR
//    Sally Chou
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

const char* usage_array[] = { "<cal_pulse_file>", "<extra frames>",0};

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

    if (argc != 3)
    {
        usage(command, usage_array, 1);
    }

    const char* cal_pulse_file_name = argv[clidx++];
    const char* extra_frames_str = argv[clidx++];
    int extra_frames = atoi(extra_frames_str);

    // zero out the cal pulse buffer
    char calPulseBuffer[GS_CAL_PULSE_FRAME_SIZE];
    (void)memset(calPulseBuffer, 0, GS_CAL_PULSE_FRAME_SIZE);

    //------------------------//
    // open the input file    //
    //------------------------//

    FILE* cal_pulse_file = fopen(cal_pulse_file_name,"r");
    if (cal_pulse_file == NULL)
    {
      fprintf(stderr,"Error opening cal pulse file %s\n",cal_pulse_file_name);
      exit(1);
    }

    //---------------------------//
    // read all cal pulse frames //
    //---------------------------//

    unsigned int frame_count = 0;
    while (fread(calPulseBuffer,GS_CAL_PULSE_FRAME_SIZE,1,cal_pulse_file) == 1)
    {
      frame_count++;
    }
    if (frame_count == 0)
    {
      fprintf(stderr,"Error reading cal pulse records\n");
      exit(1);
    }
    printf("Read %d calpulse records in original file\n",frame_count);
    
    char* ptr = calPulseBuffer;
    double time;
    (void)memcpy(&time, ptr, sizeof(double));

    fclose(cal_pulse_file);
    cal_pulse_file = fopen(cal_pulse_file_name,"a");
    if (cal_pulse_file == NULL)
    {
      fprintf(stderr,"Error opening cal pulse file %s\n",cal_pulse_file_name);
      exit(1);
    }

    //----------------------------//
    // add extra cal pulse frames //
    //----------------------------//

    for (int i=0; i < extra_frames; i++)
    {
      time += 3.24;
      (void)memcpy(ptr, &time, sizeof(double));
      if (fwrite(calPulseBuffer,GS_CAL_PULSE_FRAME_SIZE,1,cal_pulse_file) != 1)
      {
        fprintf(stderr,"Error writing cal pulse record\n");
        exit(1);
      }
    }

    //----------------------//
    // close files and exit //
    //----------------------//

    fclose(cal_pulse_file);

    return(0);
}
