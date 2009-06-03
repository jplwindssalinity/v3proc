//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		write_kpm_table
//
// SYNOPSIS
//		write_kpm_table <output_file>
//
// DESCRIPTION
//		Writes out the Kpm table in the proper format.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operands are supported:
//		<output_file>	The output file to contain the Kpm table
//
// EXAMPLES
//		An example of a command line is:
//			% write_kpm_table kpm.tab
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
#include <stdlib.h>
#include "Misc.h"
#include "Index.h"

//-----------//
// TEMPLATES //
//-----------//

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

const char* usage_array[] = { "<output_file>", 0};

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
	if (argc != 2)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* output_file = argv[clidx++];

	//--------------------//
	// generate the table //
	//--------------------//

	// V-pol is index 0, H-pol is index 1 for the 1st dim.
	static float kpm_table[2][36] =
	{ {6.3824e-01, 5.6835e-01, 4.9845e-01, 4.2856e-01, 3.5867e-01, 2.8877e-01,
	2.5092e-01, 2.1307e-01, 1.9431e-01, 1.7555e-01, 1.7072e-01, 1.6589e-01,
	1.6072e-01, 1.5554e-01, 1.4772e-01, 1.3990e-01, 1.2843e-01, 1.1696e-01,
	1.1656e-01, 1.1615e-01, 1.0877e-01, 1.0138e-01, 9.0447e-02, 7.9516e-02,
	8.6400e-02, 9.3285e-02, 8.4927e-02, 7.6569e-02, 7.2302e-02, 6.8036e-02,
	7.7333e-02, 8.6630e-02, 9.0959e-02, 9.5287e-02, 9.9616e-02, 1.0394e-01},
	{4.3769e-01, 4.0107e-01, 3.6446e-01, 3.2784e-01, 2.9122e-01, 2.5461e-01,
	2.2463e-01, 1.9464e-01, 1.7066e-01, 1.4667e-01, 1.3207e-01, 1.1747e-01,
	1.0719e-01, 9.6918e-02, 9.0944e-02, 8.4969e-02, 7.7334e-02, 6.9699e-02,
	6.9107e-02, 6.8515e-02, 6.6772e-02, 6.5030e-02, 5.7429e-02, 4.9828e-02,
	4.3047e-02, 3.6266e-02, 3.0961e-02, 2.5656e-02, 2.9063e-02, 3.2471e-02,
	2.7050e-02, 2.1629e-02, 2.8697e-02, 3.5764e-02, 4.2831e-02, 4.9899e-02}};

	//------------------//
	// open output file //
	//------------------//

	FILE* ofp = fopen(output_file, "w");
	if (ofp == NULL)
	{
		fprintf(stderr, "%s: error opening output Kpm file %s\n", command,
			output_file);
		exit(1);
	}

	//--------------//
	// write header //
	//--------------//

	Index kpm_idx;
	kpm_idx.SpecifyCenters(0.0, 35.0, 36);
	kpm_idx.Write(ofp);

	//-----------------//
	// write out table //
	//-----------------//

	unsigned int bins = kpm_idx.GetBins();
	for (int i = 0; i < 2; i++)
	{
		if (fwrite((void *)*(kpm_table + i), sizeof(float), bins, ofp) != bins)
		{
			fprintf(stderr, "%s: error writing kpm table\n", command);
			exit(0);
		}
	}

	//------------//
	// close file //
	//------------//

	fclose(ofp);

	return (0);
}
