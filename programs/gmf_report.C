//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		gmf_report
//
// SYNOPSIS
//		gmf_report <gmf_file_1> <gmf_file_2>
//
// DESCRIPTION
//		Reads in two old-style model functions and generates many curves.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operands are supported:
//		<gmf_file_1>		A Geophysical Model Function file.
//		<gmf_file_2>		A Geophysical Model Function file.
//
// EXAMPLES
//		An example of a command line is:
//			% gmf_report nscat1b.ogmf sassw.ogmf
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
#include <math.h>
#include "Misc.h"
#include "GMF.h"

//-----------//
// CONSTANTS //
//-----------//

#define QUOTES	'"'

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

const char* usage_array[] = { "<gmf_file_1>", "<gmf_file_2>", 0};
const char* pol_map[] = { "V", "H" };

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
	const char* gmf1_file = argv[clidx++];
	const char* gmf2_file = argv[clidx++];

	//------------------//
	// read in gmf file //
	//------------------//

	GMF gmf1;
	if (! gmf1.ReadOldStyle(gmf1_file))
	{
		fprintf(stderr, "%s: error reading GMF file %s\n", command, gmf1_file);
		exit(1);
	}

	GMF gmf2;
	if (! gmf2.ReadOldStyle(gmf2_file))
	{
		fprintf(stderr, "%s: error reading GMF file %s\n", command, gmf2_file);
		exit(1);
	}

	char filename[1024];
	int pol;
	double inc, spd, chi, s0;

	for (pol = 0; pol < 2; pol++)
	{
		//--------------------------//
		// sigma-0 dif vs. velocity //
		//--------------------------//

		sprintf(filename, "%s.%s.a0", gmf1_file, pol_map[pol]);
		FILE *fp1 = fopen_or_exit(filename, "w", command, "GMF output file", 1);
		fprintf(fp1, "@ title %c%s%c\n", QUOTES, filename, QUOTES);

		sprintf(filename, "%s.%s.a0", gmf2_file, pol_map[pol]);
		FILE *fp2 = fopen_or_exit(filename, "w", command, "GMF output file", 1);
		fprintf(fp2, "@ title %c%s%c\n", QUOTES, filename, QUOTES);

		sprintf(filename, "%s-%s.%s.a0", gmf1_file, gmf2_file,
			pol_map[pol]);
		FILE *fp = fopen_or_exit(filename, "w", command, "GMF output file", 1);
		fprintf(fp, "@ title %c%s%c\n", QUOTES, filename, QUOTES);

		fprintf(fp1, "@ legend on\n");
		fprintf(fp2, "@ legend on\n");
		fprintf(fp, "@ legend on\n");

		int legend_idx = 0;
		for (inc = 20.0; inc <= 61.0; inc += 5.0)
		{
			fprintf(fp, "@ legend string %d %c%g%c\n", legend_idx, QUOTES,
				inc, QUOTES);

			for (spd = 1.0; spd <= 50.0; spd += 0.5)
			{
				double gmf1_s0_sum = 0.0;
				int gmf1_s0_count = 0;
				double gmf2_s0_sum = 0.0;
				int gmf2_s0_count = 0;
				for (chi = 0.0; chi < 360.0; chi += 5.0)
				{
					if (gmf1.GetInterSigma0(pol, inc, spd, chi, &s0))
					{
						gmf1_s0_sum += s0;
						gmf1_s0_count++;
					}
					if (gmf2.GetInterSigma0(pol, inc, spd, chi, &s0))
					{
						gmf2_s0_sum += s0;
						gmf2_s0_count++;
					}
				}
				double s01 = gmf1_s0_sum / (double)gmf1_s0_count;
				double s02 = gmf2_s0_sum / (double)gmf2_s0_count;
				double dif = s01 / s02;
				fprintf(fp1, "%g %g\n", spd, 10.0 * log10(s01));
				fprintf(fp2, "%g %g\n", spd, 10.0 * log10(s02));
				fprintf(fp, "%g %g\n", spd, 10.0 * log10(dif));
			}
			fprintf(fp1, "&\n");
			fprintf(fp2, "&\n");
			fprintf(fp, "&\n");
			legend_idx++;
		}
		fclose(fp);
	}

	return (0);
}
