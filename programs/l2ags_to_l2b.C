//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		gsl2a_to_l2b
//
// SYNOPSIS
//		gsl2a_to_l2b <sim_config_file> <gs_l2a_file>
//
// DESCRIPTION
//		Simulates the SeaWinds 1b ground processing of Ground Sysytem
//		Level 2A to Level 2B data.  This program retrieves wind from
//		measurements.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//		<sim_config_file>		The sim_config_file needed listing
//								all input parameters, input files, and
//								output files.
//		<gs_l2a_file>			The ground system Level 2A file.
//
// EXAMPLES
//		An example of a command line is:
//			% l2a_to_l2b sws1b.cfg L2A.dat
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
#include "L2A.h"
#include "ConfigSim.h"
#include "L2B.h"
#include "L2AToL2B.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<long>;
template class List<OffsetList>;

//-----------//
// CONSTANTS //
//-----------//

#define CROSS_TRACK_RESOLUTION	25.0
#define ALONG_TRACK_RESOLUTION	25.0
#define CROSS_TRACK_WIDTH		2000.0

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

const char* usage_array[] = { "<sim_config_file>", "<gs_l2a_file>", 0};

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
	if (argc != 3)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* config_file = argv[clidx++];
	const char* gs_l2a_file = argv[clidx++];

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

	//-----------------------//
	// hack into config list //
	//-----------------------//

	if (! config_list.StompOrAppend(L2A_FILE_KEYWORD, gs_l2a_file))
	{
		fprintf(stderr, "%s: error hacking %s into config list\n",
			command, gs_l2a_file);
		exit(1);
	}

	//-------------------------------------//
	// create and configure level products //
	//-------------------------------------//

	L2A l2a;
	// this just sets the filename so we should be OK
	if (! ConfigL2A(&l2a, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 2A Product\n", command);
		exit(1);
	}

	L2B l2b;
	if (! ConfigL2B(&l2b, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 2B Product\n", command);
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

	//--------------//
	// configure Kp //
	//--------------//

/*
	Kp kp;
	if (! ConfigKp(&kp, &config_list))
	{
		fprintf(stderr, "%s: error configuring Kp\n", command);
		exit(1);
	}
*/

	//----------------------//
	// create the converter //
	//----------------------//

	L2AToL2B l2a_to_l2b;

	//------------//
	// open files //
	//------------//

	l2a.OpenForReading();
	l2b.OpenForWriting();

	//---------------------------------//
	// read the header to set up swath //
	//---------------------------------//

/*
	if (! l2a.ReadHeader())
	{
		fprintf(stderr, "%s: error reading Level 2A header\n", command); 
		exit(1);
	}
*/

	// assume 25 km to get along track dimension
	int along_track_bins = (int)(two_pi * r1_earth / ALONG_TRACK_RESOLUTION +
		0.5);
	int cross_track_bins = (int)(CROSS_TRACK_WIDTH / CROSS_TRACK_RESOLUTION +
		0.5);

	if (! l2b.frame.swath.Allocate(cross_track_bins, along_track_bins))
	{
		fprintf(stderr, "%s: error allocating wind swath\n", command);
		exit(1);
	}

	//-----------------------------------------//
	// transfer information to level 2B header //
	//-----------------------------------------//

	l2b.header.crossTrackResolution = CROSS_TRACK_RESOLUTION;
	l2b.header.alongTrackResolution = ALONG_TRACK_RESOLUTION;
//	l2b.header.zeroIndex = l2a.header.zeroIndex;

	//-----------------//
	// conversion loop //
	//-----------------//

	for (;;)
	{
		//-----------------------------//
		// read a level 2A data record //
		//-----------------------------//

		if (! l2a.ReadGSDataRec())
		{
			switch (l2a.GetStatus())
			{
			case L2A::OK:		// end of file
				break;
			case L2A::ERROR_READING_FRAME:
				fprintf(stderr, "%s: error reading Level 2A data\n", command);
//				exit(1);
				break;
			case L2A::ERROR_UNKNOWN:
				fprintf(stderr, "%s: unknown error reading Level 2A data\n",
					command);
//				exit(1);
				break;
			default:
				fprintf(stderr, "%s: unknown status (???)\n", command);
//				exit(1);
			}
			break;		// done, exit do loop
		}

		if (l2a.frame.ati < 100)
			continue;
		if (l2a.frame.ati > 600)
			break;
printf("%d\n", l2a.frame.ati);

		//---------//
		// convert //
		//---------//

		int retval = l2a_to_l2b.ConvertAndWrite(&l2a, &gmf, NULL, &l2b);
		switch (retval)
		{
		case 1:
			break;
		case 2:
			break;
		case 4:
		case 5:
			break;
		case 0:
			fprintf(stderr, "%s: error converting Level 2A to Level 2B\n",
				command);
			exit(1);
			break;
		}
	}

	l2a_to_l2b.Flush(&l2b);

	l2a.Close();
	l2b.Close();

	return (0);
}
