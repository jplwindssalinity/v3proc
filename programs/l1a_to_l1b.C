//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l10_to_l15
//
// SYNOPSIS
//		l10_to_l15 <sim_config_file>
//
// DESCRIPTION
//		Simulates the SeaWinds 1b ground processing of Level 1.0 to
//		Level 1.5 data.  This program converts the received power
//		into sigma-0.
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
//			% l10_to_l15 sws1b.cfg
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
#include "BufferedList.h"
#include "BufferedList.C"
#include "List.h"
#include "List.C"
#include "Misc.h"
#include "Ephemeris.h"
#include "ConfigList.h"
#include "L10.h"
#include "ConfigSim.h"
#include "L15.h"
#include "L10ToL15.h"

//-----------//
// TEMPLATES //
//-----------//

template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class List<WindVectorPlus>;

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

	L10 l10;
	if (! ConfigL10(&l10, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 1.0 Product\n", command);
		exit(1);
	}

	L15 l15;
	if (! ConfigL15(&l15, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 1.5 Product\n", command);
		exit(1);
	}

	//------------------------------//
	// create and configure antenna //
	//------------------------------//

	Instrument instrument;
	if (! ConfigAntenna(&instrument.antenna, &config_list))
	{
		fprintf(stderr, "%s: error configuring antenna\n", command);
		exit(1);
	}

	//--------------------------------//
	// create and configure ephemeris //
	//--------------------------------//

	Ephemeris ephemeris;
	if (! ConfigEphemeris(&ephemeris, &config_list))
	{
		fprintf(stderr, "%s: error configuring ephemeris\n", command);
		exit(1);
	}

	//------------//
	// open files //
	//------------//

	l10.file.OpenForInput();
	l15.file.OpenForOutput();

	//-----------------//
	// conversion loop //
	//-----------------//

	L10ToL15 l10_to_l15;

	do
	{
		//------------------------------//
		// read a level 1.0 data record //
		//------------------------------//

		if (! l10.ReadDataRec())
		{
			switch (l10.GetStatus())
			{
			case L10::OK:		// end of file
				break;
			case L10::ERROR_READING_FRAME:
				fprintf(stderr, "%s: error reading Level 1.0 data\n", command);
				exit(1);
				break;
			case L10::ERROR_UNKNOWN:
				fprintf(stderr, "%s: unknown error reading Level 1.0 data\n",
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

		if (! l10_to_l15.Convert(&l10, &instrument, &ephemeris, &l15))
		{
			fprintf(stderr, "%s: error converting Level 1.0 to Level 1.5\n",
				command);
			exit(1);
		}

		//-------------------------------//
		// write a level 1.5 data record //
		//-------------------------------//

		if (! l15.WriteDataRec())
		{
			fprintf(stderr, "%s: error writing Level 1.5 data\n", command);
			exit(1);
		}

	} while (1);

	l10.file.Close();
	l15.file.Close();

	return (0);
}
