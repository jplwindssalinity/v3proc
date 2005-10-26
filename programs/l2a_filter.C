//==============================================================//
// Copyright (C) 1998-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    l2a_filter
//
// SYNOPSIS
//    l2a_filter [ -f filter ] [ -hl12 ] <input_file> <output_file>
//
// DESCRIPTION
//    Filters a l2a file based on the filters specified.
//
// OPTIONS
//    [ - filter ]  Use the specified filter.
//    [ -h ]        Help. Displays the list of filters.
//    [ -l ]        Land Map to use to filter land (default is use landflag)
//    [ -1 ]        Beam 1 Sigma0 Correction in dB
//    [ -2 ]        Beam 2 Sigma0 Correction in dB
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
#include <unistd.h>
#include <stdlib.h>
#include "Misc.h"
#include "L2A.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"
#include "LandMap.h"

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

#define OPTSTRING  "f:hl:1:2:"

#define NO_COPOL_STRING         "copol0"
#define NO_START_FRAMES_STRING  "start0"

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -f filter ]", "[ -h ]","[ -l landmap_file]", "<input_file>","<output_file>", 0 };

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
    int opt_no_copol = 0;
    int opt_no_start_frames = 0;
    int opt_no_aft_look = 0;
    int opt_no_fore_look = 0;
    int opt_no_inner_fore = 0;
    int opt_no_inner_beam = 0;
    int opt_no_outer_beam = 0;
    int opt_no_HHVH=0;
    int opt_no_VVHV=0;
    int opt_no_land=0;
    int opt_beam_balance=0;
    float beam1_multiplier=1;
    float beam2_multiplier=1;

    SimpleLandMap* land=NULL;

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
            else if (strcasecmp(optarg, NO_COPOL_STRING) == 0)
            {
                opt_no_copol = 1;
            }
            else if (strcasecmp(optarg, NO_START_FRAMES_STRING) == 0)
            {
                opt_no_start_frames = 1;
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
	    else if (strcasecmp(optarg, "innerfore0") == 0)
            {
                opt_no_inner_fore = 1;
            }
            else if (strcasecmp(optarg, "HHVH0") == 0)
            {
                opt_no_HHVH = 1;
            }
            else if (strcasecmp(optarg, "VVHV0") == 0)
            {
                opt_no_VVHV = 1;
            }
            else if (strcasecmp(optarg, "LAND0") == 0)
            {
                opt_no_land = 1;
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
            printf("%8s : Remove copolarization measurements\n",
                NO_COPOL_STRING);
            printf("%8s : Remove first two frames\n",
                NO_START_FRAMES_STRING);
            printf("  aft0   : Remove aft look measurements\n");
            printf("  fore0  : Remove fore look measurements\n");
            printf("  inner0 : Remove inner beam measurements\n");
	    printf("  innerfore0 : Remove inner beam fore look measurements\n");
            printf("  outer0 : Remove outer beam  measurements\n");
            printf("  HHVH0 : Remove HHVH  measurements\n");
            printf("  VVHV0 : Remove VVHV  measurements\n");
            printf("  LAND0 : Remove land  measurements\n");
            exit(0);
            break;
        case 'l':
            land=new SimpleLandMap;
            land->Read(optarg);
            break;
        case '1':
            opt_beam_balance=1;
            beam1_multiplier=atof(optarg);
            beam1_multiplier=pow(10.0,0.1*beam1_multiplier);
            break;   
        case '2':
            opt_beam_balance=1;
            beam2_multiplier=atof(optarg);
            beam2_multiplier=pow(10.0,0.1*beam2_multiplier);
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

	    // Estimate Scan Angle if its not available (officila proc)
            // Assumes spacecraft vector is due North
            if(meas->numSlices==-1){
	      meas->scanAngle=meas->eastAzimuth-pi/2;
              while(meas->scanAngle < 0) meas->scanAngle+=2*pi;
	    }
            remove = opt_no_correlation &&
                (meas->measType == Meas::VV_HV_CORR_MEAS_TYPE ||
                meas->measType == Meas::HH_VH_CORR_MEAS_TYPE);

            // no copol measurements
            remove = remove || (opt_no_copol &&
                (meas->measType == Meas::VV_MEAS_TYPE ||
                meas->measType == Meas::HH_MEAS_TYPE));

            // start frames (first two frames)
            remove = remove || (opt_no_start_frames &&
                (wvc_in == 1 || wvc_in == 2));

            remove = remove || (opt_no_aft_look && meas->scanAngle>pi/2 &&
                meas->scanAngle < 3*pi/2);
            remove = remove || (opt_no_fore_look && (meas->scanAngle<pi/2 ||
                meas->scanAngle>3*pi/2));
            remove = remove || (opt_no_inner_beam && meas->beamIdx==0);
            remove = remove || (opt_no_inner_fore &&  meas->beamIdx==0 &&
			 (meas->scanAngle<pi/2 ||meas->scanAngle>3*pi/2));
            remove = remove || (opt_no_outer_beam && meas->beamIdx==1);
            remove = remove || ( opt_no_HHVH &&
                (meas->measType == Meas::HH_VH_CORR_MEAS_TYPE));
            remove = remove || ( opt_no_VVHV &&
                (meas->measType == Meas::VV_HV_CORR_MEAS_TYPE));
            
            int landflag=meas->landFlag;
            if(land){ 
               double alt,lon,lat;
	       meas->centroid.GetAltLonGCLat(&alt,&lon,&lat);
               landflag= landflag || land->IsLand(lon,lat);
	    }
            remove = remove || (opt_no_land && landflag);

            if(remove)
            {
                meas = frame->measList.RemoveCurrent();
                delete meas;
                meas = frame->measList.GetCurrent();
            }
            else
            {
		if(opt_beam_balance){
		  switch(meas->beamIdx){
		  case 0:
		    meas->value*=beam1_multiplier;
		    break;
		  case 1:
		    meas->value*=beam2_multiplier;
		    break;
		  }
		}
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





