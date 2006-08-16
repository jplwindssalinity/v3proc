//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    l1a_to_ascii
//
// SYNOPSIS
//    l1a_to_ascii <config_file> <input_file> <output_file>
//      [ start_frame ] [ end_frame ]
//
// DESCRIPTION
//    Reads frames start_frame through end_frame from a PscatL1A file
//    and writes them to an ASCII file
//
// AUTHORS
//    James Huddleston (hudd@casket.jpl.nasa.gov)
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
#include "PscatL1A.h"
#include "PscatConfig.h"

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
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template list<string>;
template map<string,string,Options::ltstr>;

const char* usage_array[] = { "<config_file>", "<input_file>",
    "<output_file>", "[ start_frame ]", "[ end_frame]", 0};

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
    // create PscatL1A object //
    //------------------------//

    PscatL1A l1a;
    if (! ConfigPscatL1A(&l1a, &config_list))
    {
        fprintf(stderr, "%s: error configuring Pscat Level 1A\n", command);
        exit(1);
    }
        
    //---------------------//
    // open the input file //
    //---------------------//

    if (! l1a.OpenForReading(input_file))
    {
        fprintf(stderr, "%s: error opening Pscat L1A file %s\n", command,
            input_file);
        exit(1);
    }

    //----------------------//
    // open the output file //
    //----------------------//

    FILE* ofp = fopen(output_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error creating output file %s\n", command,
            output_file);
        exit(1);
    }       

    int frame_number = 0;

    //---------------------//
    // copy desired frames //
    //---------------------//

    while (l1a.ReadDataRec())
    {
        if (argc==6) frame_number++;
        if (frame_number < start_frame)
            continue;
        if (frame_number > end_frame)
            break;

        l1a.frame.Unpack(l1a.buffer);
        l1a.WriteDataRecAscii(ofp);
    }

    //----------------------//
    // close files and exit //
    //----------------------//

    l1a.Close();
    fclose(ofp);
    
    return(0);
}
