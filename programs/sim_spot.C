//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		sim_spot
//
// SYNOPSIS
//		sim_spot <sim_config_file>
//
// DESCRIPTION
//		Simulates the motion of a beam spot (defined by beamwidth parameters)
//		on the ground.  An output file containing spot data as a function of
//		time is created for subsequent display.
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
#include "ConfigSim.h"
#include "LookGeom.h"
#include "Instrument.h"

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

	//
	// Setup variables for the basic geometry computations.
    //

    Attitude sc_att;
    Attitude ant_att;
    sc_att.Set(0,0,0,1,2,3);	// assume perfect attitude
    ant_att.Set(0,0,0,1,2,3);	// antenna frame equals s/c frame
    EarthPosition rspot;        // beam spot on the ground
    Vector3 rspot_geodetic;     // beam spot on ground in alt,lat,lon terms
    Vector3 rlook_ant;  // beam look direction in antenna frame
    Vector3 rlook_geo;  // beam look direction in geocentric frame
    EarthPosition rsat;

	#define NPTS 15
	#define BEAM_WIDTH_LOOK	0.008552
	#define BEAM_WIDTH_AZIMUTH 0.007269

    double look_delta[NPTS+1];
    double azimuth_delta[NPTS+1];

	//
	// Setup for drawing an ellipse
	//

	double theta,r;
	double a2 = BEAM_WIDTH_LOOK*BEAM_WIDTH_LOOK;
	double b2 = BEAM_WIDTH_AZIMUTH*BEAM_WIDTH_AZIMUTH;
	for (int j=0; j <= NPTS; j++)
	{
		theta = j*two_pi/NPTS;
		r = sqrt(a2*b2/(a2*sin(theta)*sin(theta) + b2*cos(theta)*cos(theta)));
		look_delta[j] = r*cos(theta);
		azimuth_delta[j] = r*sin(theta);
	}

	//----------------------//
	// cycle through events //
	//----------------------//

	Event event;
	while (event.time < 12.00)
	{
		sim.DetermineNextEvent(&event);
		sim.SimulateEvent(&instrument, &event);
		int beam_idx;
		switch(event.eventId)
		{
		case Event::SCATTEROMETER_BEAM_A_MEASUREMENT:
			beam_idx = 0;
			break;
		case Event::SCATTEROMETER_BEAM_B_MEASUREMENT:
			beam_idx = 1;
			break;
		default:
			continue;
			break;
		}
		double ang = instrument.antenna.beam[beam_idx].azimuthAngle +
			instrument.antenna.azimuthAngle;
		ang = fmod(ang, two_pi);
		if (ang > 0.3491 && ang < 5.9341)
			continue;
        Spacecraft *sc = &(instrument.spacecraft);
		for (int i=0; i <= NPTS; i++)
		{
        	rlook_ant.SphericalSet(1.0,
            	instrument.antenna.beam[beam_idx].lookAngle + look_delta[i],
                instrument.antenna.beam[beam_idx].azimuthAngle +
				azimuth_delta[i] + instrument.antenna.azimuthAngle);
        	rspot = earth_intercept(sc->gcVector,sc->velocityVector,
                                sc_att,ant_att,rlook_ant);
        	rspot_geodetic = rspot.get_alt_lat_lon(EarthPosition::GEODETIC);
        	printf("%g %g ",rspot_geodetic.get(1),rspot_geodetic.get(2));
		}
       	printf("\n");
	}

	return (0);
}
