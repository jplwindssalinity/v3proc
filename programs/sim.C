//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		sim
//
// SYNOPSIS
//		sim <sim_config_file>
//
// DESCRIPTION
//		Simulates the SeaWinds 1b instrument based on the parameters
//		in the simulation configuration file.
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
//			% sim sws1b.cfg
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
#include "ConfigList.h"
#include "InstrumentSim.h"
/*
#include <string.h>
#include "SimControl.h"
#include "SimConfig.h"
*/

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

	//--------------------------------//
	// read in simulation config file //
	//--------------------------------//

	ConfigList config_list;
	config_list.LogErrors();
	if (! config_list.Read(config_file))
	{
		fprintf(stderr, "%s: error reading sim config file %s\n",
			command, config_file);
		exit(1);
	}

	//-----------------------------------------------//
	// create an instrument simulator and initialize //
	//-----------------------------------------------//

	InstrumentSim sim;
	if (! sim.InitByConfig(&config_list))
	{
		fprintf(stderr, "%s: error configuring instrument simulation\n",
			command);
		exit(1);
	}

	//----------------------//
	// cycle through events //
	//----------------------//

	double time = 0.0;
	while (time < 12120.0)
	{
		sim.SimulateNextEvent(&time);
		Orbit *os = &(sim.instrument.orbit);
		printf("%g %g %g %g %g %g %g\n", time, os->gc_vector[0],
			os->gc_vector[1], os->gc_vector[2], os->velocity_vector[0],
			os->velocity_vector[1], os->velocity_vector[2]);
	}

	return (0);
}
