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
#include "Instrument.h"
#include "ConfigSim.h"
#include "L00File.h"
#include "L00Frame.h"

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

	//-------------------------------------//
	// create an instrument and initialize //
	//-------------------------------------//

	Instrument instrument;
	if (! ConfigInstrument(&instrument, &config_list))
	{
		fprintf(stderr, "%s: error configuration instrument\n", command);
		exit(1);
	}

	//-----------------------------------------------//
	// create an instrument simulator and initialize //
	//-----------------------------------------------//

	InstrumentSim sim;
	if (! ConfigInstrumentSim(&sim, &config_list))
	{
		fprintf(stderr, "%s: error configuring instrument simulation\n",
			command);
		exit(1);
	}

	//----------------------------------------//
	// create a Level 0.0 file and initialize //
	//----------------------------------------//

	L00File l00_file;
	if (! ConfigL00File(&l00_file, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 0.0\n", command);
		exit(1);
	}
	l00_file.OpenForOutput();

	//----------------------//
	// cycle through events //
	//----------------------//

	WindField windfield;
	GMF gmf;
	L00Frame l00_frame;
	char l00_buffer[MAX_L00_BUFFER_SIZE];

	Event event;
	while (event.time < 120.0)
	{
		sim.DetermineNextEvent(&event);
		sim.SimulateEvent(&instrument, &event, &windfield, &gmf);
		if (sim.GetL00Frame(&l00_frame))
		{
			int size = l00_frame.Pack(l00_buffer);
			l00_file.Write(l00_buffer, size);
		}
	}
	l00_file.Close();

	return (0);
}
