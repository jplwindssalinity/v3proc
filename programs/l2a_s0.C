//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l2a_s0
//
// SYNOPSIS
//		l2a_s0 <l2a_file> <output_file>
//
// DESCRIPTION
//		Reads in a Level 2A file and writes out the following
//		linear_idx  sigma-0
//
//		where linear_idx = ati * ctwidth + cti
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//		<l2a_file>		The Level 2A input file.
//		<output_file>	The output file.
//
// EXAMPLES
//		An example of a command line is:
//			% l2a_s0 l2a.dat l2a.s0
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
#include <stdlib.h>
#include "Misc.h"
#include "L2A.h"
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

const char* usage_array[] = { "<l2a_file>", "<output_file>",
	"<cti>", "<ati>", 0};

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
	if (argc != 3 && argc != 5)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* l2a_file = argv[clidx++];
	const char* output_file = argv[clidx++];

	int cti = -1;
    int ati = -1;
	if (argc == 5)
	{
		cti = atoi(argv[clidx++]);
		ati = atoi(argv[clidx++]);
	}

	//------------------------//
	// open the Level 2A file //
	//------------------------//

	L2A l2a;
	if (! l2a.OpenForReading(l2a_file))
	{
		fprintf(stderr, "%s: error opening Level 2A file %s\n", command,
			l2a_file);
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

	//----------------//
	// loop and write //
	//----------------//

	while (l2a.ReadDataRec())
	{
		if (argc == 5 &&
			(cti != l2a.frame.cti || ati != l2a.frame.ati)) continue;

		MeasList* ml = &(l2a.frame.measList);
		LonLat lonlat = ml->AverageLonLat();
//                Meas* mhead=ml->GetHead();
//                double lat, lon, alt;
//                mhead->centroid.GetAltLonGCLat(&alt,&lon,&lat);
        fprintf(output_fp, "# Lon=%g Lat=%g\n",lonlat.longitude*rtd,
			lonlat.latitude*rtd);
		fprintf(output_fp, "# %d %d\n", l2a.frame.ati, l2a.frame.cti);
		for (Meas* m = ml->GetHead(); m; m = ml->GetNext())
		{
			fprintf(output_fp, "%s %g %g %g\n", beam_map[m->pol],
				m->incidenceAngle * rtd, m->eastAzimuth * rtd, m->value);
		}
		fprintf(output_fp, "#####\n");
	}

	//-----------------//
	// close the files //
	//-----------------//

	fclose(output_fp);
	l2a.Close();

	return (0);
}






