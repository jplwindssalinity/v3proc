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
    Vector3 rlook_beam;  // beam look direction in beam frame
    Vector3 rlook_geo;  // beam look direction in geocentric frame
    EarthPosition rsat;
	EarthPosition rspot_prev[2];	// holds last spot position for both beams
	EarthPosition rspot_look1,rspot_look2;
	EarthPosition rspot_azi1,rspot_azi2;
	Vector3 rlook_look1,rlook_look2;
	Vector3 rlook_azi1,rlook_azi2;

	#define NPTS 15
	#define BEAM_WIDTH_LOOK	0.008552
	#define BEAM_WIDTH_AZIMUTH 0.007269

	//
	// Open various output files.
	//

	FILE *checkfile = fopen("sim_spot.check","w");
	FILE *spotfile = fopen("spotdata.dat","w");
	FILE *cgsfile1 = fopen("cgshits1.dat","w");
	FILE *cgsfile2 = fopen("cgshits2.dat","w");
	if ((checkfile == NULL) || (spotfile == NULL) || (cgsfile1 == NULL) ||
		(cgsfile2 == NULL))
	{
		printf("Error opening output files\n");
		exit(-1);
	}

	//
	// Setup for drawing an ellipse. Output in the CGS hit files.
	//

	double theta[NPTS+1],phi[NPTS+1];
	double a2 = BEAM_WIDTH_LOOK*BEAM_WIDTH_LOOK;
	double b2 = BEAM_WIDTH_AZIMUTH*BEAM_WIDTH_AZIMUTH;
	for (int j=0; j <= NPTS; j++)
	{
		phi[j] = j*two_pi/NPTS;
		theta[j] = sqrt(a2*b2 /
			(a2*sin(phi[j])*sin(phi[j]) + b2*cos(phi[j])*cos(phi[j])));
		// CGS uses 1-way 3-db beam widths
		double phi_cgs = phi[j];
		double theta_cgs = sqrt(4*a2*b2 /
			(2*a2*sin(phi[j])*sin(phi[j]) + 2*b2*cos(phi[j])*cos(phi[j])));
		// convert to rectangular angle space for plotting by xmgr
		double beam_x = theta_cgs*sin(phi_cgs)*rtd;
		double beam_y = theta_cgs*cos(phi_cgs)*rtd;
		fprintf(cgsfile1,"%g %g\n",beam_x,beam_y);
		fprintf(cgsfile2,"%g %g\n",beam_x,beam_y);
	}
	fprintf(cgsfile1,"&\n");
	fprintf(cgsfile2,"&\n");

	//
	// Identify a ground location to study potential hits on a SeaWinds CGS.
	//

	EarthPosition rcgs(10*dtr,12*dtr,EarthPosition::GEODETIC);

	//----------------------//
	// cycle through events //
	//----------------------//

	WindField windfield;
	GMF gmf;

	Event event;
	while (event.time < 220.00)
	{
		sim.DetermineNextEvent(&event);
        sim.spacecraftSim.UpdateOrbit(event.time,
            &(instrument.spacecraft));
        sim.antennaSim.UpdatePosition(event.time,
            &(instrument.antenna));
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

        Spacecraft *sc = &(instrument.spacecraft);
		Antenna *antenna =&(instrument.antenna);
		Beam *beam = &(antenna->beam[beam_idx]);

		// Determine location of CGS in beam pattern, and output if
		// sufficiently close to the gain peak.
		rlook_beam = beam_look(sc->gcVector,sc->velocityVector,rcgs,
			sc_att,antenna->antennaFrame,beam->beamFrame);
		double r,theta_cgs,phi_cgs;
		rlook_beam.SphericalGet(&r,&theta_cgs,&phi_cgs);
		// convert to rectangular angle space for plotting by xmgr
		double beam_x = theta_cgs*sin(phi_cgs)*rtd;
		double beam_y = theta_cgs*cos(phi_cgs)*rtd;
		if (theta_cgs < 2*BEAM_WIDTH_LOOK)
		{
			if (beam_idx == 0) fprintf(cgsfile1,"%g %g\n",beam_x,beam_y);
			if (beam_idx == 1) fprintf(cgsfile2,"%g %g\n",beam_x,beam_y);
		}
		// check for correct reversal
       	rspot = earth_intercept(sc->gcVector,sc->velocityVector,
			sc_att,antenna->antennaFrame,beam->beamFrame,rlook_beam);
		double distance = rspot.surface_distance(rcgs);
		if (distance > 0.01)
		{
			printf("reversal check: distance = %g\n",distance);
		}

		double ang = instrument.antenna.beam[beam_idx].azimuthAngle +
			instrument.antenna.azimuthAngle;
		ang = fmod(ang, two_pi);
		if (ang > 0.3491 && ang < 5.9341)
			continue;

		// compute distance between successive spots (of same beam)
		rlook_beam.Set(0,0,1);
       	rspot = earth_intercept(sc->gcVector,sc->velocityVector,
			sc_att,antenna->antennaFrame,beam->beamFrame,rlook_beam);
		distance = rspot.surface_distance(rspot_prev[beam_idx]);

		// update previous spot position
		rspot_prev[beam_idx] = rspot;

		// compute size of ellipse on the surface
		rlook_beam.SphericalSet(1.0,BEAM_WIDTH_LOOK,0);
       	rspot_look1 = earth_intercept(sc->gcVector,sc->velocityVector,
			sc_att,antenna->antennaFrame,beam->beamFrame,rlook_beam);
		rlook_beam.SphericalSet(1.0,-BEAM_WIDTH_LOOK,0);
       	rspot_look2 = earth_intercept(sc->gcVector,sc->velocityVector,
			sc_att,antenna->antennaFrame,beam->beamFrame,rlook_beam);
		rlook_beam.SphericalSet(1.0,BEAM_WIDTH_AZIMUTH,90*dtr);
       	rspot_azi1 = earth_intercept(sc->gcVector,sc->velocityVector,
			sc_att,antenna->antennaFrame,beam->beamFrame,rlook_beam);
		rlook_beam.SphericalSet(1.0,-BEAM_WIDTH_AZIMUTH,90*dtr);
       	rspot_azi2 = earth_intercept(sc->gcVector,sc->velocityVector,
			sc_att,antenna->antennaFrame,beam->beamFrame,rlook_beam);

		double look_width = rspot_look1.surface_distance(rspot_look2);
		double azi_width = rspot_azi1.surface_distance(rspot_azi2);

		// check for consistency
		rlook_look1 = rspot_look1 - sc->gcVector;
		rlook_look2 = rspot_look2 - sc->gcVector;
		rlook_azi1 = rspot_azi1 - sc->gcVector;
		rlook_azi2 = rspot_azi2 - sc->gcVector;
		double bw_look = acos((rlook_look1 % rlook_look2) /
			rlook_look1.Magnitude() / rlook_look2.Magnitude());
		double bw_azi = acos((rlook_azi1 % rlook_azi2) /
			rlook_azi1.Magnitude() / rlook_azi2.Magnitude());
		
		fprintf(checkfile,"%d %g %g %g %g %g %g %g\n", beam_idx, event.time,
			180/pi*(instrument.antenna.beam[beam_idx].azimuthAngle +
			 	instrument.antenna.azimuthAngle),
			distance, look_width, azi_width, bw_look, bw_azi);

		for (int i=0; i <= NPTS; i++)
		{
			rlook_beam.SphericalSet(1.0,theta[i],phi[i]);
       		rspot = earth_intercept(sc->gcVector,sc->velocityVector,
				sc_att,antenna->antennaFrame,beam->beamFrame,rlook_beam);
        	rspot_geodetic = rspot.get_alt_lat_lon(EarthPosition::GEODETIC);
        	fprintf(spotfile,"%g %g ",
				rspot_geodetic.get(1),rspot_geodetic.get(2));
		}
       	fprintf(spotfile,"\n");
	}

	fclose(checkfile);
	fclose(spotfile);
	fclose(cgsfile1);
	fclose(cgsfile2);
	return (0);
}
