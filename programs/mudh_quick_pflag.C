//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    mudh_quick_pflag
//
// SYNOPSIS
//    mudh_quick_pflag <mudhtab_file> <mudh_thresh> <mudh_file>
//        <output_pflag_file>
//
// DESCRIPTION
//    Performs MUDH classification and writes out two arrays:
//      A floating point MUDH index array and a byte flag array.
//      The byte flag array has the following meanings:
//      0 = classified as no rain
//      1 = classified as rain
//      2 = unclassifiable
//
// OPTIONS
//
// OPERANDS
//    <mudhtab_file>       The MUDH table.
//    <mudhtab_thresh>     The threshold for rain determination.
//    <mudh_file>          The input MUDH file.
//    <output_pflag_file>  The output pflag file.
//
// EXAMPLES
//    An example of a command line is:
//      % mudh_quick_pflag zzz.mudhtabex 0.015 1550.mudh 1550.pflag
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
#include <stdlib.h>
#include "Misc.h"
#include "mudh.h"

//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  ""

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<mudhtab_file>", "<mudh_thresh>",
    "<mudh_file>", "<output_pflag_file>", 0 };

static double norain_tab[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];
static double rain_tab[NBD_DIM][SPD_DIM][DIR_DIM][MLE_DIM];

static unsigned short nbd_array[AT_WIDTH][CT_WIDTH];
static unsigned short spd_array[AT_WIDTH][CT_WIDTH];
static unsigned short dir_array[AT_WIDTH][CT_WIDTH];
static unsigned short mle_array[AT_WIDTH][CT_WIDTH];
static unsigned short lon_array[AT_WIDTH][CT_WIDTH];
static unsigned short lat_array[AT_WIDTH][CT_WIDTH];

static float          index_tab[AT_WIDTH][CT_WIDTH];
static unsigned char  flag_tab[AT_WIDTH][CT_WIDTH];

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

    const char* mudhtab_file = argv[optind++];
    float mudh_thresh = atof(argv[optind++]);
    const char* mudh_file = argv[optind++];
    const char* output_pflag_file = argv[optind++];

    //--------------//
    // simple calcs //
    //--------------//

    float nbd_spread = NBD_MAX - NBD_MIN;
    int max_inbd = NBD_DIM - 2;    // save room for missing NBD index

    float spd_spread = SPD_MAX - SPD_MIN;
    int max_ispd = SPD_DIM - 1;

    float dir_spread = DIR_MAX - DIR_MIN;
    int max_idir = DIR_DIM - 1;

    float mle_spread = MLE_MAX - MLE_MIN;
    int max_imle = MLE_DIM - 1;

    //-------------------//
    // read mudhtab file //
    //-------------------//

    FILE* mudhtab_ifp = fopen(mudhtab_file, "r");
    if (mudhtab_ifp == NULL)
    {
        fprintf(stderr, "%s: error opening mudhtab file %s\n", command,
            mudhtab_file);
        exit(1);
    }
    unsigned int size = NBD_DIM * SPD_DIM * DIR_DIM * MLE_DIM;
    if (fread(norain_tab, sizeof(double), size, mudhtab_ifp) != size ||
        fread(rain_tab, sizeof(double), size, mudhtab_ifp) != size)
    {
        fprintf(stderr, "%s: error reading mudhtab file %s\n", command,
            mudhtab_file);
        exit(1);
    }
    fclose(mudhtab_ifp);

    //-------------------//
    // read in mudh file //
    //-------------------//

    FILE* ifp = fopen(mudh_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening MUDH file %s\n", command,
            mudh_file);
        exit(1);
    }
    size = CT_WIDTH * AT_WIDTH;
    if (fread(nbd_array, sizeof(short), size, ifp) != size ||
        fread(spd_array, sizeof(short), size, ifp) != size ||
        fread(dir_array, sizeof(short), size, ifp) != size ||
        fread(mle_array, sizeof(short), size, ifp) != size ||
        fread(lon_array, sizeof(short), size, ifp) != size ||
        fread(lat_array, sizeof(short), size, ifp) != size)
    {
        fprintf(stderr, "%s: error reading MUDH file %s\n", command,
           mudh_file);
        exit(1);
    }
    fclose(ifp);

    //----------//
    // classify //
    //----------//

    for (int ati = 0; ati < AT_WIDTH; ati++)
    {
        for (int cti = 0; cti < CT_WIDTH; cti++)
        {
            if (spd_array[ati][cti] == MAX_SHORT ||
                dir_array[ati][cti] == MAX_SHORT ||
                mle_array[ati][cti] == MAX_SHORT ||
                lon_array[ati][cti] == MAX_SHORT ||
                lat_array[ati][cti] == MAX_SHORT)
            {
                index_tab[ati][cti] = -1.0;
                flag_tab[ati][cti] = 2;
                continue;
            }

            // convert back to real values
            double nbd = (double)nbd_array[ati][cti] * 0.001 - 10.0;
            double spd = (double)spd_array[ati][cti] * 0.01;
            double dir = (double)dir_array[ati][cti] * 0.01;
            double mle = (double)mle_array[ati][cti] * 0.001 - 30.0;

            //-----------------------------//
            // convert speed to MUDH index //
            //-----------------------------//

            int ispd = (int)((spd - SPD_MIN) * (float)max_ispd /
                spd_spread + 0.5);
            if (ispd < 0) ispd = 0;
            if (ispd > max_ispd) ispd = max_ispd;

            //---------------------------//
            // convert MLE to MUDH index //
            //---------------------------//

            int imle = (int)((mle - MLE_MIN) * (float)max_imle /
                mle_spread + 0.5);
            if (imle < 0) imle = 0;
            if (imle > max_imle) imle = max_imle;

            //---------------------------//
            // convert NBD to MUDH index //
            //---------------------------//

            int inbd = max_inbd + 1;    // index for "No NBD available"
            if (nbd_array[ati][cti] != MAX_SHORT)
            {
                inbd = (int)((nbd - NBD_MIN) * (float)max_inbd /
                    nbd_spread + 0.5);
                if (inbd < 0) inbd = 0;
                if (inbd > max_inbd) inbd = max_inbd;
            }

            //-------------------------------------------//
            // convert direction parameter to MUDH index //
            //-------------------------------------------//

            int idir = (int)((dir - DIR_MIN) * (float)max_idir /
                dir_spread + 0.5);
            if (idir < 0) idir = 0;
            if (idir > max_idir) idir = max_idir;

            //------------------------------------------------------//
            // look up the value from the rain classification table //
            //------------------------------------------------------//

            float mudh_value = rain_tab[inbd][ispd][idir][imle];
            index_tab[ati][cti] = mudh_value;

            if (mudh_value <= mudh_thresh)
            {
                flag_tab[ati][cti] = 0;
            }
            else
            {
                flag_tab[ati][cti] = 1;
            }
        }
    }

    //------------------//
    // write pflag file //
    //------------------//

    FILE* ofp = fopen(output_pflag_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            output_pflag_file);
        exit(1);
    }
    fwrite(index_tab, sizeof(float), CT_WIDTH * AT_WIDTH, ofp);
    fwrite(flag_tab,   sizeof(char), CT_WIDTH * AT_WIDTH, ofp);
    fclose(ofp);

    return (0);
}
