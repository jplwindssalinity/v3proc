//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l00_to_l10
//
// SYNOPSIS
//		l00_to_l10 <sim_config_file>
//
// DESCRIPTION
//		Simulates the SeaWinds 1b ground processing of Level 0.0 to
//		Level 1.0 data.  This program converts the raw telemetry
//		(typically in dn) of the Level 0.0 product to engineering
//		units of the Level 1.0 product.
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
//			% l00_to_l10 sws1b.cfg
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
#include "List.h"
#include "List.C"
#include "Misc.h"
#include "ConfigList.h"
#include "L00.h"
#include "ConfigSim.h"
#include "L10.h"
#include "L00ToL10.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<Meas>;
template class List<LonLat>;
template class List<MeasSpot>;
template class List<OrbitState>;
template class List<WindVector>;

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

	//-------------------------------------//
	// create and configure level products //
	//-------------------------------------//

	L00 l00;
	if (! ConfigL00(&l00, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 0.0 Product\n", command);
		exit(1);
	}

	L10 l10;
	if (! ConfigL10(&l10, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 1.0 Product\n", command);
		exit(1);
	}

	//------------//
	// open files //
	//------------//

	l00.file.OpenForInput();
	l10.file.OpenForOutput();

	//-----------------//
	// conversion loop //
	//-----------------//

	L00ToL10 l00_to_l10;

	do
	{
		//----------------------------//
		// read a level 0 data record //
		//----------------------------//

		if (! l00.ReadDataRec())
		{
			switch (l00.GetStatus())
			{
			case L00::OK:		// end of file
				break;
			case L00::ERROR_READING_FRAME:
				fprintf(stderr, "%s: error reading Level 0.0 data\n", command);
				exit(1);
				break;
			case L00::ERROR_UNKNOWN:
				fprintf(stderr, "%s: unknown error reading Level 0.0 data\n",
					command);
				exit(1);
				break;
			default:
				fprintf(stderr, "%s: unknown status (???)\n", command);
				exit(1);
			}
			break;		// done, exit do loop
		}

		//---------//
		// convert //
		//---------//

		if (! l00_to_l10.Convert(&l00, &l10))
		{
			fprintf(stderr, "%s: error converting Level 0.0 to Level 1.0\n",
				command);
			exit(1);
		}

		//-------------------------------//
		// write a level 1.0 data record //
		//-------------------------------//

		if (! l10.WriteDataRec())
		{
			fprintf(stderr, "%s: error writing Level 1 data\n", command);
			exit(1);
		}

	} while (1);

	l00.file.Close();
	l10.file.Close();

	return (0);
}
