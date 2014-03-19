//==============================================================//
// Copyright (C) 1997-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    windfield_to_vctr
//
// SYNOPSIS
//    windfield_to_vctr [ -r deg ] [ -s speed ] <type> <windfield_file>
//      <vctr_file>
//
// DESCRIPTION
//    Converts a windfield into a vector file for plotting in IDL.
//    If a resolution is specified, the windfield is interpolated.
//
// OPTIONS
//    [ -r deg ]    Interpolate to the specified resolution.
//    [ -s speed ]  Force all vector to have the specified speed.
//
// OPERANDS
//    The following operands are supported:
//    <type>            The type of windfield.
//    <windfield_file>  The windfield filename.
//    <vctr_file>       The output vector file.
//
// EXAMPLES
//    An example of a command line is:
//      % windfield_to_vctr -r 1.0 ecmwf ecmwf.dat ecmwf.1.vctr
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
// AUTHORS
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
#include <unistd.h>
#include <stdlib.h>
#include "Misc.h"
#include "Wind.h"
#include "Constants.h"
#include "List.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<AngleInterval>;

//-----------//
// CONSTANTS //
//-----------//

#define OPTSTRING  "r:s:"

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

int opt_res = 0;
float resolution = 0.0;

int opt_speed = 0;
float fixed_speed = 0.0;

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -r deg ]", "[ -s speed ]", "<type>",
    "<windfield_file>", "<vctr_file>", 0 };

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

    int c;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1)
    {
        switch(c)
        {
        case 'r':
            opt_res = 1;
            resolution = atof(optarg) * dtr;
            break;
        case 's':
            opt_speed = 1;
            fixed_speed = atof(optarg);
            break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }
 
	if (argc != optind + 3)
		usage(command, usage_array, 1);
	const char* type = argv[optind++];
	const char* windfield_file = argv[optind++];
	const char* vctr_file = argv[optind++];

	//-------------------//
	// read in windfield //
	//-------------------//

	WindField windfield;
	if (! windfield.ReadType(windfield_file, type))
	{
		fprintf(stderr, "%s: error reading windfield file %s (%s)\n", command,
			windfield_file, type);
		exit(1);
	}

    if (opt_speed)
    {
        windfield.FixSpeed(fixed_speed);
    }

	//---------------------//
	// write out vctr file //
	//---------------------//

	if (opt_res)
	{
		WindField new_wf;
		if (! new_wf.NewRes(&windfield, resolution, resolution))
		{
			fprintf(stderr, "%s: error creating new windfield\n", command);
			fprintf(stderr, "    requested resolution = %g\n",resolution);
			exit(1);
		}
		if (! new_wf.WriteVctr(vctr_file))
		{
			fprintf(stderr, "%s: error writing vctr file %s\n", command,
				vctr_file);
			exit(1);
		}
	}
	else
	{
		if (! windfield.WriteVctr(vctr_file))
		{
			fprintf(stderr, "%s: error writing vctr file %s\n", command,
				vctr_file);
			exit(1);
		}
	}

	return (0);
}
