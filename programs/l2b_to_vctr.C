//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l20_to_bev
//
// SYNOPSIS
//		l20_to_bev <l20_file> <bev_file>
//
// DESCRIPTION
//		Converts a l20 file into multiple bev (binary earth vector)
//		files for plotting in IDL.  Output filenames are created by
//		adding the rank number (0 for selected) to the base name.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operands are supported:
//		<l20_file>		The input l20 wind field
//		<bev_base>		The output bev file basename
//
// EXAMPLES
//		An example of a command line is:
//			% l20_to_bev 96interp.bin output.bev
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
#include "L20.h"
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

const char* usage_array[] = { "<l20_file>", "<bev_base>", 0};

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
	if (argc != 3)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* l20_file = argv[clidx++];
	const char* bev_base = argv[clidx++];

	//------------------//
	// read in l20 file //
	//------------------//

	L20 l20;
	if (! l20.OpenForReading(l20_file))
	{
		fprintf(stderr, "%s: error opening L20 file %s\n", command, l20_file);
		exit(1);
	}
	if (! l20.ReadHeader())
	{
		fprintf(stderr, "%s: error reading L20 header from file %s\n",
			command, l20_file);
		exit(1);
	}

	if (! l20.ReadDataRec())
	{
		fprintf(stderr, "%s: error reading L20 data record from file  %s\n",
			command, l20_file);
		exit(1);
	}

	//---------------------//
	// write out bev files //
	//---------------------//

	for (int i = 0; i < 5; i++)
	{
		char filename[1024];
		sprintf(filename, "%s.%d", bev_base, i);
		if (! l20.WriteBev(filename, i))
		{
			fprintf(stderr, "%s: error writing BEV file %s\n", command,
				filename);
			exit(1);
		}
	}

	return (0);
}
