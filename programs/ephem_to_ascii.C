//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		ephem_to_ascii
//
// SYNOPSIS
//		ephem_to_ascii <ephem file> <output_file>
//
// DESCRIPTION
//		Reads in a binary ephem file and writes out the data in ascii form
//
// OPTIONS
//		None.
//
// EXAMPLES
//		An example of a command line is:
//			% ephem_to_ascii ephem.dat ephem.txt
//
// ENVIRONMENT
//		Not environment dependent.
//
// EXIT STATUS
//		The following exit values are returned:
//		1	Program executed successfully
//		>0	Program had an error
//
// NOTES
//		None.
//
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
#include "Ephemeris.h"
#include "Meas.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<EarthPosition>;
template class List<Meas>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<long>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

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

const char* usage_array[] = { "<ephemfile>", "<output_file>", 0};

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
	const char* ephemfile = argv[clidx++];
	const char* output_file = argv[clidx++];

	//---------------------//
	// open the ephem file //
	//---------------------//

	FILE* ephem_fp = fopen(ephemfile,"r");
	if (ephem_fp == NULL)
	{
		fprintf(stderr, "%s: error opening ephem file %s\n", command,
			ephemfile);
		exit(1);
	}

	//------------------//
	// open output file //
	//------------------//

	FILE* output_fp = fopen(output_file, "w");
	if (output_fp == NULL)
	{
		fprintf(stderr, "%s: error opening output file %s\n", command,
			output_file);
		exit(1);
	}

    OrbitState os;

	//----------------//
	// loop and write //
	//----------------//

	while (os.Read(ephem_fp))
	{
        fprintf(output_fp,"%10g %10g %10g %10g %10g %10g %10g\n",os.time,
          os.rsat.Get(0), os.rsat.Get(1),os.rsat.Get(2),
          os.vsat.Get(0),os.vsat.Get(1), os.vsat.Get(2));
	}

	//-----------------//
	// close the files //
	//-----------------//

	fclose(output_fp);
	fclose(ephem_fp);

	return (0);
}
