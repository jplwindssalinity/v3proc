//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		plot_solution_curves
//
// SYNOPSIS
//		plot_solution_curves <sim_config_file>
//
// DESCRIPTION
//		Plots the solution curves for a given set of measurements.
//		Input values are read from standard input and are
//		polarization (1=V), incidence angle (deg), azimuth angle (deg),
//			and sigma-0 (dB)
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//		<sim_config_file>		The sim_config_file needed listing
//								all input parameters, input files, and
//								output files.
//
// EXAMPLES
//		An example of a command line is:
//			% plot_solution_curves sws1b.cfg < meas.dat > curves.plt
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
#include "ConfigList.h"
#include "GMF.h"
#include "ConfigSim.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<EarthPosition>;
template class List<StringPair>;
template class List<WindVectorPlus>;
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

const char* usage_array[] = { "<sim_config_file>", 0};

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
	const char* config_file = argv[clidx++];

	//---------------------//
	// read in config file //
	//---------------------//

	ConfigList config_list;
	if (! config_list.Read(config_file))
	{
		fprintf(stderr, "%s: error reading sim config file %s\n",
			command, config_file);
		exit(1);
	}

	//-------------------------------------//
	// read the geophysical model function //
	//-------------------------------------//
 
	GMF gmf;
	if (! ConfigGMF(&gmf, &config_list))
	{
		fprintf(stderr, "%s: error configuring GMF\n", command);
		exit(1);
	}

	//----------------------------//
	// read the values from stdin //
	//----------------------------//

	MeasList meas_list;
	for (;;)
	{
		int pol;
		float inc, azi, s0;
		if (scanf("%d %g %g %g", &pol, &inc, &azi, &s0) != 4)
			break;
		Meas* new_meas = new Meas();
		new_meas->value = pow(10.0, 0.1 * s0);
		if (pol == 1)
			new_meas->pol = V_POL;
		else
			new_meas->pol = H_POL;
		new_meas->eastAzimuth = (90.0 - azi) * dtr;
		new_meas->incidenceAngle = inc * dtr;
		meas_list.Append(new_meas);
	}

	//---------------------//
	// generate mod curves //
	//---------------------//

	gmf.WriteSolutionCurves(stdout, &meas_list, 0.0087, 0.0873, 0.0873,
		0.05, 4);

	return (0);
}
