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
#include "SimControl.h"
#include "SimConfig.h"

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
	sim_control_list.LogErrors();
	if (! sim_control_list.Read(sim_control_file))
	{
		fprintf(stderr, "%s: error reading sim control file %s\n",
			command, sim_control_file);
		exit(1);
	}

	//--------------------------------------------------//
	// create a simulation control object               //
	// use the simulation control list to initialize it //
	//--------------------------------------------------//

	SimControl sim_control;
	if (! ConfigSimControl(&sim_control, &sim_control_list))
	{
		fprintf(stderr, "%s: error configuring simulation\n", command);
		exit(1);
	}

	//----------------------//
	// cycle through events //
	//----------------------//

	SimEvent sim_event;
	while (sim_event.time < 12120.0)
	{
		sim_event = sim_control.NextEvent(sim_event.time);
		OrbitState os = sim_control.orbit.GetOrbitState((double)sim_event.time);
		printf("%g %g %g %g %g %g %g\n", sim_event.time, os.gc_vector[0],
			os.gc_vector[1], os.gc_vector[2], os.velocity_vector[0],
			os.velocity_vector[1], os.velocity_vector[2]);
	}

	return (0);
}
