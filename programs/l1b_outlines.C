//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		l1b_outlines
//
// SYNOPSIS
//		l1b_outlines <l1b_file> <Otln_file>
//
// DESCRIPTION
//		Reads in a Level 1B file and writes out a Binary Vector
//		Graphics (Otln) file containing the cell outlines
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//		<l1b_file>		The Level 1B input file.
//		<Otln_file>		The Otln output file.
//
// EXAMPLES
//		An example of a command line is:
//			% l1b_outlines l1b.dat l1b.otln
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
#include "L1B.h"
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

const char* usage_array[] = { "<l1b_file>", "<Otln_file>", 0};

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
	const char* l1b_file = argv[clidx++];
	const char* otln_file = argv[clidx++];

	//------------------------//
	// open the Level 1B file //
	//------------------------//

	L1B l1b;
	if (! l1b.OpenForReading(l1b_file))
	{
		fprintf(stderr, "%s: error opening Level 1B file %s\n", command,
			l1b_file);
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

	while (l1b.ReadDataRec())
	{
		MeasSpotList* msl = &(l1b.frame.spotList);
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
	l1b.CloseInputFile();

	return (0);
}
