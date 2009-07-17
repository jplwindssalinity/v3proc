//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		orbit_sim
//
// SYNOPSIS
//		orbit_sim <orbit_config_file>
//
// DESCRIPTION
//		Simulates an orbit based on the configuration file.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//		<orbit_config_file>		The orbital parameters
//
// EXAMPLES
//		An example of a command line is:
//			% orbit_sim orbit.cfg
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
#include "Misc.h"
#include "List.h"
#include "List.C"
#include "ConfigList.h"
#include "Tracking.h"
#include "Tracking.C"
#include "Spacecraft.h"
#include "SpacecraftSim.h"
#include "ConfigSim.h"
#include "BufferedList.h"
#include "BufferedList.C"

//-----------//
// TEMPLATES //
//-----------//
class AngleInterval;

template class List<AngleInterval>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<WindVectorPlus>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;


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

const char* usage_array[] = { "<orbit_config_file>", 0};

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

	//---------------------------//
	// read in orbit config file //
	//---------------------------//

	ConfigList config_list;
	if (! config_list.Read(config_file))
	{
		fprintf(stderr, "%s: error reading config file %s\n",
			command, config_file);
		exit(1);
	}

	//----------------------------------------------//
	// create a spacecraft and spacecraft simulator //
	//----------------------------------------------//

	Spacecraft spacecraft;

	SpacecraftSim spacecraft_sim;
	if (! ConfigSpacecraftSim(&spacecraft_sim, &config_list))
	{
		fprintf(stderr, "%s: error configuring spacecraft simulator\n",
			command);
		exit(1);
	}

	//--------------------------//
	// create an ephemeris file //
	//--------------------------//

/*
	char* epehemeris_filename;
	epehemeris_filename = config_list.Get(EPHEMERIS_FILE_KEYWORD);
	if (! epehemeris_filename)
	{
		fprintf(stderr, "%s: error getting ephemeris filename\n", command);
		exit(1);
	}
	FILE* eph_fp = fopen(epehemeris_filename, "w");
	if (eph_fp == NULL)
	{
		fprintf(stderr, "%s: error opening ephemeris file %s\n", command,
			epehemeris_filename);
		exit(1);
	}
*/


        // set starting time to southernmost point
        double epoch=spacecraft_sim.GetEpoch();
        double spacecraft_start_time=spacecraft_sim.FindPrevArgOfLatTime(epoch,-pi/2,EQX_TIME_TOLERANCE);

	if (! spacecraft_sim.Initialize(spacecraft_start_time))
	  {
	    fprintf(stderr, "%s: error initializing spacecraft simulator\n",
		    command);
	    exit(1);
	  }


	double period=spacecraft_sim.GetPeriod();
	//----------------------//
	// cycle through events //
	//----------------------//

	SpacecraftEvent spacecraft_event;

	for (;;)
	{
		spacecraft_sim.DetermineNextEvent(&spacecraft_event);

		switch(spacecraft_event.eventId)
		{
		case SpacecraftEvent::UPDATE_STATE:
			spacecraft_sim.UpdateOrbit(spacecraft_event.time,
				&spacecraft);
			double alt, lat, lon;
			spacecraft.orbitState.rsat.GetAltLonGDLat(&alt,&lon,&lat);
			printf("%g %g %g %g\n", spacecraft.orbitState.time-spacecraft_start_time, lat*rtd, lon*rtd, alt);
//			spacecraft.orbitState.Write(eph_fp);
			break;
		case SpacecraftEvent::EQUATOR_CROSSING:
		        break;
		default:
			fprintf(stderr, "%s: unknown spacecraft event\n", command);
			exit(1);
			break;
		}
		if (spacecraft.orbitState.time > period+spacecraft_start_time)
			break;
	}

	return (0);
}
