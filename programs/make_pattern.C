//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

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
#include "Tracking.h"
#include "Tracking.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<WindVectorPlus>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

//-----------//
// CONSTANTS //
//-----------//

#define AZ_WIDTH	1.6
#define ELEV_WIDTH	1.8

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
	if (! config_list.Read(config_file))
	{
		fprintf(stderr, "%s: error reading sim config file %s\n",
			command, config_file);
		exit(1);
	}

	//---------------------------------------------------//
	// Make and write a beam pattern (use with each beam) //
	//---------------------------------------------------//

    // Parameters defining the structure of the gain measurements.
    int Nxm = 647;
    int Nym = 401;
    double xm_spacing = 0.031 * dtr;
    double ym_spacing = 0.05 * dtr;
    int ixm_zero = 324;
    int iym_zero = 201;

    //
    // Setup the parameters and arrays needed for the beam patterns.
    // The width's and spacings are specified for azimuth and elevation
    // angles as defined for the SeaWinds-1A measured antenna patterns.
    // For now, the two beam patterns have the same parameters, and
    // match the parameters of the measured gains.
    //

    double x_spacing = xm_spacing;
    double y_spacing = ym_spacing;
    int Nx = Nxm;
    int Ny = Nym;
    int ix_zero = ixm_zero;
    int iy_zero = iym_zero;

    float** power_gainv = (float**)make_array(sizeof(float),2,Nx,Ny);
    if (power_gainv == NULL)
    {
        printf("Error allocating v-pol pattern array\n");
        exit(-1);
    }
    float** power_gainh = (float**)make_array(sizeof(float),2,Nx,Ny);
    if (power_gainh == NULL)
    {
        printf("Error allocating h-pol pattern array\n");
        exit(-1);
    }

    //
	// Synthesize pattern data using a sinc^2 (rectangular aperture with
	// uniform illumination) and the appropriate beam widths.
    // Also scale by the peak pattern values.
	// See page 125 in Microwave Remote Sensing, vol 1, Ulaby, Moore, and Fung.
    //

	double az_width = AZ_WIDTH*dtr;
	double elev_width = ELEV_WIDTH*dtr;
	double max_gainV = pow(10.0, G0V/10.0);
	double max_gainH = pow(10.0, G0H/10.0);

    for (int i=0; i < Nx; i++)
    for (int j=0; j < Ny; j++)
    {
        double Em = (i - ix_zero)*x_spacing;
        double Am = (j - iy_zero)*y_spacing;
		Vector3 ulook;
		ulook.SphericalSet(1.0,pi/2.0-Em,Am);
		Vector3 xhat(1.0,0.0,0.0);
		double theta = acos(ulook % xhat);
		double phi;
		if ((ulook.Get(2) == 0.0) && (ulook.Get(1) >= 0.0))
		{
			phi = 0.0;
		}
		else if ((ulook.Get(2) == 0.0) && (ulook.Get(1) < 0.0))
		{
			phi = pi;
		}
		else
		{
			phi = atan2(ulook.Get(1),ulook.Get(2));
		}

		double Fn1,Fn2;
		if ((sin(theta) == 0.0) || (cos(phi) == 0.0))
		{
			Fn1 = 1.0;
		}	
		else
		{
			Fn1 = sin(pi*0.88/elev_width*sin(theta)*cos(phi)) /
				 (pi*0.88/elev_width*sin(theta)*cos(phi));
		}

		if ((sin(theta) == 0.0) || (sin(phi) == 0.0))
		{
			Fn2 = 1.0;
		}	
		else
		{
			Fn2 = sin(pi*0.88/az_width*sin(theta)*sin(phi)) /
				 (pi*0.88/az_width*sin(theta)*sin(phi));
		}

		double Fn = Fn1 * Fn2;
		Fn *= Fn;
		power_gainv[i][j] = Fn*max_gainV;
		power_gainh[i][j] = Fn*max_gainH;
    }

    //
    // Make beam objects and write them out.
    //

    Beam beamv;
    beamv.SetBeamPattern(Nx,Ny,ix_zero,iy_zero,x_spacing,y_spacing,
                                0.0,0.0,power_gainv);
    beamv.WriteBeamPattern("beam2.pat");

    Beam beamh;
    beamh.SetBeamPattern(Nx,Ny,ix_zero,iy_zero,x_spacing,y_spacing,
                                0.0,0.0,power_gainh);
    beamh.WriteBeamPattern("beam1.pat");

	return (0);
}
