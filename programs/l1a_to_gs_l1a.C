//==========================================================//
// Copyright (C) 1998, California Institute of Technology.  //
// U.S. Government sponsorship acknowledged.                //
//==========================================================//

//----------------------------------------------------------------------
// NAME
//      l1a_to_gs_l1a
//
// SYNOPSIS
//        l1a_to_gs_l1a <config_file> <input_file> <output_file>  <start_frame> <end_frame>
//
// DESCRIPTION
//          Reads frames start_frame through end_frame from a L1b file and
//          writes them to an ASCII file
//      OPTIONS
//          Last two arguments are optional
// AUTHOR
//          Sally Chou
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

//-----------//
// TEMPLATES //
//-----------//

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
    if (argc != 6 && argc!=4)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* config_file = argv[clidx++];
    const char* input_file = argv[clidx++];
    const char* output_file = argv[clidx++];
        int start_frame=-1, end_frame=2;
        if(argc==6){
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
    // create L1A object      //
    //------------------------//
    L1A l1a;
    if (! ConfigL1A(&l1a, &config_list))
    {
        fprintf(stderr, "%s: error configuring Level 1A Product\n", command);
        exit(1);
    }
        
    //------------------------//
    // open the input file    //
    //------------------------//

    if (! l1a.OpenForReading(input_file))
    {
        fprintf(stderr, "%s: error opening input file %s\n", command,
            input_file);
        exit(1);
    }

    //------------------------//
    // open the output file   //
    //------------------------//

    if (! l1a.OpenForWriting(output_file))
    {
        fprintf(stderr, "%s: error creating output file %s\n", command,
            input_file);
        exit(1);
    }       

    int frame_number=1;

    //---------------------//
    // copy desired frames //
    //---------------------//

    while (l1a.ReadDataRec() && frame_number <= end_frame)
    {
        if (frame_number >= start_frame)
        {
            l1a.frame.Unpack(l1a.buffer);
            l1a.WriteGSDataRec();
        }
        if (start_frame>=0) frame_number++;
    }

    //----------------------//
    // close files and exit //
    //----------------------//
    l1a.Close();
    return(0);
}
