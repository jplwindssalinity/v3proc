//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		sim
//
// SYNOPSIS
//		sim <sim_config_file>
//
// DESCRIPTION
//		Simulates the SeaWinds 1b instrument based on the parameters
//		in the simulation configuration file.
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
#include <fcntl.h>
#include <math.h>
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "ConfigList.h"
#include "Spacecraft.h"
#include "ConfigSim.h"
#include "Array.h"
#include "Beam.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<Meas>;
template class List<LonLat>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
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

	//-----------------------------------------------//
	// create an instrument and instrument simulator //
	//-----------------------------------------------//

	Instrument instrument;
	if (! ConfigInstrument(&instrument, &config_list))
	{
		fprintf(stderr, "%s: error configuring instrument\n", command);
		exit(-1);
	}

	//--------------------------------------------//
	// Make and write beam patterns for each beam //
	//--------------------------------------------//

	int Nx = 512;
	int Ny = 512;
	int ix_zero = 256;
	int iy_zero = 256;
	double az_width = 1.19*dtr;
	double elev_width = 1.4*dtr;
	// Interpret x and y as azimuth and elevation angles.
	double x_width = az_width;
	double y_width = elev_width;
//	double x_width = 5.0*2.0*sin(az_width/2);
//	double y_width = 5.0*2.0*sin(elev_width/2);
	double x_spacing = x_width/Nx;
	double y_spacing = y_width/Ny;
	double max_gaindB = 30.0;
	double max_gain = pow(10,max_gaindB/10);

	for (int ib=0; ib < instrument.antenna.numberOfBeams; ib++)
	{
		Beam* cur_beam = &(instrument.antenna.beam[ib]);
		float** power_gain = (float**)make_array(sizeof(float),2,Nx,Ny);
		if (power_gain == NULL)
		{
			printf("Error allocating pattern array\n");
			exit(-1);
		}
		double r,x,y,theta,phi,Fn;
		for (int i=0; i < Nx; i++)
		for (int j=0; j < Ny; j++)
		{
			x = (i - ix_zero)*x_spacing;
			y = (j - iy_zero)*y_spacing;

			// Compute spherical angles assuming x and y are components of
			// the unit look vector in the beam frame.
			//theta = asin(sqrt(x*x + y*y));
			//phi = atan2(y,x);

			// Compute spherical angles assuming x and y are azimuth and
			// elevation angles.
			Vector3 ulook;
			ulook.AzimuthElevationSet(1.0,x,y);
			ulook.SphericalGet(&r,&theta,&phi);

			// compute normalized pattern with width elev_width in the x-z
			// plane, and width az_width in the y-z plane.
			// This pattern is for a rectangular aperture with uniform
			// illumination (ie., sinc^2).
			Fn = sin(pi*0.88/elev_width*sin(theta)*cos(phi)) /
					 (pi*0.88/elev_width*sin(theta)*cos(phi)) *
				 sin(pi*0.88/az_width*sin(theta)*sin(phi)) /
					 (pi*0.88/az_width*sin(theta)*sin(phi));
			Fn *= Fn;
			power_gain[i][j] = Fn*max_gain;
		}

		cur_beam->SetBeamPattern(Nx,Ny,ix_zero,iy_zero,x_spacing,y_spacing,
									power_gain);
		cur_beam->WriteBeamPattern("beampattern.dat");
	}

	return (0);
}
