//=========================================================//
// Copyright (C) 2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    mudh_swath
//
// SYNOPSIS
//    mudh_swath [ -r ] <output_array_file> <pflag_file...>
//
// DESCRIPTION
//    Generates a 2-d array of percentage of rain classification.
//
// OPTIONS
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
#include <stdlib.h>
#include "Misc.h"
#include "Misc.h"
#include "List.h"
#include "List.C"
#include "mudh.h"

//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "r"

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

int opt_rel = 0;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -r ]", "<output_array_file>",
    "<pflag_file...>", 0 };

static unsigned long  total_count[AT_WIDTH][CT_WIDTH];
static unsigned long  rain_count[AT_WIDTH][CT_WIDTH];

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
        case 'r':
            opt_rel = 1;
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

        //-------//
        // count //
        //-------//

        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
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
