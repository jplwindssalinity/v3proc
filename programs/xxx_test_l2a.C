//==============================================================//
// Copyright (C) 1999-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    mudh_calc (in disguise)
//
// SYNOPSIS
//    mudh_calc <ins_config_file> <hdf_l2a_file> <hdf_l2b_file>
//        <output_mudh_file>
//
// DESCRIPTION
//    Calculates the following parameters and stores them in an array:
//      NBD, direction_from_alongtrack, retrieved_speed, avg_mle,
//      longitude, latitude.
//
// OPTIONS
//
// OPERANDS
//    <ins_config_file>   The config file (for the GMF).
//    <hdf_l2a_file>      The input HDF L2A file.
//    <hdf_l2b_file>      The input HDF L2B file.
//    <output_mudh_file>  The output file.
//
// EXAMPLES
//    An example of a command line is:
//      % mudh_calc $cfg l2a.203 l2b.203 203.mudh
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
#include "L2AH.h"
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

#define OPTSTRING        ""
#define MAX_S0_PER_ROW   3240
#define UNUSABLE         0x0001
#define NEGATIVE         0x0004

#define UNNORMALIZE_MLE_FLAG  0    // leave the mle alone

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<ins_config_file>", "<hdf_l2a_file>",
    "<hdf_l2b_file>", "<output_mudh_file>", 0 };

unsigned short nbd_array[AT_WIDTH][CT_WIDTH];
unsigned short spd_array[AT_WIDTH][CT_WIDTH];
unsigned short dir_array[AT_WIDTH][CT_WIDTH];
unsigned short mle_array[AT_WIDTH][CT_WIDTH];
unsigned short lon_array[AT_WIDTH][CT_WIDTH];
unsigned short lat_array[AT_WIDTH][CT_WIDTH];

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

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 4)
        usage(command, usage_array, 1);

    const char* ins_config_file = argv[optind++];
    const char* l2a_hdf_file = argv[optind++];
    const char* l2b_hdf_file = argv[optind++];
    const char* output_mudh_file = argv[optind++];

    //--------------------------------//
    // read in simulation config file //
    //--------------------------------//

    ConfigList config_list;
    if (! config_list.Read(ins_config_file))
    {
        fprintf(stderr, "%s: error reading sim config file %s\n",
            command, ins_config_file);
        exit(1);
    }

    //-------------------------------------//
    // read the geophysical model function //
    //-------------------------------------//

    GMF gmf;
    char* gmf_filename = config_list.Get(GMF_FILE_KEYWORD);
    if (gmf_filename == NULL)
    {
        fprintf(stderr, "%s: error determining GMF file\n", command);
        exit(1);
    }

    if (! gmf.ReadOldStyle(gmf_filename))
    {
        fprintf(stderr, "%s: error reading GMF file %s\n", command,
            gmf_filename);
        exit(1);
    }

    //--------------//
    // configure Kp //
    //--------------//

    Kp kp;
    if (! ConfigKp(&kp, &config_list))
    {
        fprintf(stderr, "%s: error configuring Kp\n", command);
        exit(1);
    }

    //-------------------//
    // read the l2b file //
    //-------------------//

    L2B l2b;
    l2b.SetInputFilename(l2b_hdf_file);
    if (! l2b.ReadHDF(UNNORMALIZE_MLE_FLAG))
    {
        fprintf(stderr, "%s: error reading L2B file %s\n", command,
            l2b_hdf_file);
        exit(1);
    }
    WindSwath* swath = &(l2b.frame.swath);

    //------------------//
    // set the l2a file //
    //------------------//

    L2AH l2ah;
    if (! l2ah.OpenForReading(l2a_hdf_file))
    {
        fprintf(stderr, "%s: error opening L2A HDF file %s\n", command,
            l2a_hdf_file);
        exit(1);
    }

