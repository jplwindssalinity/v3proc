//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l2b_to_flower
//
// SYNOPSIS
//		l2b_to_flower <l2b_file> [ flower_file ]
//
// DESCRIPTION
//		Converts a Level 2B file into a flower file showing the
//		objective function as a polar plot.  The output filename is
//		created by adding the extension ".flwr" to the l2b_filename.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operands are supported:
//		<l2b_file>			The input Level 2B wind field
//		[ flower_file ]		The output flower filename
//
// EXAMPLES
//		An example of a command line is:
//			% l2b_to_flower l2b.dat
//
// ENVIRONMENT
//		Not environment dependent.
//
// EXIT STATUS
//		The following exit values are returned:
//		0	Program executed successfully
//		>0	Program had an error
//
// NOTES
//		None.
//
// AUTHOR
//		James N. Huddleston
//		hudd@acid.jpl.nasa.gov
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
#include "L2B.h"
#include "List.h"
#include "List.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<EarthPosition>;
template class List<WindVectorPlus>;

//-----------//
// CONSTANTS //
//-----------//

#define EXTENSION	"flwr"

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

const char* usage_array[] = { "<l2b_file>", "[ flower_file ]", 0};

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
	int		argc,
	char*	argv[])
{
	//------------------------//
	// parse the command line //
	//------------------------//

	const char* command = no_path(argv[0]);
	if (argc < 2 || argc > 3)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* l2b_file = argv[clidx++];
	char output_file[1024];
	if (argc == 3)
		sprintf(output_file, "%s", argv[clidx++]);
	else
		sprintf(output_file, "%s.%s", l2b_file, EXTENSION);

	//------------------//
	// read in l2b file //
	//------------------//

	L2B l2b;
	if (! l2b.OpenForReading(l2b_file))
	{
		fprintf(stderr, "%s: error opening L2B file %s\n", command, l2b_file);
		exit(1);
	}
	if (! l2b.ReadHeader())
	{
		fprintf(stderr, "%s: error reading L2B header from file %s\n",
			command, l2b_file);
		exit(1);
	}

	if (! l2b.ReadDataRec())
	{
		fprintf(stderr, "%s: error reading L2B data record from file %s\n",
			command, l2b_file);
		exit(1);
	}

	//------------------------//
	// write out flower files //
	//------------------------//

	if (! l2b.frame.swath.WriteFlower(output_file))
	{
		fprintf(stderr, "%s: error writing flower file %s\n", command,
			output_file);
		exit(1);
	}

	return (0);
}
