//==============================================================//
// Copyright (C) 1998-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    l2b_get_high_speeds
//
// SYNOPSIS
//    l2b_get_high_speeds <input_file> <output_file> [ cti ] [ ati ]
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
    "<hdf_flag (1 for hdf, 0 otherwise)>", "< min_speed>", 0};

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
    if (argc != 5)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* input_file = argv[clidx++];
    const char* output_file = argv[clidx++];
    int hdf_flag = atoi(argv[clidx++]);
    float min_speed = atof(argv[clidx++]);

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

    FILE* ofp = fopen(output_file, "w");
    for(int cti=0;cti<l2b.frame.swath.GetCrossTrackBins();cti++){
      for(int ati=0;ati<l2b.frame.swath.GetAlongTrackBins();ati++){
	WVC* wvc = l2b.frame.swath.swath[cti][ati];
	if (! wvc) continue;
	if (! wvc->selected )  continue;
	if (wvc->selected->spd < min_speed) continue;
	fprintf(ofp,"%d %d %g %g\n",cti,ati,wvc->selected->spd,wvc->selected->dir);
      }
    }
    fclose(ofp);
  

    //----------------------//
    // close files and exit //
    //----------------------//

    l2b.Close();
    return(0);
}
