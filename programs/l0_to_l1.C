//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l0_to_l1
//
// SYNOPSIS
//		l0_to_l1 <sim_config_file>
//
// DESCRIPTION
//		Simulates the SeaWinds 1b ground processing of Level 0 to
//		Level 1 data.  This program converts the raw telemetry
//		(typically in dn) of the Level 0 product to engineering units
//		of the Level 1 product.
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
//			% l0_to_l1 sws1b.cfg
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
#include "L0.h"
#include "ConfigSim.h"
#include "L1.h"
#include "L0ToL1.h"

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
	config_list.LogErrors();
	if (! config_list.Read(config_file))
	{
		fprintf(stderr, "%s: error reading sim config file %s\n",
			command, config_file);
		exit(1);
	}

	//--------------------------------------//
	// create and configure product objects //
	//--------------------------------------//

	L0 l0;
	if (! ConfigL0(&l0, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 0 Product\n", command);
		exit(1);
	}

	L1 l1;
	if (! ConfigL1(&l1, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 1 Product\n", command);
		exit(1);
	}

	//---------//
	// convert //
	//---------//

	L0ToL1 l0_to_l1;
	while (l0.ReadDataRec())
	{
		if (! l0_to_l1.Convert(&l0, &l1))
		{
			fprintf(stderr, "%s: error converting Level 0 to Level 1\n",
				command);
			exit(1);
		}
		l1.WriteDataRec();
	}
	l0.CloseCurrentFile();
	l1.CloseCurrentFile();
		
	return (0);
}
