//==============================================================//
// Copyright (C) 1997-2000, California Institute of Technology. //
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
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "ConfigList.h"
#include "L2A.h"
#include "ConfigSim.h"
#include "L2B.h"
#include "L2AToL2B.h"
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
template class List<WindVectorPlus>;
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

const char* usage_array[] = { "<sim_config_file>", "<output_l2b_file>",
    "[hdf_source_flag 1=hdf 0=default]", "[hdf_target_flag 1=hdf 0=default]",
    "[l2b_hdf_file (for updating)]", 0};

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

    const char* command = no_path(argv[0]);
    if (argc != 3 && argc !=6)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* config_file = argv[clidx++];
    const char* l2b_output_file = argv[clidx++];

    int hdf_source_flag = 0;
    int hdf_target_flag = 0;
    char* hdf_file = NULL;
    if (argc == 6)
    {
        hdf_source_flag = atoi(argv[clidx++]);
        hdf_target_flag = atoi(argv[clidx++]);
        hdf_file = argv[clidx++];
    }

    //------------------------//
    // tell how far you have  //
    // gotten if you receive  //
    // the siguser1 signal    //
    //------------------------//

    sigset(SIGUSR1,&report);

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
    if (hdf_source_flag)
    {
        l2b.SetInputFilename(hdf_file);
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

    if (! hdf_source_flag)
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

    if (! hdf_source_flag)
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

    float** spd = NULL;
    float** dir = NULL;
    int** num_ambigs = NULL;
    int** sel_idx = NULL;

    //-----------------------------//
    // read a level 2B data record //
    //-----------------------------//

    if (hdf_source_flag)
    {
        if (l2b.ReadHDF(hdf_file) == 0)
        {
            fprintf(stderr, "%s: error reading HDF L2B file %s\n", command,
                hdf_file);
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
    else if (! l2b.ReadDataRec())
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
            fprintf(stderr, "%s: unknown status (???)\n", command);
            exit(1);
        }
    }

    //---------//
    // HDF I/O //
    //---------//

    if (hdf_target_flag)
    {
        //--------------------//
        // Read Nudge Vectors //
        //--------------------//

        if (! hdf_source_flag)
        {
            if (! l2b.ReadNudgeVectorsFromHdfL2B(hdf_file))
            {
                fprintf(stderr,
                    "%s: error reading nudge vectors from HDF L2B file %s\n",
                    command, hdf_file);
                exit(1);
            }
        }

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
        sel_idx = (int**) make_array(sizeof(int), 2, atibins, ctibins);
        if (! l2b.GetArraysForUpdatingDirthHdf(spd, dir, num_ambigs))
        {
            fprintf(stderr,
                "%s: Failure to create array for updating hdf file\n",
                command);
            exit(1);
        }
    }

    //------------------------//
    // Just Ambiguity Removal //
    //------------------------//

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

    if (hdf_target_flag)
    {
        //-------------------------------------------//
        // First Update Arrays with selected vectors //
        //-------------------------------------------//

        for (int i = 0; i < atibins; i++)
        {
            for (int j = 0; j < ctibins; j++)
            {
                WVC* wvc = l2b.frame.swath.swath[j][i];
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
                else if (l2a_to_l2b.wrMethod == L2AToL2B::S3 )
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
		   WindVectorPlus* wvp=wvc->ambiguities.GetHead();
		   for(int k=0;k< num_ambigs[i][j];k++){
		     if(wvc->selected==wvp) sel_idx[i][j]=k+1;
		     wvp=wvc->ambiguities.GetNext();
		   }
		}
            }
        }

        //-----------------//
        // Update HDF file //
        //-----------------//

        if (! l2b.frame.swath.UpdateHdf(hdf_file, spd, dir, num_ambigs,
            sel_idx))
        {
            fprintf(stderr, "%s: Unable to update hdf file\n", command);
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