//    int l2a_length = l2a_file.GetDataLength();

    //------------------//
    // open output file //
    //------------------//

    FILE* ofp = fopen(output_mudh_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            output_mudh_file);
        exit(1);
    }

    //---------------//
    // for each cell //
    //---------------//

    for (int ati = 0; ati < AT_WIDTH; ati++)
    {
        for (int cti = 0; cti < CT_WIDTH; cti++)
        {
            nbd_array[ati][cti] = MAX_SHORT;
            spd_array[ati][cti] = MAX_SHORT;
            dir_array[ati][cti] = MAX_SHORT;
            mle_array[ati][cti] = MAX_SHORT;
            lon_array[ati][cti] = MAX_SHORT;
            lat_array[ati][cti] = MAX_SHORT;

            MeasList* meas_list = l2ah.GetWVC(cti, ati, L2AH::OCEAN_ONLY);
            if (meas_list == NULL)
                continue;

            if (meas_list->NodeCount() == 0)
                continue;

            LonLat avg_lon_lat = meas_list->AverageLonLat();

            WVC* wvc = swath->GetWVC(cti, ati);
            if (wvc == NULL)
                continue;

            WindVectorPlus* wvp = wvc->ambiguities.GetHead();
            if (wvp == NULL)
                continue;

            float spd = wvp->spd;
            float dir = wvp->dir;
            float mle = wvp->obj;

            //-----------------//
            // set speed array //
            //-----------------//
            // integer speed at 0.01 m/s (0 - 50 m/s)

            int ispd = (int)(spd / 0.01 + 0.5);
            if (ispd < 0) ispd = 0;
            if (ispd > 5000) ispd = 5000;
            spd_array[ati][cti] = (unsigned short)ispd;

            //---------------//
            // set MLE array //
            //---------------//
            // integer MLE at 0.001 (-30 - 0)

            int imle = (int)((mle + 30.0) / 0.001 + 0.5);
            if (imle < 0) imle = 0;
            if (imle > 30000) imle = 30000;
            mle_array[ati][cti] = (unsigned short)imle;

            //------------------------//
            // set lon and lat arrays //
            //------------------------//
            // integer lon at 0.01 degree resolution (0 - 360 deg)
            // integer lat at 0.01 degree resolution (-90 - 90 deg)

            double lon = wvc->lonLat.longitude * rtd;
            double lat = wvc->lonLat.latitude * rtd;

            int ilon = (int)(lon * 100.0 + 0.5);
            lon_array[ati][cti] = (unsigned short)ilon;
            int ilat = (int)((lat + 90.0) * 100.0 + 0.5);
            lat_array[ati][cti] = (unsigned short)ilat;

            //--------------//
            // clear arrays //
            //--------------//

            double norm_dif_sum[2];        // beam
            double x_outer_comp_sum[2];    // fore/aft
            double y_outer_comp_sum[2];    // fore/aft
            unsigned int count[2][2];      // beam, fore/aft
            for (int i = 0; i < 2; i++)
            {
                norm_dif_sum[i] = 0.0;
                x_outer_comp_sum[i] = 0.0;
                y_outer_comp_sum[i] = 0.0;
                for (int j = 0; j < 2; j++)
                {
                    count[i][j] = 0;
                }
            }

            //-----------//
            // calculate //
            //-----------//

            for (Meas* meas = meas_list->GetHead(); meas;
                meas = meas_list->GetNext())
            {
                float chi = wvp->dir - meas->eastAzimuth + pi;
                float value;
                gmf.GetInterpolatedValue(meas->measType, meas->incidenceAngle,
                    spd, chi, &value);

                // eliminate very small true sigma-0's
                if (value < 1E-6)
                    continue;

                float kpm = 0.16;
                float use_alpha = (1.0 + kpm*kpm) * meas->A - 1.0;
                float var = (use_alpha * value + meas->B) * value + meas->C;
                float std_dev = sqrt(var);

                float norm_dif = (meas->value - value) / std_dev;
                norm_dif_sum[meas->beamIdx] += norm_dif;

                // hack out fore/aft index
                int foreaft_idx = (int)(meas->scanAngle + 0.5);
                if (foreaft_idx != 0)
                    foreaft_idx = 1;

                if (meas->beamIdx == 1)    // outer beam
                {
                    x_outer_comp_sum[foreaft_idx] += cos(meas->eastAzimuth);
                    y_outer_comp_sum[foreaft_idx] += sin(meas->eastAzimuth);
                }
                count[meas->beamIdx][foreaft_idx]++;
            }
//          wvc.FreeContents();

            //---------------------//
            // set direction array //
            //---------------------//

            // use outer beam (index=1) for direction
            // need both fore and aft measurements
            if (count[1][0] > 0 && count[1][1] > 0)
            {
                // calculate target angle
                for (int i = 0; i < 2; i++)    // fore/aft
                {
                    x_outer_comp_sum[i] /= (double)count[1][i];
                    y_outer_comp_sum[i] /= (double)count[1][i];
                }
/*
                float target_angle =
                    atan2(y_outer_comp_sum[0] + y_outer_comp_sum[1],
                    x_outer_comp_sum[0] + x_outer_comp_sum[1]);
*/
                float target_angle =
                    (atan2(y_outer_comp_sum[0], x_outer_comp_sum[0]) +
                     atan2(y_outer_comp_sum[1], x_outer_comp_sum[1])) / 2.0;

                float dir_val = ANGDIF(target_angle, dir);

                // make it with respect to along track angle
                if (dir_val > pi_over_two)
                    dir_val = dir_val - pi_over_two;
                else
                    dir_val = pi_over_two - dir_val;
                dir_val *= rtd;

                // integer dir at 0.01 deg (0 - 90 deg)
                int idir = (int)(dir_val / 0.01 + 0.5);
                if (idir < 0) idir = 0;
                if (idir > 9000) idir = 9000;
                dir_array[ati][cti] = (unsigned short)idir;
            }

            //---------------//
            // set NBD array //
            //---------------//

            // requires fore and aft looks from both beams
            // why?  i don't know.  better to be conservative
            if (count[0][0] > 0 && count[0][1] > 0 &&
                count[1][0] > 0 && count[1][1] > 0)
            {
                unsigned int beam_count[2];    // beam
                beam_count[0] = count[0][0] + count[0][1];    // inner beam
                beam_count[1] = count[1][0] + count[1][1];    // outer beam
                float mean_norm_dif[2];    // beam
                for (int beam_idx = 0; beam_idx < 2; beam_idx++)
                {
                    mean_norm_dif[beam_idx] = norm_dif_sum[beam_idx] /
                        (double)beam_count[beam_idx];
                }
                float mean_dif = mean_norm_dif[0] - mean_norm_dif[1];

                // calculate the expected std of the mean ratio difference
                double sub_std = sqrt(1.0 / (double)beam_count[0] +
                    1.0 / (double)beam_count[1]);
                double nbd = mean_dif / sub_std;

                // integer NBD at 0.01 m/s (-10 - 10)
                int inbd = (int)((nbd + 10.0) / 0.001 + 0.5);
                if (inbd < 0) inbd = 0;
                if (inbd > 20000) inbd = 20000;
                nbd_array[ati][cti] = (unsigned short)inbd;
            }
            meas_list->FreeContents();
        }
    }

    //--------------//
    // write arrays //
    //--------------//

    fwrite(nbd_array, sizeof(short), CT_WIDTH * AT_WIDTH, ofp);
    fwrite(spd_array, sizeof(short), CT_WIDTH * AT_WIDTH, ofp);
    fwrite(dir_array, sizeof(short), CT_WIDTH * AT_WIDTH, ofp);
    fwrite(mle_array, sizeof(short), CT_WIDTH * AT_WIDTH, ofp);
    fwrite(lon_array, sizeof(short), CT_WIDTH * AT_WIDTH, ofp);
    fwrite(lat_array, sizeof(short), CT_WIDTH * AT_WIDTH, ofp);
    fclose(ofp);

    return (0);
}
