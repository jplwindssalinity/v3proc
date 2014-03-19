//==============================================================//
// Copyright (C) 1998-2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    dtc_format
//
// SYNOPSIS
//    dtc_format [ -b beam ] [ -f format ] [ -i input_file ]
//      [ -o output_file ]
//
// DESCRIPTION
//    Converts DTC files among various formats.  Options can be
//      put wherever necessary and are applied in order.
//
// OPTIONS
//    [ -b beam ]         Specifies the beam number (1 or 2).  This
//                          option is only needed when converting to
//                          or from the ground system format which puts
//                          both beams in one file.
//    [ -f format ]       Indicates the format of the DTC file.  Can be
//                          b (binary), g (ground system), h (hex), or
//                          o (old binary).
//    [ -i input_file ]   Specifies an input file name.
//    [ -o output_file ]  Specifies an output file name.
//
// OPERANDS
//    None.
//
// EXAMPLES
//    Examples of command lines are:
//
//      # convert a hex file to a binary file
//      % dtc_format -f h -i dtc.1.hex -f b -o dtc.1.bin
//
//      # convert two binary files to a ground system file
//      % dtc_format -f b -b 1 -i dtc.1.bin -b 2 -i dtc.2.bin -o dtc.gs
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
//    James N. Huddleston (hudd@casket.jpl.nasa.gov)
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
#include <unistd.h>
#include "Misc.h"
#include "Tracking.h"

//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING       "b:f:i:o:"
#define MAX_BEAM_INDEX  2

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int read_it(int beam_idx, const char format, const char* input_file,
        int got_it[2], DopplerTracker* dtc_0, DopplerTracker* dtc_1);
int write_it(int beam_idx, const char format, const char* output_file,
        int got_it[2], DopplerTracker* dtc_0, DopplerTracker* dtc_1);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -b beam ]", "[ -f format ]",
    "[ -i input_file ]", "[ -o output_file ]", "(format can be b,g,h,o)", 0 };

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

    int got_it[2] = {0, 0};
    int beam_idx = 0;
    char format = 'b';
    const char* input_file = NULL;
    const char* output_file = NULL;
    DopplerTracker dtc_0, dtc_1;

    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);
    if (argc == 1)
    {
        usage(command, usage_array, 1);
        exit(1);
    }

    extern char *optarg;
    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'b':
            beam_idx = atoi(optarg) - 1;
            if (beam_idx < 0 || beam_idx > 1)
            {
                fprintf(stderr, "%s: beam must be 1 or 2\n", command);
                exit(1);
            }
            break;
        case 'f':
            format = *optarg;
            break;
        case 'i':
            input_file = optarg;
            if (! read_it(beam_idx, format, input_file, got_it, &dtc_0,
                &dtc_1))
            {
                fprintf(stderr, "%s: error reading DTC file %s\n", command,
                    input_file);
                exit(1);
            }
            break;
        case 'o':
            output_file = optarg;
            if (! write_it(beam_idx, format, output_file, got_it, &dtc_0,
                &dtc_1))
            {
                fprintf(stderr, "%s: error writing DTC file %s\n", command,
                    output_file);
                exit(1);
            }
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

    return(0);
}

//---------//
// read_it //
//---------//

int
read_it(
    int              beam_idx,
    const char       format,
    const char*      input_file,
    int              got_it[2],
    DopplerTracker*  dtc_0,
    DopplerTracker*  dtc_1)
{
    //-------------------------//
    // select a DopplerTracker //
    //-------------------------//

    DopplerTracker* dtc_use;
    switch (beam_idx)
    {
    case 0:
        dtc_use = dtc_0;
        break;
    case 1:
        dtc_use = dtc_1;
        break;
    default:
        return(0);
        break;
    }

    //----------------------//
    // read based on format //
    //----------------------//

    switch (format)
    {
    case 'b':
        if (! dtc_use->ReadBinary(input_file))
        {
            fprintf(stderr, "Error reading binary DTC file %s\n", input_file);
            exit(1);
        }
        got_it[beam_idx] = 1;
        break;
    case 'g':
        if (! dtc_0->ReadGS(input_file, dtc_1))
        {
            fprintf(stderr, "Error reading GS DTC file %s\n", input_file);
            exit(1);
        }
        got_it[0] = 1;
        got_it[1] = 1;
        break;
    case 'h':
        if (! dtc_use->ReadHex(input_file))
        {
            fprintf(stderr, "Error reading hex DTC file %s\n", input_file);
            exit(1);
        }
        got_it[beam_idx] = 1;
        break;
    case 'o':
        if (! dtc_use->ReadOldBinary(input_file))
        {
            fprintf(stderr, "Error reading old binary DTC file %s\n",
                input_file);
            exit(1);
        }
        got_it[beam_idx] = 1;
        break;
    }

    return(1);
}

//----------//
// write_it //
//----------//

int
write_it(
    int              beam_idx,
    const char       format,
    const char*      output_file,
    int              got_it[2],
    DopplerTracker*  dtc_0,
    DopplerTracker*  dtc_1)
{
    //-------------------------//
    // select a DopplerTracker //
    //-------------------------//

    DopplerTracker* dtc_use;
    switch (beam_idx)
    {
    case 0:
        dtc_use = dtc_0;
        break;
    case 1:
        dtc_use = dtc_1;
        break;
    default:
        return(0);
        break;
    }
    if (! got_it[beam_idx])
        return(0);

    //-----------------------//
    // write based on format //
    //-----------------------//

    switch (format)
    {
    case 'b':
        if (! dtc_use->WriteBinary(output_file))
        {
            fprintf(stderr, "Error writing binary DTC file %s\n", output_file);
            exit(1);
        }
        break;
    case 'g':
        if (! got_it[0] || ! got_it[1])
        {
            fprintf(stderr, "Both beams are needed to write out GS format\n");
            return(0);
        }
        if (! dtc_0->WriteGS(output_file, dtc_1))
        {
            fprintf(stderr, "Error writing GS DTC file %s\n", output_file);
            exit(1);
        }
        break;
    case 'h':
        if (! dtc_use->WriteHex(output_file))
        {
            fprintf(stderr, "Error writing hex DTC file %s\n", output_file);
            exit(1);
        }
        break;
    case 'o':
        if (! dtc_use->WriteOldBinary(output_file))
        {
            fprintf(stderr, "Error writing old binary DTC file %s\n",
                output_file);
            exit(1);
        }
        break;
    }

    return(1);
}
