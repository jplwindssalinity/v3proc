//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    l2a_filter
//
// SYNOPSIS
//    l2a_filter [ -f filter ] [ -h ] <input_file> <output_file>
//
// DESCRIPTION
//    Filters a l2a file based on the filters specified.
//
// OPTIONS
//    [ - filter ]  Use the specified filter.
//    [ -h ]        Help. Displays the list of filters.
//
// OPERANDS
//    The following operands are supported:
//      <input_file>   The input L2A file.
//      <output_file>  The output L2A file.
//
// EXAMPLES
//    An example of a command line is:
//      % l2a_filter -f corr0 l2a.dat l2a.nocorr.dat
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
#include "Misc.h"
#include "L2A.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<EarthPosition>;
template class List<Meas>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<long>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "f:h"

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -f filter ]", "[ -h ]", "<input_file>",
    "<output_file>", 0 };

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    //------------//
    // initialize //
    //------------//

    int opt_no_correlation = 0;
    int opt_no_aft_look = 0;
    int opt_no_fore_look = 0;
    int opt_no_inner_beam = 0;
    int opt_no_outer_beam = 0;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    extern int optind;
    extern char *optarg;
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'f':
            if (strcasecmp(optarg, "corr0") == 0)
            {
                opt_no_correlation = 1;
            }
            else if (strcasecmp(optarg, "aft0") == 0)
            {
                opt_no_aft_look = 1;
            }
            else if (strcasecmp(optarg, "fore0") == 0)
            {
                opt_no_fore_look = 1;
            }
            else if (strcasecmp(optarg, "outer0") == 0)
            {
                opt_no_outer_beam = 1;
            }
            else if (strcasecmp(optarg, "inner0") == 0)
            {
                opt_no_inner_beam = 1;
            }
            else
            {
                fprintf(stderr, "%s: error parsing filter %s\n", command,
                    optarg);
                exit(1);
            }
            break;
        case 'h':
            printf("Filters:\n");
            printf("  corr0  : Remove correlation measurements\n");
            printf("  aft0   : Remove aft look measurements\n");
            printf("  fore0  : Remove fore look measurements\n");
            printf("  inner0 : Remove inner beam measurements\n");
            printf("  outer0 : Remove outer beam  measurements\n");
            exit(0);
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc != optind + 2)
        usage(command, usage_array, 1);

    const char* input_file = argv[optind++];
    const char* output_file = argv[optind++];

    //-------------------------------//
    // create and open the input L2A //
    //-------------------------------//

    L2A l2a;
    if (! l2a.OpenForReading(input_file))
    {
        fprintf(stderr, "%s: error opening input file %s\n", command,
            input_file);
        exit(1);
    }

    //--------------------------//
    // open the output L2A file //
    //--------------------------//

    if (! l2a.OpenForWriting(output_file))
    {
        fprintf(stderr, "%s: error creating output file %s\n", command,
            input_file);
        exit(1);
    }       

    //-------------//
    // copy header //
    //-------------//
  
    l2a.ReadHeader();
    l2a.WriteHeader();

    //-------------//
    // filter loop //
    //-------------//

    int wvc_in = 0;
    int wvc_out = 0;
    int remove=0;
    L2AFrame* frame = &(l2a.frame);
    while (l2a.ReadDataRec())
    {            

      for (Meas* meas = frame->measList.GetHead(); meas; )
        {
	  wvc_in++;
	  remove = opt_no_correlation &&
	      (meas->measType == Meas::VV_HV_CORR_MEAS_TYPE ||
	       meas->measType == Meas::HH_VH_CORR_MEAS_TYPE);
          remove= remove || (opt_no_aft_look && meas->scanAngle>pi/2 
		  && meas->scanAngle<3*pi/2);
          remove= remove || (opt_no_fore_look && (meas->scanAngle<pi/2 ||
						  meas->scanAngle>3*pi/2));
          remove= remove || (opt_no_inner_beam && meas->beamIdx==0);
          remove= remove || (opt_no_outer_beam && meas->beamIdx==1);
	  if(remove)
	    {
	      meas = frame->measList.RemoveCurrent();
	      delete meas;
	      meas = frame->measList.GetCurrent();
	    }
	  else
	    {
	      meas = frame->measList.GetNext();
	      wvc_out++;
	    }
	}
        l2a.WriteDataRec();
    }

    //----------------------//
    // close files and exit //
    //----------------------//

    l2a.Close();
    printf("WVC In: %d,  WVC Out: %d\n", wvc_in, wvc_out);

    return(0);
}
