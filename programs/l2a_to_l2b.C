//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l17_to_l20
//
// SYNOPSIS
//		l17_to_l20 <sim_config_file>
//
// DESCRIPTION
//		Simulates the SeaWinds 1b ground processing of Level 1.7 to
//		Level 2.0 data.  This program retrieves wind from measurements.
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
//			% l17_to_l20 sws1b.cfg
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
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "ConfigList.h"
#include "L17.h"
#include "ConfigSim.h"
#include "L20.h"
#include "L17ToL20.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<Meas>;
template class List<LonLat>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;

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

	L17 l17;
	if (! ConfigL17(&l17, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 1.7 Product\n", command);
		exit(1);
	}

	L20 l20;
	if (! ConfigL20(&l20, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 2.0 Product\n", command);
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

	//------------//
	// open files //
	//------------//

	l17.file.OpenForInput();
	l20.file.OpenForOutput();

	//-----------------//
	// conversion loop //
	//-----------------//

	L17ToL20 l17_to_l20;

	do
	{
		//------------------------------//
		// read a level 1.7 data record //
		//------------------------------//

		if (! l17.ReadDataRec())
		{
			switch (l17.GetStatus())
			{
			case L17::OK:		// end of file
				break;
			case L17::ERROR_READING_FRAME:
				fprintf(stderr, "%s: error reading Level 1.7 data\n", command);
				exit(1);
				break;
			case L17::ERROR_UNKNOWN:
				fprintf(stderr, "%s: unknown error reading Level 1.7 data\n",
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

		if (! l17_to_l20.ConvertAndWrite(&l17, &gmf, &l20))
		{
			fprintf(stderr, "%s: error converting Level 1.7 to Level 2.0\n",
				command);
			exit(1);
		}

	} while (1);

	l17.file.Close();
	l20.Close();

	return (0);
}
