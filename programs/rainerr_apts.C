//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    rain_apts
//
// SYNOPSIS
//    rain_apts [ -f ] [ -m minutes ] [ -i irr_thresh ] [ -v thresh ]
//        <rain/flag_file> <mudh_file> <output_base>
//
// DESCRIPTION
//    Generates apts files of rain rate, integrated rain rate,
//    rain index value, and rain flag.
//
// OPTIONS
//    [ -f ]             It's a flag file, not a rain file.
//    [ -m minutes ]     Collocated within minutes.
//    [ -i irr_thresh ]  Threshold the integrated rain rate.
//    [ -v thresh ]      Threshold the flag file value for "rain".
//                       (Instead of using the flag in the file.)
//
// OPERANDS
//    <rain/flag_file>  The input rain or flag file.
//    <mudh_file>       The matching MUDH file (for lon and lat)
//    <output_base>     The output base.
//
// EXAMPLES
//    An example of a command line is:
//      % rain_apts 1208.rain 1208.mud 1208
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
// AUTHOR
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Misc.h"
#include "ParTab.h"
#include "ArgDefs.h"
#include "ConfigList.h"
#include "PolyErrNo.h"
#include "PolyTable.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"
#include "GMF.h"
#include "ConfigSimDefs.h"
#include "L2B.h"
#include "ConfigSim.h"
#include "Interpolate.h"
#include "SeaPac.h"
#include "mudh.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<Meas>;
template class List<MeasSpot>;
template class List<OffsetList>;
template class List<long>;
template class List<OrbitState>;
template class BufferedList<OrbitState>;
template class List<EarthPosition>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<WindVectorPlus>;
template class List<AngleInterval>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING    "m:i:v:"

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -m minutes ]",
    "[ -i irr_thresh ]", "[ -v thresh ]", "<irain_file>", "<flag_file>" "<mudh_file>",
    "<output_base>", 0 };

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

    int minutes = 60;
    float irr_thresh = 2.0;
    int opt_flag_thresh = 0;
    float flag_thresh = 0.0;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'm':
            minutes = atoi(optarg);
            break;
        case 'i':
            irr_thresh = atof(optarg);
            break;
        case 'v':
            flag_thresh = atof(optarg);
            opt_flag_thresh = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 4)
        usage(command, usage_array, 1);

    const char* irr_file = argv[optind++];
    const char* flag_file = argv[optind++];
    const char* mudh_file = argv[optind++];
    const char* output_base = argv[optind++];

    //----------------//
    // read mudh file //
    //----------------//

    unsigned short nbd_array[AT_WIDTH][CT_WIDTH];
    unsigned short spd_array[AT_WIDTH][CT_WIDTH];
    unsigned short dir_array[AT_WIDTH][CT_WIDTH];
    unsigned short mle_array[AT_WIDTH][CT_WIDTH];
    unsigned short lon_array[AT_WIDTH][CT_WIDTH];
    unsigned short lat_array[AT_WIDTH][CT_WIDTH];

    FILE* mudh_ifp = fopen(mudh_file, "r");
    if (mudh_ifp == NULL)
    {
        fprintf(stderr, "%s: error opening MUDH file %s\n", command,
            mudh_file);
        exit(1);
    }
    unsigned long size = CT_WIDTH * AT_WIDTH;
    if (fread(nbd_array, sizeof(short), size, mudh_ifp) != size ||
        fread(spd_array, sizeof(short), size, mudh_ifp) != size ||
        fread(dir_array, sizeof(short), size, mudh_ifp) != size ||
        fread(mle_array, sizeof(short), size, mudh_ifp) != size ||
        fread(lon_array, sizeof(short), size, mudh_ifp) != size ||
        fread(lat_array, sizeof(short), size, mudh_ifp) != size)
    {
        fclose(mudh_ifp);
        fprintf(stderr, "%s: error reading MUDH file %s\n", command,
            mudh_file);
        exit(1);
    }
    fclose(mudh_ifp);

    //-------------------------//
    // Set up ARRAYS           //
    //-------------------------//
    unsigned char  rain_rate[AT_WIDTH][CT_WIDTH];
    unsigned char  time_dif[AT_WIDTH][CT_WIDTH];
    unsigned char  which_ssmi[AT_WIDTH][CT_WIDTH];
    unsigned short integrated_rain_rate[AT_WIDTH][CT_WIDTH];

    float          index_tab[AT_WIDTH][CT_WIDTH];
    unsigned char  flag_tab[AT_WIDTH][CT_WIDTH];


    //-----------------//
    // read irain file //
    //-----------------//
    FILE* ifp = fopen(irr_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening irain file %s\n", command,
            irr_file);
        exit(1);
    }

    fread(rain_rate, sizeof(char), CT_WIDTH * AT_WIDTH, ifp);
    fread(time_dif, sizeof(char), CT_WIDTH * AT_WIDTH, ifp);
    fread(which_ssmi, sizeof(char), CT_WIDTH * AT_WIDTH, ifp);
    fread(integrated_rain_rate, sizeof(short), CT_WIDTH * AT_WIDTH, ifp);
    fclose(ifp);

    //-----------------//
    // read flag file  //
    //-----------------//
    ifp = fopen(flag_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening flag file %s\n", command,
            flag_file);
        exit(1);
    }

    fread(index_tab, sizeof(float), CT_WIDTH * AT_WIDTH, ifp);
    fread(flag_tab,   sizeof(char), CT_WIDTH * AT_WIDTH, ifp);
    fclose(ifp);

    //---------------------------------------//
    // eliminate rain data out of time range //
    //---------------------------------------//

    for (int ati = 0; ati < AT_WIDTH; ati++)
      {
	for (int cti = 0; cti < CT_WIDTH; cti++)
	  {
	    int co_time = time_dif[ati][cti] * 2 - 180;
	    if (abs(co_time) > minutes ||
		lon_array[ati][cti] == MAX_SHORT ||
		lat_array[ati][cti] == MAX_SHORT ||
		flag_tab[ati][cti]  == 2 )
	      {
		// flag as bad
		integrated_rain_rate[ati][cti] = 2000;
	      }
	  }
      }

    //-----------------------------------------//
    // rain err flag file                      //
    //-----------------------------------------//

    char filename[1024];
    sprintf(filename, "%s.ref.apts", output_base);
    FILE* ref_ofp = fopen(filename, "w");
    if (ref_ofp == NULL)
      {
	fprintf(stderr,
                "%s: error opening integrated rain rate flag file %s\n",
                command, filename);
	exit(1);
      }
    fprintf(ref_ofp, "apts\n");
    for (int ati = 0; ati < AT_WIDTH; ati++)
      {
	for (int cti = 0; cti < CT_WIDTH; cti++)
	  {
	    if (integrated_rain_rate[ati][cti] >= 1000)
	      continue;
	    float lon = (float)lon_array[ati][cti] * 0.01;
	    float lat = (float)lat_array[ati][cti] * 0.01 - 90.0;
	    float irr = integrated_rain_rate[ati][cti] * 0.1;
	    int ref;
	    if (irr == 0.0)
	      ref = 1;
	    else if (irr <= irr_thresh)
	      ref = 2;
	    else
	      ref = 3;
	    if (opt_flag_thresh)
	      {
		// re-threshold to get the flag
		if (index_tab[ati][cti] > flag_thresh)
		  ref+=3;
	      }
	    else
	      {
		// use the file's flag
		if (flag_tab[ati][cti] == 1)
		  ref+=3;
	      }                
	    fprintf(ref_ofp, "%g %g %d\n", lon, lat, ref);
	  }
      }
    fclose(ref_ofp);
    
    return (0);
}
