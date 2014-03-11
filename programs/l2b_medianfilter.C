//==============================================================//
// Copyright (C) 1997-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    l2b_medianfilter
//
// SYNOPSIS
//    l2b_medianfilter <sim_config_file>
//
// DESCRIPTION
//    Simulates the SeaWinds 1b ground processing of Level 2A to
//    Level 2B data.  This program retrieves wind from measurements.
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operand is supported:
//    <sim_config_file>  The sim_config_file needed listing
//                       all input parameters, input files, and
//                       output files.
//
// EXAMPLES
//    An example of a command line is:
//      % l2b_medianfilter sws1b.cfg
//
// ENVIRONMENT
//    Not environment dependent.
//
// EXIT STATUS
//    The following exit values are returned:
//    0   Program executed successfully
//    >0  Program had an error
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
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "List.h"
#include "BufferedList.h"
#include "Misc.h"
#include "ConfigList.h"
#include "L2A.h"
#include "ConfigSim.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Tracking.h"
#include "Array.h"

using std::list;
using std::map; 

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<AngleInterval>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;

//-----------//
// CONSTANTS //
//-----------//

#define HIRES12
#define HDF_NUM_AMBIGUITIES   4

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//


//--------------------//
// Report handler     //
// runs if SIGUSR1 is //
// recieved.          //
//--------------------//

int global_frame_number=0;

