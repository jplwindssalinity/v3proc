//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l20_metrics
//
// SYNOPSIS
//		l20_metrics <l20_file> <truth> [ output_base ]
//
// DESCRIPTION
//		Generates output files containing wind retrieval metrics
//		including: RMS speed error, RMS direction error, speed bias,
//		first ranked skill, first + second ranked skill, ambiguity
//		removal skill.
//
// OPTIONS
//		[ output_base ]	The base name to use for output files.  If
//						omitted, <l20_file> is used as the base name.
//
// OPERANDS
//		The following operand is supported:
//		<l20_file>		The level 2.0 file to read.
//		<truth>			The truth wind field.
//
// EXAMPLES
//		An example of a command line is:
//			% l20_metrics l20.dat vap.wf metrics
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
#include "List.h"
#include "List.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<WindVectorPlus>;
template class List<EarthPosition>;

//-----------//
// CONSTANTS //
//-----------//

#define ARRAY_SIZE		1024

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

const char* usage_array[] = { "<l20_file>", "<truth>", "[ output_base ]", 0};

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
	if (argc < 3 || argc > 4)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* l20_file = argv[clidx++];
	const char* truth_file = argv[clidx++];
	const char* output_base;
	if (argc == 4)
		output_base = argv[clidx++];
	else
		output_base = l20_file;

	//-----------------------------------//
	// read in level 2.0 file wind swath //
	//-----------------------------------//

	WindSwath swath;
	swath.ReadL20(l20_file);

	//----------------------------//
	// read in "truth" wind field //
	//----------------------------//

	WindField truth;
	truth.ReadVap(truth_file);

	//-------------//
	// generate... //
	//-------------//

	//-------------------------//
	// rms speed error vs. ctd //
	//-------------------------//

	float value[ARRAY_SIZE];
	int count[ARRAY_SIZE];
	float ctd[ARRAY_SIZE];
	int max_idx;
	if (! swath.RmsSpeedError(&truth, value, count, ctd, &max_idx))
	{
		fprintf(stderr, "%s: error calculating RMS speed error\n", command);
		exit(1);
	}

	char filename[1024];
	sprintf(filename, "%s.rms_spd_err", output_base);
	FILE* ofp = fopen(filename, "w");
	if (ofp == NULL)
	{
		fprintf(stderr, "%s: error opening output file %s\n", command,
			filename);
		exit(1);
	}

	for (int i = 0; i < max_idx; i++)
	{
		if (count[i] > 0)
			fprintf(ofp, "%g %g %d\n", ctd[i], value[i], count[i]);
	}
	fclose(ofp);

	//-----------------------------//
	// rms direction error vs. ctd //
	//-----------------------------//

	return (0);
}
