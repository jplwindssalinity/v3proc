//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l15_outlines
//
// SYNOPSIS
//		l15_outlines <l15_file> <Otln_file>
//
// DESCRIPTION
//		Reads in a Level 1.5 file and writes out a Binary Vector
//		Graphics (Otln) file containing the cell outlines
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//		<l15_file>		The Level 1.5 input file.
//		<Otln_file>		The Otln output file.
//
// EXAMPLES
//		An example of a command line is:
//			% l15_outlines l15.dat l15.otln
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
#include "L15.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"

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

const char* usage_array[] = { "<l15_file>", "<Otln_file>", 0};

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
	const char* l15_file = argv[clidx++];
	const char* otln_file = argv[clidx++];

	//-------------------------//
	// open the Level 1.5 file //
	//-------------------------//

	L15 l15;
	if (! l15.OpenForReading(l15_file))
	{
		fprintf(stderr, "%s: error opening Level 1.5 file %s\n", command,
			l15_file);
		exit(1);
	}

	//----------------//
	// open Otln file //
	//----------------//

	FILE* output_fp = fopen(otln_file, "w");
	if (output_fp == NULL)
	{
		fprintf(stderr, "%s: error opening Otln file %s\n", command, otln_file);
		exit(1);
	}

	char* hdr = OTLN_HEADER;
	if (fwrite((void *)hdr, 4, 1, output_fp) != 1)
	{
		fprintf(stderr, "%s: error writing header to output file %s\n",
			command, otln_file);
		exit(1);
	}

	//----------------//
	// loop and write //
	//----------------//

	while (l15.ReadDataRec())
	{
		MeasSpotList* msl = &(l15.frame.spotList);
		for (MeasSpot* ms = msl->GetHead(); ms; ms = msl->GetNext())
		{
			for (Meas* m = ms->GetHead(); m; m = ms->GetNext())
			{
double alt, lon, lat;
m->centroid.GetAltLonGCLat(&alt, &lon, &lat);
printf("%g %g\n", lon, lat);
				if (! m->outline.WriteOtln(output_fp))
				{
					fprintf(stderr, "%s: error writing Otln data to file %s\n",
						command, otln_file);
					exit(1);
				}
			}
		}
	}

	//-----------------//
	// close the files //
	//-----------------//

	fclose(output_fp);
	l15.Close();

	return (0);
}
