//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    w25_to_vctr
//
// SYNOPSIS
//    w25_to_vctr <w25_file> <vctr_file>
//
// DESCRIPTION
//    Converts an NSCAT 25 km single wind vector file into a vctr
//    (vector) file for plotting in IDL.
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operands are supported:
//      <w25_file>   The input Level 2B wind field
//      <vctr_file>  The output vctr file name
//
// EXAMPLES
//    An example of a command line is:
//      % w25_to_vctr w25.dat w25.vctr
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
#include "Misc.h"
#include "Wind.h"
#include "List.h"
#include "List.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<AngleInterval>;

//-----------//
// CONSTANTS //
//-----------//

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

const char* usage_array[] = { "<w25_file>", "<vctr_file>", 0};

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
    if (argc != 3)
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* w25_file = argv[clidx++];
    const char* vctr_file = argv[clidx++];

    //------------------//
    // read in w25 file //
    //------------------//

    WindSwath wind_swath;
    if (! wind_swath.ReadNscatSwv25(w25_file))
    {
        fprintf(stderr, "%s: error reading NSCAT file %s\n", command,
            w25_file);
        exit(1);
    }

    //---------------------//
    // write out vctr file //
    //---------------------//

    if (! wind_swath.WriteVctr(vctr_file, 0))
    {
        fprintf(stderr, "%s: error writing vctr file %s\n", command,
            vctr_file);
        exit(1);
    }

    return (0);
}
