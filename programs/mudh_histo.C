//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    mudh_histo
//
// SYNOPSIS
//    mudh_histo <start_rev> <end_rev> <output_base>
//
// DESCRIPTION
//    Generates four single-parameter historgrams
//
// OPTIONS
//
// OPERANDS
//    <start_rev>    Duh.
//    <end_rev>      Duh again.
//    <output_base>  The output base.
//
// EXAMPLES
//    An example of a command line is:
//      % mudh_table -m 15 1500 1600 rv1500-1600.15min
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

#define OPTSTRING    ""
#define AT_WIDTH     1624
#define CT_WIDTH     76

#define MAX_SHORT  65535

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<start_rev>", "<end_rev>", "<output_base>", 0 };

unsigned long accum[4][65536];

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

    if (argc < optind + 3)
        usage(command, usage_array, 1);

    int start_rev = atoi(argv[optind++]);
    int end_rev = atoi(argv[optind++]);
    const char* output_base = argv[optind++];

    //--------------------//
    // process rev by rev //
    //--------------------//

    unsigned short nbd_array[AT_WIDTH][CT_WIDTH];
    unsigned short spd_array[AT_WIDTH][CT_WIDTH];
    unsigned short dir_array[AT_WIDTH][CT_WIDTH];
    unsigned short mle_array[AT_WIDTH][CT_WIDTH];

    for (int rev = start_rev; rev <= end_rev; rev++)
    {
        //----------------//
        // read mudh file //
        //----------------//

        char mudh_file[1024];
        sprintf(mudh_file, "%d.mudh", rev);
        FILE* ifp = fopen(mudh_file, "r");
        if (ifp == NULL)
        {
            fprintf(stderr, "%s: error opening MUDH file %s\n", command,
                mudh_file);
            fprintf(stderr, "%s: continuing...\n", command);
            continue;
        }
        fread(nbd_array, sizeof(short), CT_WIDTH * AT_WIDTH, ifp);
        fread(spd_array, sizeof(short), CT_WIDTH * AT_WIDTH, ifp);
        fread(dir_array, sizeof(short), CT_WIDTH * AT_WIDTH, ifp);
        fread(mle_array, sizeof(short), CT_WIDTH * AT_WIDTH, ifp);
        fclose(ifp);

        //------------//
        // accumulate //
        //------------//

        for (int ati = 0; ati < AT_WIDTH; ati++)
        {
            for (int cti = 0; cti < CT_WIDTH; cti++)
            {
                if (nbd_array[ati][cti] == MAX_SHORT ||
                    spd_array[ati][cti] == MAX_SHORT ||
                    dir_array[ati][cti] == MAX_SHORT ||
                    mle_array[ati][cti] == MAX_SHORT)
                {
                    continue;
                }

                accum[0][nbd_array[ati][cti]]++;
                accum[1][spd_array[ati][cti]]++;
                accum[2][dir_array[ati][cti]]++;
                accum[3][mle_array[ati][cti]]++;
            }
        }
    }

    //------------------//
    // write histograms //
    //------------------//

    const char* param_map[] = { "nbd", "spd", "dir", "mle" };
    double scale[4] = { 0.001, 0.01, 0.01, 0.001 };
    double bias[4] = { -10.0, 0.0, 0.0, -30.0 };
    for (int param_idx = 0; param_idx < 4; param_idx++)
    {
        char filename[1024];
        sprintf(filename, "%s.%s.histo", output_base, param_map[param_idx]);
        FILE* ofp = fopen(filename, "w");
        if (ofp == NULL)
        {
            fprintf(stderr, "%s: error opening output file %s\n", command,
                filename);
            exit(1);
        }

        unsigned long total = 0;
        for (int i = 0; i < 65536; i++)
            total += accum[param_idx][i];

        unsigned long big_pdf = 0;
        for (int i = 0; i < 65536; i++)
        {
            // convert back to real values
            double value = (double)i * scale[param_idx] + bias[param_idx];
            double fraction = (double)accum[param_idx][i] / (double)total;
            big_pdf += accum[param_idx][i];
            double big_pdf_fraction = (double)big_pdf / (double)total;
            if (accum[param_idx][i] > 0)
                fprintf(ofp, "%g %g %g\n", value, fraction, big_pdf_fraction);
        }
        fclose(ofp);
    }

    return (0);
}
