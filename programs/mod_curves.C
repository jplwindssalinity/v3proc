//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		mod_curves
//
// SYNOPSIS
//		mod_curves <gmf_file> <measurement_file>
//
// DESCRIPTION
//		Plots the modulation curves and solution curve for the given
//		measurements.
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
//			% mod_curves nscat1b.dat meas.dat
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

#define DSPD	0.01
#define DPHI	1.0*dtr

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
			state = IN;
			break;
		case IN:
			if (strlen(line) == 1)
			{
				gmf.ModCurves(stdout, &measlist, DSPD, DPHI);
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
