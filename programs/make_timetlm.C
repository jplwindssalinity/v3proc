//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		make_timetlm
//
// SYNOPSIS
//	 make_timetlm <config_file> <input_file> <output_file>  <start_frame> <end_frame>
//
// DESCRIPTION
//   Reads data from a GS-native l1a file, and
//   constructs a time telemetry file for use as an input to the SEAPAC
//   level 1A processor (along with the ephemeris and attitude files and
//   the level 0 telemetry data file produced from the same GS-native L1A file).
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

#define OPTSTRING				"c:o:i:"

const char* usage_array[] = { "[ -c config_file ]", "[ -i l1ags_file ]",
    "[ -o output_file ]", 0 };

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int		argc,
    char*	argv[])
{
    char* config_file = NULL;
    char* l1ags_file = NULL;
    char* output_file = NULL;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    while (1)
    {
      int c = getopt(argc, argv, OPTSTRING);
      if (c == 'c')
      {
        config_file = optarg;
      }
      else if (c == 'i')
      {
        l1ags_file = optarg;
      }
      else if (c == 'o')
      {
        output_file = optarg;
      }
      else if (c == -1) break;
    }

	//---------------------//
	// check for arguments //
	//---------------------//

    if (config_file == NULL)
    {
	  usage(command, usage_array, 1);
      exit(-1);
    }
    if (l1ags_file == NULL)
    {
	  usage(command, usage_array, 1);
      exit(-1);
    }
    if (output_file == NULL)
    {
	  usage(command, usage_array, 1);
      exit(-1);
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

 	if (! l1a.OpenForReading(l1ags_file))
    {
    	fprintf(stderr, "%s: error opening input file %s\n", command,
    		l1ags_file);
    	exit(1);
    }

    //------------------------//
    // open the output file   //
    //------------------------//

    FILE* output_stream = fopen(output_file,"w");
 	if (output_stream == NULL)
    {
    	fprintf(stderr, "%s: error creating output file %s\n", command,
    		output_file);
    	exit(1);
    }       

    //--------------------------------//
    // Get time info from first frame //
    //--------------------------------//

    if (! l1a.ReadGSDataRec() )
    {
    	fprintf(stderr, "%s: error reading from %s\n", command,
    		l1ags_file);
    	exit(1);
    }       
    l1a.gsFrame.Unpack(l1a.gsBuffer);

    char utc1[25];
    utc1[24] = '\0';
    (void)memcpy(utc1, l1a.gsFrame.in_pcd.frame_time, 24);
    unsigned int i_vtcw1;
    (void)memcpy((void*)&i_vtcw1,
                 (void*)(l1a.gsFrame.status.corres_instr_time+1),
                 sizeof(unsigned int));
    double vtcw1 = l1a.frame.VTCWToDouble(l1a.gsFrame.status.vtcw);

    //-------------------------------//
    // Get time info from last frame //
    //-------------------------------//

    int frame_number=1;

    while (l1a.ReadGSDataRec())
    {
      frame_number++;
    }
    l1a.gsFrame.Unpack(l1a.gsBuffer);

    char utc2[25];
    utc2[24] = '\0';
    (void)memcpy(utc2, l1a.gsFrame.in_pcd.frame_time, 24);
    unsigned int i_vtcw2;
    (void)memcpy((void*)&i_vtcw2,
                 (void*)(l1a.gsFrame.status.corres_instr_time+1),
                 sizeof(unsigned int));
    double vtcw2 = l1a.frame.VTCWToDouble(l1a.gsFrame.status.vtcw);

    //---------------------------------------//
    // Compute time correlation coefficients //
    //---------------------------------------//

    double slope = ((double)i_vtcw1 - (double)i_vtcw2)/32.0 /
                    ((vtcw1 - vtcw2)/1e6);
    double intercept = (double)i_vtcw1/32.0 - slope*vtcw1/1e6;

/*
    printf("utc1 = %24s\n",utc1);
    printf("i_vtcw1 = %d\n",i_vtcw1);
    printf("vtcw1 = %16.2f\n",vtcw1);
    printf("utc2 = %24s\n",utc2);
    printf("i_vtcw2 = %d\n",i_vtcw2);
    printf("vtcw2 = %16.2f\n",vtcw2);
    printf("slope, intercept = %g, %g\n",slope,intercept);
*/

    //------------------------------//
    // Write the output file header //
    //------------------------------//

    #define CR 10 // carriage return
    #define LF 13 // line feed

    char names[20][33] = { "num_header_records            = ",
                           "LongName                      = ",
                           "ShortName                     = ",
                           "producer_agency               = ",
                           "producer_institution          = ",
                           "project_id                    = ",
                           "data_format_type              = ",
                           "GranulePointer                = ",
                           "InputPointer                  = ",
                           "sis_id                        = ",
                           "build_id                      = ",
                           "RangeBeginningDate            = ",
                           "RangeBeginningTime            = ",
                           "RangeEndingDate               = ",
                           "RangeEndingTime               = ",
                           "ProductionDateTime            = ",
                           "num_data_records              = ",
                           "data_record_length            = ",
                           "spare_metadata_element        = ",
                           "spare_metadata_element        = "};

    char values[20][46] = { "20                                           ",
                            "QuikSCAT Time Conversion Data                ",
                            "QSCATTCD                                     ",
                            "NASA                                         ",
                            "JPL                                          ",
                            "QuikSCAT                                     ",
                            "ASCII                                        ",
                            "QS_SCLK_UTC                                  ",
                            "Scott                                        ",
                            "686-644-17/1998-06-08                        ",
                            "2.1.0/1998-06-09                             ",
                            "1998-250                                     ",
                            "03:30:00.000                                 ",
                            "1998-270                                     ",
                            "11:06:02.451                                 ",
                            "1998-257T18:18:54.000                        ",
                            "1                                            ",
                            "100                                          ",
                            "                                             ",
                            "                                             "};
    // Overwrite range specs with data from l1ags_file
    (void)memcpy(values[11],utc1,8);
    (void)memcpy(values[12],utc1+9,12);
    (void)memcpy(values[13],utc2,8);
    (void)memcpy(values[14],utc2+9,12);
    (void)memcpy(values[15],utc2,21);

    for (int i=0; i < 20; i++)
    {
      fprintf(output_stream,"%s%s;%c\n",names[i],values[i],LF);
    }

    //---------------------//
    // Write the data line //
    //---------------------//

    fprintf(output_stream,
            "%16f %21s %14.10f %15.3f HK2_ssr_980907_0840mdt.bin;%c\n",
            vtcw1,utc1,slope,intercept,LF);

    //----------------------//
    // close files and exit //
    //----------------------//

    l1a.Close();
    return(0);
}
