//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    mudh_swath
//
// SYNOPSIS
//    mudh_swath [ -r ] [ -s spd:spd ] [ -d dir:delta ]
//        <output_array_file> <pflag_file...>
//
// DESCRIPTION
//    Generates a 2-d array of percentage of rain classification.
//
// OPTIONS
//    [ -s spd:spd ]   Restrict the output to a speed range.
//    [ -d dir:dir ]   Restrict the output to a direction range.
//    [ -r ]  Make the output relative. Normalized to the percent
//              for that ATI.
//
// OPERANDS
//    <output_array_file>  The output array file.
//    <pflag_file...>     The input pflag files.
//
// EXAMPLES
//    An example of a command line is:
//      % mudh_swath 5000-6000.array [56]*pflag
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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "Misc.h"
#include "Constants.h"
#include "List.h"
#include "mudh.h"

//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "d:s:r"

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

int opt_rel = 0;
int opt_dir = 0;
int opt_spd = 0;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -r ]", "[ -s spd:spd ]", "[ -d dir:dir ]",
    "<output_array_file>", "<pflag_file...>", 0 };

static unsigned long  total_count[AT_WIDTH][CT_WIDTH];
static unsigned long  rain_count[AT_WIDTH][CT_WIDTH];

static float          index_tab[AT_WIDTH][CT_WIDTH];
static unsigned char  flag_tab[AT_WIDTH][CT_WIDTH];

#define DEFAULT_MUDH_DIR  "/export/svt11/hudd/allmudh"

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

    float min_spd = 0.0;
    float max_spd = 0.0;
    float target_dir = 0.0;
    float delta_dir = 0.0;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'r':
            opt_rel = 1;
            break;
        case 's':
            if (sscanf(optarg, " %f:%f", &min_spd, &max_spd) != 2)
            {
                fprintf(stderr, "%s: error parsing speed range %s\n", command,
                    optarg);
                exit(1);
            }
            opt_spd = 1;
            break;
        case 'd':
            if (sscanf(optarg, " %f:%f", &target_dir, &delta_dir) != 2)
            {
                fprintf(stderr, "%s: error parsing directions %s\n", command,
                    optarg);
                exit(1);
            }
            target_dir *= dtr;
            delta_dir *= dtr;
            opt_dir = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc < optind + 2)
        usage(command, usage_array, 1);

    const char* output_array_file = argv[optind++];
    int start_idx = optind;
    int end_idx = argc;

    //--------//
    // arrays //
    //--------//

    unsigned short  nbd_array[AT_WIDTH][CT_WIDTH];
    unsigned short  spd_array[AT_WIDTH][CT_WIDTH];
    unsigned short  dir_array[AT_WIDTH][CT_WIDTH];
    unsigned int array_size = CT_WIDTH * AT_WIDTH;

    //---------------------------//
    // accumulate the flag files //
    //---------------------------//

    for (int file_idx = start_idx; file_idx < end_idx; file_idx++)
    {
        const char* pflag_file = argv[file_idx];
        FILE* ifp = fopen(pflag_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening input file %s\n", command,
                pflag_file);
            exit(1);
        }
        fread(index_tab, sizeof(float), CT_WIDTH * AT_WIDTH, ifp);
        fread(flag_tab,   sizeof(char), CT_WIDTH * AT_WIDTH, ifp);
        fclose(ifp);

        //-------------------------------//
        // read a mudh file if necessary //
        //-------------------------------//

        if (opt_spd || opt_dir)
        {
            char* ptr = strrchr(pflag_file, '.');
            ptr -= 5;
            char rev[5];
            strncpy(rev, ptr, 5);

            char filename[1024];
            sprintf(filename, "%s/%s.mudh", DEFAULT_MUDH_DIR, rev);
            ifp = fopen(filename, "r");
            if (ifp == NULL)
            {
                fprintf(stderr, "%s: error opening MUDH file %s\n", command,
                    filename);
                continue;
            }

            fread(nbd_array, sizeof(short), array_size, ifp);
            fread(spd_array, sizeof(short), array_size, ifp);
            fread(dir_array, sizeof(short), array_size, ifp);
//            fread(mle_array, sizeof(short), array_size, ifp);
//            fread(lon_array, sizeof(short), array_size, ifp);
//            fread(lat_array, sizeof(short), array_size, ifp);
            fclose(ifp);
        }

        //-------//
        // count //
        //-------//

        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                //----------------------------------------//
                // restrict to speed and direction ranges //
                //----------------------------------------//

                if (opt_spd)
                {
                    float spd = (float)spd_array[ati][cti] * 0.01;
                    if (spd < min_spd || spd > max_spd)
                        continue;
                }
                if (opt_dir)
                {
                    float dir = dtr * (float)dir_array[ati][cti] * 0.01;
                    if (ANGDIF(dir, target_dir) > delta_dir)
                        continue;
                }

                switch(flag_tab[ati][cti])
                {
                case 0:    // inner no rain
                case 3:    // outer no rain
                    total_count[ati][cti]++;
                    break;
                case 1:    // inner rain
                case 4:    // outer rain
                    rain_count[ati][cti]++;
                    total_count[ati][cti]++;
                    break;
                default:
                    break;
                }
            }
        }
    }

    float array[CT_WIDTH][AT_WIDTH];
    for (int ati = 0; ati < AT_WIDTH; ati++)
    {
        unsigned long ati_rain = 0;
        unsigned long ati_total = 0;
        if (opt_rel)
        {
            // calculate the average percent
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                ati_rain += rain_count[ati][cti];
                ati_total += total_count[ati][cti];
            }
        }
        float avg = (float)ati_rain / (float)ati_total;
        for (int cti = 0; cti < CT_WIDTH; cti++)
        {
            array[cti][ati] = 0.0;
            if (total_count[ati][cti] > 10)
            {
                array[cti][ati] = (float)rain_count[ati][cti] /
                    (float)total_count[ati][cti];
                if (opt_rel && ati_total > 0)
                {
                    array[cti][ati] -= avg;
                }
            }
        }
    }

    //-----------------//
    // write the array //
    //-----------------//

    FILE* ofp = fopen(output_array_file, "w");
    if (ofp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            output_array_file);
        exit(1);
    }
    int x_size = CT_WIDTH;
    int y_size = AT_WIDTH;
    fwrite(&x_size, sizeof(int), 1, ofp);
    fwrite(&y_size, sizeof(int), 1, ofp);
    if (fwrite(array, sizeof(float), AT_WIDTH * CT_WIDTH, ofp) !=
        AT_WIDTH * CT_WIDTH)
    {
        fprintf(stderr, "%s: error writing output file %s\n", command,
            output_array_file);
        exit(1);
    }
    fclose(ofp);

    return (0);
}
