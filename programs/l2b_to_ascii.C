//==============================================================//
// Copyright (C) 1998-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    l2b_to_ascii
//
// SYNOPSIS
//    l2b_to_ascii <input_file> <output_file> [ cti ] [ ati ]
//
// DESCRIPTION
//    Reads frames start_frame through end_frame from a L1b file and
//    writes them to an ASCII file
//
// OPTIONS
//    Last two arguments are optional
//
// AUTHOR
//    Bryan Stiles
//    bstiles@acid.jpl.nasa.gov
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
#include "Misc.h"
#include "L2B.h"
#include "List.h"
#include "BufferedList.h"
#include "Tracking.h"

//-----------//
// TEMPLATES //
//-----------//

class AngleInterval;
template class List<AngleInterval>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<long>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

const char* usage_array[] = { "<input_file>", "<output_file>",
    "<hdf_flag (1 for hdf, default 0)>", "[ cti ]", "[ ati ]", 0};

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
    if (argc != 3 && argc != 4 && argc != 6)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* input_file = argv[clidx++];
    const char* output_file = argv[clidx++];
    int ati = 0;
    int cti = 0;
    int hdf_flag = 0;
    if (argc >= 4)
    {
        hdf_flag = atoi(argv[clidx++]);
    }
    if (argc == 6)
    {
        cti = atoi(argv[clidx++]);
        ati = atoi(argv[clidx++]);
    }

    //-------------------//
    // create L2B object //
    //-------------------//

    L2B l2b;

    //---------------------//
    // open the input file //
    //---------------------//

    if (hdf_flag)
    {
/*
        l2b.SetInputFilename(input_file);
        if (l2b.ReadHDF() == 0)
*/
        if (! l2b.ReadPureHdf(input_file))
        {
            fprintf(stderr, "%s: error reading HDF L2B file %s\n", command,
                input_file);
            exit(1);
        }
    }
    else
    {
        if (! l2b.OpenForReading(input_file))
        {
            fprintf(stderr, "%s: error opening input file %s\n", command,
                input_file);
            exit(1);
        }

        //---------------------//
        // copy desired frames //
        //---------------------//

        if (! l2b.ReadHeader())
        {
            fprintf(stderr, "%s: error reading from input file %s\n", command,
                input_file);
            exit(1);
        }
        if (! l2b.ReadDataRec())
        {
            fprintf(stderr, "%s: error reading from input file %s\n", command,
                input_file);
            exit(1);
        }
    }

    if (argc <= 4)
    {
        //----------------------//
        // open the output file //
        //----------------------//

        if (! l2b.OpenForWriting(output_file))
        {
            fprintf(stderr, "%s: error creating output file %s\n", command,
                input_file);
            exit(1);
        }      

        if (! l2b.WriteAscii())
        {
            fprintf(stderr, "%s: error writing to output file %s\n", command,
                output_file);
            exit(1);
        }
    }
    else
    {
        FILE* ofp = fopen(output_file, "w");
        WVC* wvc = l2b.frame.swath.swath[cti][ati];
        if (! wvc)
        {
            fprintf(stderr,
                "No wind vector cell available at cti=%d, ati=%d\n", cti, ati);
            exit(0);
        }
        wvc->WriteAscii(ofp);
        fclose(ofp);
    }

    //----------------------//
    // close files and exit //
    //----------------------//

    l2b.Close();
    return(0);
}
