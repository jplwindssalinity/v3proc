//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		sim
//
// SYNOPSIS
//		sim <sim_control_file>
//
// DESCRIPTION
//		Simulates the SeaWinds 1b instrument based on the parameters
//		in the simulation control file.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//		<sim_control_file>		The sim_control_file needed listing
//								all input parameters, input files, and
//								output files.
//
// EXAMPLES
//		An example of a command line is:
//			% sim sim.ctrl.1
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
#include <string.h>
#include "Misc.h"
#include "ConfigList.h"
#include "OrbitSim.h"

//-----------//
// CONSTANTS //
//-----------//

#define SEMI_MAJOR_AXIS_KEYWORD			"SEMI_MAJOR_AXIS"
#define ECCENTRICITY_KEYWORD			"ECCENTRICITY"
#define INCLINATION_KEYWORD				"INCLINATION"
#define ARGUMENT_OF_PERIGEE_KEYWORD		"ARGUMENT_OF_PERIGEE"
 
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

const char* usage_array[] = { "<sim_control_file>", 0};

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
	const char* sim_control_file = argv[clidx++];

	//---------------------------------//
	// read in simulation control file //
	//---------------------------------//

	ConfigList sim_control_list;
	if (! sim_control_list.Read(sim_control_file))
	{
		fprintf(stderr, "%s: error reading sim control file %s\n",
			command, sim_control_file);
		exit(1);
	}

	//-------------------------//
	// set up orbit parameters //
	//-------------------------//

	double semi_major_axis;
	sim_control_list.GetDouble(SEMI_MAJOR_AXIS_KEYWORD,
		&semi_major_axis);

	double eccentricity;
	sim_control_list.GetDouble(ECCENTRICITY_KEYWORD,
		&eccentricity);

	double inclination;
	sim_control_list.GetDouble(INCLINATION_KEYWORD,
		&inclination);

	double argument_of_perigee;
	sim_control_list.GetDouble(ARGUMENT_OF_PERIGEE_KEYWORD,
		&argument_of_perigee);

	OrbitSim orbit_sim(semi_major_axis, eccentricity, inclination,
		argument_of_perigee);

	orbit_sim.Initialize(0.0, 0.0, 0);

	for (int itime = 0; itime < 30300; itime += 10)
	{
		OrbitState os = orbit_sim.GetOrbitState((double)itime);
		printf("%d %g %g\n", itime, os.gc_longitude * RTD, os.gc_latitude * RTD);
	}

	return (0);
}
