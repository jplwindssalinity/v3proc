//==============================================================//
// Copyright (C) 1998-2002, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    histo
//
// SYNOPSIS
//    histo [ -cns ] <input_file> <min> <max> <step> <output_file>
//
// DESCRIPTION
//    Generates a histogram based on the first value in each row.  The
//    values outside of the specified range are dropped (not clipped).
//    The program will calculate an appropriate step size based on the
//    range and the approximate step size given.
//
// OPTIONS
//    The following options are supported:
//      [ -c ]  Also calculate a cumulative histogram.
//      [ -n ]  Output numbers, i.e. don't scale.
//      [ -s ]  Use a stair-step to plot the histogram. This is good
//                for very course histogram display.
//
// OPERANDS
//    The following operands are supported:
//      <input_file>   The input file name.
//      <min>          The minimum value to histogram.
//      <max>          The minimum value to histogram.
//      <step>         The approximate step size.
//      <output_file>  The output file name.
//
// EXAMPLES
//    An example of a command line is:
//      % histo in.dat 0.0 5.0 0.01 out.dat
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
#include <string.h>
#include "Misc.h"

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "cns"

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

const char* usage_array[] = { "[ -cns ]", "<input_file>", "<min>", "<max>",
    "<step>", "<output_file>", 0 };

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

    int opt_cumulative = 0;
    int opt_numbers = 0;
    int opt_step = 0;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);
//  extern char* optarg;
    extern int optind;

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'c':
            opt_cumulative = 1;
            break;
        case 'n':
            opt_numbers = 1;
            break;
        case 's':
            opt_step = 1;
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    if (argc != optind + 5)
        usage(command, usage_array, 1);

    char* input_file = argv[optind++];
    float min = atof(argv[optind++]);
    float max = atof(argv[optind++]);
    float step = atof(argv[optind++]);
    char* output_file = argv[optind++];

    //----------------------//
    // create needed arrays //
    //----------------------//

    int bins = (int)((max - min) / step + 0.5);
    step = (max - min) / (float)bins;
    unsigned int* count = new unsigned int[bins];
    for (int i = 0; i < bins; i++)
        count[i] = 0;

    //---------------------//
    // open the input file //
    //---------------------//

    FILE* ifp = fopen(input_file, "r");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening input file %s\n", command,
            input_file);
        exit(1);
    }

    //---------------------//
    // read and accumulate //
    //---------------------//

    char line[1024];
    do
    {
        if (fgets(line, 1024, ifp) == NULL)
            break;

        float x;
        if (sscanf(line, " %g", &x) != 1)
        {
            fprintf(stderr, "%s: error parsing line.  Continuing...\n",
                command);
            continue;
        }

        int index = (int)((x - min) / step + 0.5);
        if (index < 0 || index >= bins)
            continue;

        count[index]++;
    } while (1);

    //----------------------//
    // close the input file //
    //----------------------//

    fclose(ifp);

    //--------//
    // output //
    //--------//

    float scale = 1.0;
    double sum_scale = 1.0;
    if (! opt_numbers) {
        unsigned long total_count = 0;
        for (int i = 0; i < bins; i++)
        {
            total_count += count[i];
        }
        scale = 1.0 / (step * float(total_count));
        sum_scale = 1.0 / float(total_count);
    }

    FILE* ofp = fopen(output_file, "w");
    if (ifp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            output_file);
        exit(1);
    }

    double sum = 0.0;
    float half_step = step / 2.0;
    for (int i = 0; i < bins; i++)
    {
        if (opt_cumulative && opt_step)
        {
            sum += count[i] * sum_scale;
            fprintf(ofp, "%g %g %g\n", i * step + min - half_step,
                count[i] * scale, sum);
            fprintf(ofp, "%g %g %g\n", i * step + min + half_step,
                count[i] * scale, sum);
        }
        else if (opt_cumulative && ! opt_step)
        {
            sum += count[i] * sum_scale;
            fprintf(ofp, "%g %g %g\n", i * step + min, count[i] * scale, sum);
        }
        else if (! opt_cumulative && opt_step)
        {
            fprintf(ofp, "%g %g\n", i * step + min - half_step,
                count[i] * scale);
            fprintf(ofp, "%g %g\n", i * step + min + half_step,
                count[i] * scale);
        }
        else if (! opt_cumulative && ! opt_step)
        {
            fprintf(ofp, "%g %g\n", i * step + min, count[i] * scale);
        }
    }

    fclose(ofp);

    delete [] count;
    return (0);
}
