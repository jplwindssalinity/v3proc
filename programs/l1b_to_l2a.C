//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		l1b_to_l2a
//
// SYNOPSIS
//		l1b_to_l2a <sim_config_file>
//
// DESCRIPTION
//		Simulates the SeaWinds 1b ground processing of Level 1B to
//		Level 2A data.  This program groups the sigma0 data onto
//		a grid.
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
//			% l1b_to_l2a sws1b.cfg
//
// ENVIRONMENT
//		Not environment dependent.
//
// EXIT STATUS
//		The following exit values are returned:
//		1	Program executed successfully
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
#include "ConfigSim.h"
#include "Ephemeris.h"
#include "L1B.h"
#include "L2A.h"
#include "L1BToL2A.h"
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
template list<string>;
template map<string,string,Options::ltstr>;

//-----------//
// CONSTANTS //
//-----------//

#define	USE_COMPOSITING_KEYWORD		"USE_COMPOSITING"

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

	//----------------------//
	// Get compositing flag //
	//----------------------//

	int use_compositing;
	if (! config_list.GetInt(USE_COMPOSITING_KEYWORD, &use_compositing))
		return(0);

	//-----------------------//
	// create spacecraft sim //
	//-----------------------//

	SpacecraftSim spacecraft_sim;
	if (! ConfigSpacecraftSim(&spacecraft_sim, &config_list))
	{
		fprintf(stderr, "%s: error configuring spacecraft simulator\n",
			command);
		exit(1);
	}

	//---------------------------//
	// create and configure Grid //
	//---------------------------//

	Grid grid;
	if (! ConfigGrid(&grid, &config_list))
	{
		fprintf(stderr, "%s: error configuring grid\n", command);
		exit(1);
	}

	//-------------------------------//
	// configure the grid start time //
	//-------------------------------//

	double grid_start_time, grid_end_time;
	double instrument_start_time, instrument_end_time;
	double spacecraft_start_time, spacecraft_end_time;

	if (! ConfigControl(&spacecraft_sim, &config_list,
		&grid_start_time, &grid_end_time,
		&instrument_start_time, &instrument_end_time,
		&spacecraft_start_time, &spacecraft_end_time))
	{
		fprintf(stderr, "%s: error configuring simulation times\n", command);
		exit(1);
	}
	grid.SetStartTime(grid_start_time);
	grid.SetEndTime(grid_end_time);

	//------------//
	// open files //
	//------------//

	grid.l1b.OpenForReading();
	grid.l2a.OpenForWriting();

	//-----------------//
	// conversion loop //
	//-----------------//

	L1BToL2A l1b_to_l2a;

	long counter = 0;
	do
	{
		counter++;
		if (counter % 1000 == 0) printf("L1B record count = %ld bytes cout =%g\n",counter,(double)ftello(grid.l1b.GetInputFp()));

		//-----------------------------//
		// read a level 1B data record //
		//-----------------------------//

		if (! grid.l1b.ReadDataRec())
		{
			switch (grid.l1b.GetStatus())
			{
			case L1B::OK:	// end of file
				break;
			case L1B::ERROR_READING_FRAME:
				fprintf(stderr, "%s: error reading Level 1B data\n", command);
				exit(1);
				break;
			case L1B::ERROR_UNKNOWN:
				fprintf(stderr, "%s: unknown error reading Level 1B data\n",
					command);
				exit(1);
				break;
			default:
				fprintf(stderr, "%s: unknown status\n", command);
				exit(1);
			}
			break;	// done, exit do loop
		}

		//-------//
		// Group //
		//-------//

		if (! l1b_to_l2a.Group(&grid, use_compositing))
		{
			fprintf(stderr, "%s: error converting Level 1B to Level 2A\n",
				command);
			exit(1);
		}

	} while (1);

	//
	// Write out data in the grid that hasn't been written yet.
	//

	grid.Flush(use_compositing);

	grid.l1b.Close();
	grid.l2a.Close();

	return (0);
}
