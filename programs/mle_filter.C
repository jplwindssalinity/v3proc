//==============================================================//
// Copyright (C) 1997-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    mle_filter
//
// SYNOPSIS
//    mle_filter <sim_config_file>
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
//      % mle_filter sws1b.cfg
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
#include "mudh.h"

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

#define DIF_DIM           37
#define RSPD_DIM          8
#define MAX_SPD           20.0
#define RANK_DIM          4
#define MIN_COUNT         10
#define WINDOW_SIZE       7

#define CT_WIDTH  78

#define MAX_ANG_DIF  45.0

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

const char* usage_array[] = { "<sim_config_file>", "<min_prob>",
    "<output_l2b_file>", "[hdf_source_flag 1=hdf 0=default]",
    "[hdf_target_flag 1=hdf 0=default]", "[l2b_hdf_file (for updating)]", 0};

float prob_array[AT_WIDTH][CT_WIDTH];
int   idx_array[AT_WIDTH][CT_WIDTH];

extern float g_available_fraction;
extern float g_speed_stopper;

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
    fprintf(stderr, "mle_filter: Starting frame number %d\n",
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
    if (argc != 4 && argc != 7)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* config_file = argv[clidx++];
    float min_prob = atof(argv[clidx++]);
    const char* l2b_output_file = argv[clidx++];

    int hdf_source_flag = 0;
    int hdf_target_flag = 0;
    char* hdf_file = NULL;
    if (argc == 7)
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
        if (! l2b.GetArraysForUpdatingDirthHdf(spd, dir, num_ambigs))
        {
            fprintf(stderr,
                "%s: Failure to create array for updating hdf file\n",
                command);
            exit(1);
        }
    }

    //--------------------------------------------------//
    // calculate the average probability for the window //
    //--------------------------------------------------//

    int half_window = WINDOW_SIZE / 2;

    WindSwath* swath = &(l2b.frame.swath);
    for (int ati = 0; ati < AT_WIDTH; ati++)
    {
        int ati_min = ati - half_window;
        int ati_max = ati + half_window + 1;
        if (ati_min < 0)
            ati_min = 0;
        if (ati_max > AT_WIDTH)
            ati_max = AT_WIDTH;

        for (int cti = 0; cti < CT_WIDTH; cti++)
        {
            int cti_min = cti - half_window;
            int cti_max = cti + half_window + 1;
            if (cti_min < 0)
                cti_min = 0;
            if (cti_max > CT_WIDTH)
                cti_max = CT_WIDTH;

            //----------------------------//
            // calculate the dif indicies //
            //----------------------------//

            WVC* wvc = swath->GetWVC(cti, ati);
            if (wvc == NULL)
                continue;

            wvc->selected = NULL;    // initialize
            float best_avg_prob = 0.0;
            WindVectorPlus* best_vector = NULL;

            for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
                wvp = wvc->ambiguities.GetNext())
            {
                float target_dir = wvp->dir;
                int wvc_count = 0;
                float prob_sum = 0.0;
                for (int i = ati_min; i < ati_max; i++)
                {
                    for (int j = cti_min; j < cti_max; j++)
                    {
                        WVC* other_wvc = swath->GetWVC(j, i);
                        if (other_wvc == NULL)
                            continue;

                        //----------------------------//
                        // find the nearest ambiguity //
                        //----------------------------//

                        WindVectorPlus* nearest =
                            other_wvc->GetNearestToDirection(target_dir);

                        //------------------------------//
                        // get the probability indicies //
                        //------------------------------//

                        int dif_idx[3];
                        for (int i = 0; i < 3; i++)
                            dif_idx[i] = DIF_DIM - 1;

                        WindVectorPlus* other_wvp_1 =
                            other_wvc->ambiguities.GetHead();
                        if (other_wvp_1 == NULL)
                            continue;

                        float wvc_prob_sum = 0.0;
                        float scale = other_wvp_1->obj;
                        float near_prob = 0.0;

                        for (WindVectorPlus* other_wvp =
                            other_wvc->ambiguities.GetHead(); other_wvp;
                            other_wvp = other_wvc->ambiguities.GetNext())
                        {
                            float this_prob = exp((other_wvp->obj - scale)/2.0);

                            if (other_wvp == nearest)
                                near_prob = this_prob;

                            wvc_prob_sum += this_prob;
                        }

                        if (wvc_prob_sum == 0.0)
                            continue;

                        near_prob /= wvc_prob_sum;

                        //--------------------------------------//
                        // accumulate the nearest's probability //
                        //--------------------------------------//

                        //--------------------------//
                        // check direction nearness //
                        //--------------------------//

                        float difference = ANGDIF(nearest->dir, target_dir);
                        // only count it if it is aligned, otherwise it is
                        // like having zero probability (harsh!)
                        if (difference < MAX_ANG_DIF * dtr)
                            prob_sum += near_prob;

                        wvc_count++;
                    }
                }
                float avg_prob = prob_sum / (float)wvc_count;
                if (avg_prob > best_avg_prob)
                {
                    best_avg_prob = avg_prob;
                    best_vector = wvp;
                }
            }
            if (best_avg_prob >= min_prob)
                wvc->selected = best_vector;
        }
    }

    char filename[1024];
    sprintf(filename, "%s.init", l2b_output_file);
    if (! l2b.WriteVctr(filename, 0))
    {
        fprintf(stderr, "%s: error writing vctr file %s\n", command,
            filename);
        exit(1);
    }

    //------------------------//
    // Just Ambiguity Removal //
    //------------------------//

    g_speed_stopper = 4.0;
    g_available_fraction = 0.25;
    swath->MedianFilter(5, 250, 0, 0, 0);

    sprintf(filename, "%s.pass", l2b_output_file);
    if (! l2b.WriteVctr(filename, 0))
    {
        fprintf(stderr, "%s: error writing vctr file %s\n", command,
            filename);
        exit(1);
    }

    g_speed_stopper = 0.0;
    swath->MedianFilter(5, 100, 0, 0, 0);

    if (! hdf_target_flag)
    {
        if (! l2b.WriteHeader() || ! l2b.WriteDataRec())
        {
            fprintf(stderr, "%s: error writing L2B file\n", command);
            exit(1);
        }
    }

    //----------------------//
    // write out vctr files //
    //----------------------//

    int max_rank = swath->GetMaxAmbiguityCount();
    for (int i = 0; i <= max_rank; i++)
    {
        sprintf(filename, "%s.%d", l2b_output_file, i);
        if (! l2b.WriteVctr(filename, i))
        {
            fprintf(stderr, "%s: error writing vctr file %s\n", command,
                filename);
            exit(1);
        }
    }

    swath->SelectNudge();
    sprintf(filename, "%s.nudge", l2b_output_file);
    if (! l2b.WriteVctr(filename, 0))
    {
        fprintf(stderr, "%s: error writing vctr file %s\n", command, filename);
        exit(1);
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
                else
                {
                    int k = num_ambigs[i][j] - 1;
                    spd[i][j*HDF_NUM_AMBIGUITIES+k] = wvc->selected->spd;
                    dir[i][j*HDF_NUM_AMBIGUITIES+k] = wvc->selected->dir;
                }
            }
        }

        //-----------------//
        // Update HDF file //
        //-----------------//

        if (! l2b.frame.swath.UpdateHdf(hdf_file, spd, dir, num_ambigs,
            num_ambigs))
        {
            fprintf(stderr, "%s: Unable to update hdf file\n", command);
            exit(1);
        }
    }

    l2b.Close();
    free_array((void*)spd, 2, atibins, ctibins*HDF_NUM_AMBIGUITIES);
    free_array((void*)dir, 2, atibins, ctibins*HDF_NUM_AMBIGUITIES);
    free_array((void*)num_ambigs, 2, atibins, ctibins);

    return (0);
}