void
report(
    int  sig_num)
{
    fprintf(stderr, "l2b_medianfilter: Starting frame number %d\n",
        global_frame_number);
    return;
}

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

    const char* command      = no_path(argv[0]);

    char* config_file        = NULL;
    char* l2b_output_file    = NULL;
    char* l2b_hdf_nudge_file = NULL;
    char* ext_filename       = NULL;

    int nudge_HDF_flag       = 0; // 1 if read nudges from HDF file, 0 if not.
    int nudge_l2b_flag       = 0;
    int read_flags_HDF       = 0;
    
    int ext_file_flag        = 0; // 0 undefined, 1 HDF, 2 retdat
    int ext_source_flag      = 0; // 1 if load data from ext_file, 0 if not
    int ext_target_flag      = 0; // 1 if save data to ext_file, 0 if not
    
    int print_usage_flag     = 0;

    int read_nudge_vectors_RETDAT = 1;
    
    int optind = 1;
    while ( (optind < argc) && (argv[optind][0]=='-') )
      {    
	std::string sw = argv[optind];
	if( sw == "-c" )
	  {
	    ++optind;
	    config_file = argv[optind];
	  }
	else if( sw == "-o" )
	  {
	    ++optind;
	    l2b_output_file = argv[optind];
	  }
	else if( sw == "-nudgeHDF" )
	  {
	    ++optind;
	    l2b_hdf_nudge_file = argv[optind];
	    nudge_HDF_flag = 1;
	    read_nudge_vectors_RETDAT = 0; // Turn off reading nudge vectors from RETDAT
	  }
	else if( sw == "-flagsHDF" )
	  {
	    ++optind;
	    l2b_hdf_nudge_file = argv[optind];
	    read_flags_HDF = 1;
	  }
	else if( sw == "-nudgel2b" )
	  {
	    ++optind;
	    nudge_l2b_flag = 1;
	    read_nudge_vectors_RETDAT = 0; // Turn off reading nudge vectors from RETDAT
	  }
	else if( sw == "-extfile" )
	  {    
	    ++optind;
	    std::string file_type = argv[optind];

	    if(      file_type == "HDF"    ) ext_file_flag = 1;
	    else if( file_type == "RETDAT" ) ext_file_flag = 2;
	    else
	      {
		fprintf(stderr,"Unknown file type specified!\n");
		print_usage_flag = 1;
	      }

	    ext_filename    = argv[++optind];
	    ext_source_flag = atoi(argv[++optind]);
	    ext_target_flag = atoi(argv[++optind]);
	  }
	else
	  print_usage_flag = 1;
	++optind;
      }

    if( config_file == NULL || l2b_output_file == NULL || print_usage_flag )
      {
	fprintf(stderr,"\nUsage: %s -c config_file -o output_l2b_file\n",command);
        fprintf(stderr,"       [-nudgeHDF hdf_l2b_file] [-extfile (HDF||RETDAT)\n");
	fprintf(stderr,"       ext_filename ext_source_flag ext_target_flag]\n\n");
	return (1);
      }
    

    //------------------------//
    // tell how far you have  //
    // gotten if you receive  //
    // the siguser1 signal    //
    //------------------------//

    //sigset(SIGUSR1,&report);

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

    //-------------------------------------//
    // create and configure level products //
    //-------------------------------------//

    L2B l2b;
    if (ext_source_flag)
    {
        l2b.SetInputFilename(ext_filename);
    }
    else if (! ConfigL2B(&l2b, &config_list))
    {
        fprintf(stderr, "%s: error configuring Level 2B Product\n", command);
        exit(1);
    }

    // Overwrite with new name
    l2b.SetOutputFilename(l2b_output_file);

    //------------------------------------//
    // create and configure the converter //
    //------------------------------------//

    L2AToL2B l2a_to_l2b;
    if (! ConfigL2AToL2B(&l2a_to_l2b, &config_list))
    {
        fprintf(stderr, "%s: error configuring L2AToL2B\n", command);
        exit(1);
    }

    //------------//
    // open files //
    //------------//

    if (! ext_source_flag)
    {
        if (! l2b.OpenForReading())
        {
            fprintf(stderr, "%s: error opening L2B file for reading\n",
                command);
            exit(1);
        }
    }
    if (! l2b.OpenForWriting())
    {
        fprintf(stderr, "%s: error opening L2B file %s for writing\n",
            command, l2b_output_file);
        exit(1);
    }

    //---------------------------------//
    // read the header to set up swath //
    //---------------------------------//

    if (! ext_source_flag)
    {
        if (! l2b.ReadHeader())
        {
            fprintf(stderr, "%s: error reading Level 2B header\n", command);
            exit(1);
        }
    }

    //-----------------//
    // conversion loop //
    //-----------------//

    int ctibins = 0;
    int atibins = 0;

    float**      spd = NULL;
    float**      dir = NULL;
    int** num_ambigs = NULL;
    int**    sel_idx = NULL;

    //-----------------------------//
    // read a level 2B data record //
    //-----------------------------//

    if (ext_source_flag)
    {
      if( ext_file_flag == 1 ) // HDF specified
	{
	  if (l2b.ReadPureHdf(ext_filename,1) == 0)  // Use ReadPureHDF, old one broken.
	    {
	      fprintf(stderr, "%s: error reading HDF L2B file %s\n", command,
		      ext_filename);
	      exit(1);
	    }
	  l2b.header.crossTrackResolution = 25.0;
	  l2b.header.alongTrackResolution = 25.0;
	  l2b.header.zeroIndex = 38;

#ifdef HIRES12
	  l2b.header.crossTrackResolution = 12.5;
	  l2b.header.alongTrackResolution = 12.5;
	  l2b.header.zeroIndex = 76;
#endif
	}
      else if( ext_file_flag == 2) // RETDAT specified
	{
	  if( !l2b.ReadRETDAT( ext_filename, read_nudge_vectors_RETDAT ) )
	    {
	      fprintf(stderr, "%s: error reading RETDAT L2B file %s\n", command,
		      ext_filename);
	      exit(1);
	    }
	}
    }
    
    else if ( !l2b.ReadDataRec())
    {
        switch (l2b.GetStatus())
        {
        case L2B::OK:
            fprintf(stderr, "%s: Unexpected EOF Level 2B data\n", command);
            exit(1);
            break;    // end of file
        case L2B::ERROR_READING_FRAME:
            fprintf(stderr, "%s: error reading Level 2B data\n", command);
            exit(1);
            break;
        case L2B::ERROR_UNKNOWN:
            fprintf(stderr, "%s: unknown error reading Level 2B data\n",
                command);
            exit(1);
            break;
        default:
            fprintf(stderr, "%s: unknown status\n", command);
            exit(1);
        }
    }
    
    //-------------------------------------------------------//
    // Read Nudge Vectors & rain/ice/land flags from L2B HDF //
    //-------------------------------------------------------//    

    if( nudge_HDF_flag )
      {
	if (! l2b.ReadNudgeVectorsFromHdfL2B( l2b_hdf_nudge_file, 1 ) ) 
	  {
	    fprintf(stderr,
		    "%s: error reading nudge vectors from HDF L2B file %s\n",
		    command, l2b_hdf_nudge_file);
	    exit(1);
	  }
      }
    
    if( read_flags_HDF ) {
      if( ! l2b.ReadLandIceRainFlagsFromHdfL2B( l2b_hdf_nudge_file, 1, 1 ) ) {
        fprintf( stderr, "%s: error reading land/ice/rain flags from HDF L2B file: %s\n",
          command, l2b_hdf_nudge_file );
        exit(1);
      }
    }
    
    //------------------------//
    // Just Ambiguity Removal //
    //------------------------//

    if( nudge_l2b_flag )
    {
      fprintf(stdout,"Using nudge vector located in l2b file.\n");
      l2b.frame.swath.nudgeVectorsRead = 1;
    }
    
    int retval = l2a_to_l2b.InitFilterAndFlush(&l2b);

    switch (retval)
      {
      case 1:
        break;
      case 2:
        break;
      case 4:
      case 5:
        break;
      case 0:
        fprintf(stderr, "%s: error converting Level 2A to Level 2B\n",
		command);
        exit(1);
        break;
      }

    //---------//
    // HDF I/O //
    //---------//

    if ( ext_target_flag && ext_file_flag == 1 ) 
    {

        //---------------//
        // Create Arrays //
        //---------------//
	
	ctibins = l2b.frame.swath.GetCrossTrackBins();
	atibins = l2b.frame.swath.GetAlongTrackBins();
	spd = (float**)make_array(sizeof(float), 2, atibins,
				  ctibins * HDF_NUM_AMBIGUITIES);
        dir = (float**)make_array(sizeof(float), 2, atibins,
				  ctibins * HDF_NUM_AMBIGUITIES);
	
        num_ambigs = (int**) make_array(sizeof(int), 2, atibins, ctibins);
        sel_idx    = (int**) make_array(sizeof(int), 2, atibins, ctibins);
        if (! l2b.GetArraysForUpdatingDirthHdf( spd, dir, num_ambigs) )
	  {
            fprintf(stderr,
		    "%s: Failure to create array for updating hdf file\n",
		    command);
            exit(1);
	  }

        //-------------------------------------------//
        // First Update Arrays with selected vectors //
        //-------------------------------------------//

        for (int i = 0; i < atibins; i++)
        {
            for (int j = 0; j < ctibins; j++)
            {
                WVC* wvc = l2b.frame.swath.swath[j][i];

		sel_idx[i][j] = 0;
		
                if (! wvc)
                {
                    num_ambigs[i][j] = 0;
                    continue;
                }
                else if (! (wvc->selected))
                {
                    num_ambigs[i][j] = 0;
                    continue;
                }
                else if (l2a_to_l2b.wrMethod == L2AToL2B::S3)
                {
                    num_ambigs[i][j]+=1;
                    if (num_ambigs[i][j] > HDF_NUM_AMBIGUITIES)
                        num_ambigs[i][j] = HDF_NUM_AMBIGUITIES;
                    int k = num_ambigs[i][j] - 1;

                    spd[i][j*HDF_NUM_AMBIGUITIES+k] = wvc->selected->spd;
                    dir[i][j*HDF_NUM_AMBIGUITIES+k] = wvc->selected->dir;
                    sel_idx[i][j]=num_ambigs[i][j];
                }
                else
                {
                    WindVectorPlus* wvp = wvc->ambiguities.GetHead();
                    for (int k = 0; k < num_ambigs[i][j]; k++)
                    {
                        if (wvc->selected == wvp) sel_idx[i][j] = k + 1;
                        wvp = wvc->ambiguities.GetNext();
                    }
                }
            }
        }

        //-----------------//
        // Update HDF file //
        //-----------------//
	
        if (! l2b.frame.swath.UpdateHdf( ext_filename, spd, dir, num_ambigs,
					 sel_idx))
	  {
	    fprintf(stderr, "%s: Unable to update hdf file\n", command);
            exit(1);
	  }
    }
    else if( ext_target_flag && ext_file_flag == 2 )
      {
	if( !l2b.WriteRETDAT( ext_filename ) )
	  {
            fprintf(stderr, "%s: Unable to update RETDAT file\n", command);
            exit(1);
	  }
      }
    
    l2b.Close();

    free_array((void*)spd, 2, atibins, ctibins*HDF_NUM_AMBIGUITIES);
    free_array((void*)dir, 2, atibins, ctibins*HDF_NUM_AMBIGUITIES);
    free_array((void*)num_ambigs, 2, atibins, ctibins);
    free_array((void*)sel_idx, 2, atibins, ctibins);
    
    return (0);
}
