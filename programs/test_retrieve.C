//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		test_retrieve
//
// SYNOPSIS
//		test_retrieve <gmf_file> <measurement_file>
//
// DESCRIPTION
//		Tests the wind retrieval by generating GMF values for a given
//		wind vector with given polarization and incidence angles, and
//		the calling the wind retrieval algorithm to obtain (hopefully)
//		the original wind vector.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operands are supported:
//		<gmf_file>			The GMF file.
//		<measurement_file>	A file containing the following for each
//							wind vector cell:
//
//			Wind_Speed  Wind_Direction
//			Polarization_1  Incidence_Angle_1  Azimuth_Angle_1
//			Polarization_2  Incidence_Angle_2  Azimuth_Angle_2
//			...
//			<blank line>
//
// EXAMPLES
//		An example of a command line is:
//			% test_retrieve nscat1b.dat meas.dat
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

#include <stdlib.h>
#include <string.h>
#include "Misc.h"
#include "GMF.h"
#include "Constants.h"

//-----------//
// CONSTANTS //
//-----------//

#define LINE_SIZE	1024

#define INIT_SPD	0.05
#define INIT_PHI	1.0*dtr
#define FINAL_SPD	0.01
#define FINAL_PHI	0.1*dtr

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

enum LoopStateE { READY, IN };

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<gmf_file>", "<measurement_file>", 0};

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
	const char* gmf_file = argv[clidx++];
	const char* meas_file = argv[clidx++];

	//-------------//
	// read in GMF //
	//-------------//

	GMF gmf;
	if (! gmf.ReadOldStyle(gmf_file))
	{
		fprintf(stderr, "%s: error reading GMF file %s\n", command,
			gmf_file);
		exit(1);
	}

	//---------------------------//
	// open the measurement file //
	//---------------------------//

	FILE* ifp = fopen(meas_file, "r");
	if (! ifp)
	{
		fprintf(stderr, "%s: error opening measurment file %s\n", command,
			meas_file);
		exit(1);
	}

	char line[LINE_SIZE];
	LoopStateE state = READY;
	MeasurementList measlist;
	WVC* wvc = NULL;
	double spd, dir;
	int done = 0;
	do
	{
		if (fgets(line, LINE_SIZE, ifp) != line)
		{
			// force a retrieve
			line[0] = '\n';
			line[1] = '\0';
			state = IN;
			done = 1;
		}
		switch(state)
		{
		case READY:
			if (sscanf(line, "%lf %lf", &spd, &dir) != 2)
			{
				fprintf(stderr, "%s: error reading speed and direction\n",
					command);
				exit(1);
			}
			dir *= dtr;
			wvc = new WVC;
			state = IN;
			break;
		case IN:
			if (strlen(line) == 1)
			{
				// retrieve!
				gmf.FindSolutions(&measlist, wvc, INIT_SPD, INIT_PHI);
				wvc->WriteAmbigsAscii(stdout);
				gmf.RefineSolutions(&measlist, wvc, INIT_SPD, INIT_PHI,
					FINAL_SPD, FINAL_PHI);
				printf("&\n");
				wvc->WriteAmbigsAscii(stdout);
				wvc->RemoveDuplicates();
				printf("&\n");
				wvc->WriteAmbigsAscii(stdout);
				delete wvc;
				state = READY;
			}
			else
			{
				PolE pol;
				double inc, az;
				if (sscanf(line, "%d %lf %lf", &pol, &inc, &az) != 3)
				{
					fprintf(stderr, "%s: error reading pol, inc, az\n",
						command);
					exit(1);
				}
				Measurement* meas = new Measurement();
				meas->pol = (PolE)pol;
				meas->incidenceAngle = inc;
				meas->eastAzimuth = az * dtr;
				double chi = (dir - (meas->eastAzimuth + pi));
				while (chi < 0.0)
					chi += two_pi;
				while (chi > two_pi)
					chi -= two_pi;
				gmf.GetInterpolatedValue(meas->pol, meas->incidenceAngle,
					spd, chi, &(meas->value));
				measlist.Append(meas);
			}
		}
	} while (! done);

	return (0);
}
