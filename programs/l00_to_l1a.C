//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		l00_to_l1a
//
// SYNOPSIS
//		l00_to_l1a <sim_config_file>
//
// DESCRIPTION
//		Simulates the SeaWinds 1b ground processing of Level 0.0 to
//		Level 1A data.  This program converts the raw telemetry
//		(typically in dn) of the Level 0.0 product to engineering
//		units of the Level 1A product.
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
//			% l00_to_l1a sws1b.cfg
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
#include "L00.h"
#include "ConfigSim.h"
#include "L1A.h"
#include "L00ToL1A.h"
#include "Array.h"
#include "Tracking.h"
#include "Tracking.C"

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<AngleInterval>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class List<WindVectorPlus>;
template class List<long>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

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
	// create and configure level products //
	//-------------------------------------//

	L00 l00;
	if (! ConfigL00(&l00, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 0.0 Product\n", command);
		exit(1);
	}

	L1A l1a;
	if (! ConfigL1A(&l1a, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 1A Product\n", command);
		exit(1);
	}

	//------------//
	// open files //
	//------------//

	l00.OpenForReading();
	l1a.OpenForWriting();

	//-----------------//
	// conversion loop //
	//-----------------//

	L00ToL1A l00_to_l1a;

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

		if (! l00_to_l1a.Convert(&l00, &l1a))
		{
			fprintf(stderr, "%s: error converting Level 0.0 to Level 1A\n",
				command);
			exit(1);
		}

		//------------------------------//
		// write a level 1A data record //
		//------------------------------//

		if (! l1a.WriteDataRec())
		{
			fprintf(stderr, "%s: error writing Level 1 data\n", command);
			exit(1);
		}

	} while (1);

	l00.Close();
	l1a.Close();

	return (0);
}
