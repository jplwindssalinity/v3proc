//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		convert_pattern
//
// SYNOPSIS
//		convert_pattern <V pol pattern> <H pol pattern>
//
// DESCRIPTION
//		Reads in the SeaWinds antenna patterns and generates
//		simulation patterns.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//		<V pol pattern>		The pattern file for V-pol
//		<H pol pattern>		The pattern file for H-pol
//
// EXAMPLES
//		An example of a command line is:
//			% convert_pattern 5nl42917_Vpol.dat 5nk62922_Hpol.dat
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
#include "Constants.h"
#include "Attitude.h"
#include "CoordinateSwitch.h"
#include "Matrix3.h"
#include "Beam.h"
#include "Array.h"

//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

#define POINTS	260000
#define HPOL	0
#define VPOL	1
#define G0V		40.91
#define G0H		39.27

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int read_pattern(const char* filename, float* el, float* az, float* gain,
		int pol, int* number_read);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "<V pol pattern>", "<H pol pattern>",
	"<mech az>", "<mech el>", 0};

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
	if (argc != 5)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* v_pol_file = argv[clidx++];
	const char* h_pol_file = argv[clidx++];
	float mech_az = atof(argv[clidx++]) * dtr;
	float mech_el = atof(argv[clidx++]) * dtr;

	//----------------------//
	// read in the patterns //
	//----------------------//

	float v_el[POINTS], v_az[POINTS], v_gain[POINTS];
	int v_pol_count;
	if (! read_pattern(v_pol_file, v_el, v_az, v_gain, VPOL, &v_pol_count))
	{
		fprintf(stderr, "%s: error reading file %s\n", command, v_pol_file);
		exit(1);
	}

	float h_el[POINTS], h_az[POINTS], h_gain[POINTS];
	int h_pol_count;
	if (! read_pattern(h_pol_file, h_el, h_az, h_gain, HPOL, &h_pol_count))
	{
		fprintf(stderr, "%s: error reading file %s\n", command, h_pol_file);
		exit(1);
	}

	//----------------------------------//
	// set up coordinate transformation //
	//----------------------------------//

	//
	// The measurement frame is the coordinate system used to measure the
	// antenna pattern.  It is referenced here to the antenna frame.
	// Note that we are assuming that the dish will be mounted as shown in
	// figure 2 of memo 3347-97-010 (S. Lou and M. Spencer) with positive
	// elevation (as defined in same memo) causing deflection downwards
	// towards the s/c nadir pointing z-axis.  It is possible that the
	// mounting will be rolled away from this position.  Also, we assume
	// that the Xm axis (boresight) lies in the s/c x-z plane.
	// Azimuth mounting errors could change this.
	//

	Attitude measurement_attitude;
	measurement_attitude.Set(0.0, mech_el, mech_az, 3, 2, 1);
	CoordinateSwitch ant_to_meas(measurement_attitude);
	CoordinateSwitch meas_to_ant = ant_to_meas.ReverseDirection();

	//--------------//
	// locate peaks //
	//--------------//

	float max_v_gain = -99.0;
	float max_v_az = 0.0;
	float max_v_el = 0.0;
	for (int i = 0; i < v_pol_count; i++)
	{
		if (v_gain[i] > max_v_gain)
		{
			max_v_gain = v_gain[i];
			max_v_az = v_az[i];
			max_v_el = v_el[i];
		}
	}

	float max_h_gain = -99.0;
	float max_h_az = 0.0;
	float max_h_el = 0.0;
	for (int i = 0; i < h_pol_count; i++)
	{
		if (h_gain[i] > max_h_gain)
		{
			max_h_gain = h_gain[i];
			max_h_az = h_az[i];
			max_h_el = h_el[i];
		}
	}

	printf("%g %g %g\n", max_v_az * rtd, max_v_el * rtd, max_v_gain);
	printf("%g %g %g\n", max_h_az * rtd, max_h_el * rtd, max_h_gain);

	//
	// The desired beam frames have z-axes that point in the direction of the
	// peak gains.  We form these vectors here.
	//

	Vector3 max_v,max_h;
	max_v.SphericalSet(1.0, pi/2 - max_v_el, max_v_az);
	max_h.SphericalSet(1.0, pi/2 - max_h_el, max_h_az);
	Vector3 z_v = meas_to_ant.Forward(max_v);
	Vector3 z_h = meas_to_ant.Forward(max_h);

	//
	// The x and y axes of the beam frames are defined by yawing and
	// pitching the antenna frame (in that order) until the z-axis is correct.
	// Mounting errors may require additional rotation terms.
	//

	double r,theta,phi;

	z_v.SphericalGet(&r,&theta,&phi);
	Attitude beamv_frame;
	beamv_frame.Set(0.0,theta,phi,3,2,1);

	z_h.SphericalGet(&r,&theta,&phi);
	Attitude beamh_frame;
	beamh_frame.Set(0.0,theta,phi,3,2,1);

	// Define coordinate switches.
	CoordinateSwitch ant_to_beamv(beamv_frame);
	CoordinateSwitch ant_to_beamh(beamh_frame);

	//
	// Now we form a combined coordinate switch that will take a vector
	// from the measurement frame into the corresponding beam frame.
	//

	CoordinateSwitch meas_to_beamv = meas_to_ant;
    meas_to_beamv.Append(&ant_to_beamv);
	CoordinateSwitch meas_to_beamh = meas_to_ant;
    meas_to_beamh.Append(&ant_to_beamh);
	
	CoordinateSwitch beamv_to_meas = meas_to_beamv.ReverseDirection();
	CoordinateSwitch beamh_to_meas = meas_to_beamh.ReverseDirection();

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
	// angles as defined for the SeaWinds-1B simulator (see Matrix3.C).
	// For now, the two beam patterns have the same parameters.
	//

	double x_width = 10 * dtr;
	double y_width = 10 * dtr;
	double x_spacing = 0.01 * dtr;
	double y_spacing = 0.01 * dtr;
	int Nx = (int) (x_width / x_spacing);
	int Ny = (int) (y_width / y_spacing);
	// Set the zero point in the middle of the specified widths.
	int ix_zero = Nx/2;
	int iy_zero = Ny/2;

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
	// Bilinearly interpolate the measured gain patterns onto the specified
	// beam patterns.  To ensure an accurate reproduction of the beam patterns
	// in the new coordinate systems, the beam pattern arrays should be spaced
	// more closely than the measurements were.
	//

    for (int i=0; i < Nx; i++)
    for (int j=0; j < Ny; j++)
    {
        double x = (i - ix_zero)*x_spacing;
        double y = (j - iy_zero)*y_spacing;

        Vector3 ulook_beam;
        ulook_beam.AzimuthElevationSet(1.0,x,y);

		//
		// V-pol (Bilinear interpolation)
		//

		{	// separate block of code dealing with V-pol

		// Form desired look vector in measurement frame.
		Vector3 ulook_meas_v = beamv_to_meas.Forward(ulook_beam);

		// Convert the look vector to the modified spherical coordinates
		// used in the measurements.
		ulook_meas_v.SphericalGet(&r,&theta,&phi);
		double Em = pi/2 - theta;

	    // Compute 2-D indices for the lower left point in the grid square
		// around the desired point.
	    int ix1 = (int)(Em/xm_spacing) + ixm_zero;
   		int iy1 = (int)(phi/ym_spacing) + iym_zero;

	    if ((ix1 < 0) ||
   	     (ix1 > Nxm - 2) ||
   	     (iy1 < 0) ||
   	     (iy1 > Nym - 2))
   		{
			printf("Error: out of range interpolation\n");
			exit(-1);
   		}

	    // The actual location of the lower left point of the grid square.
   	 	double x1 = (ix1 - ixm_zero)*xm_spacing;
   	 	double y1 = (iy1 - iym_zero)*ym_spacing;

    	// The power gain at the four grid square points.
    	double pg1 = v_gain[ix1*Nym + iy1];
    	double pg2 = v_gain[(ix1+1)*Nym + iy1];
    	double pg3 = v_gain[(ix1+1)*Nym + iy1+1];
    	double pg4 = v_gain[ix1*Nym + iy1+1];

    	// The proportional location of the requested point in the grid square.
    	double t = (Em - x1) / xm_spacing;
    	double u = (phi - y1) / ym_spacing;

    	// The interpolated power gain.
		power_gainv[i][j] = (1-t)*(1-u)*pg1+t*(1-u)*pg2+t*u*pg3+(1-t)*u*pg4;

		// Add the peak gain value to the normalized pattern value.
		power_gainv[i][j] += G0V;

		// Convert to real gain values.
		power_gainv[i][j] = pow(power_gainv[i][j]/10.0,10.0);

		}

		//
		// H-pol (Bilinear interpolation)
		//

		{	// separate block of code dealing with H-pol

		// Form desired look vector in measurement frame.
		Vector3 ulook_meas_h = beamh_to_meas.Forward(ulook_beam);

		// Convert the look vector to the modified spherical coordinates
		// used in the measurements.
		ulook_meas_h.SphericalGet(&r,&theta,&phi);
		double Em = pi/2 - theta;

	    // Compute 2-D indices for the lower left point in the grid square
		// around the desired point.
	    int ix1 = (int)(Em/xm_spacing) + ixm_zero;
   		int iy1 = (int)(phi/ym_spacing) + iym_zero;

	    if ((ix1 < 0) ||
   	     (ix1 > Nxm - 2) ||
   	     (iy1 < 0) ||
   	     (iy1 > Nym - 2))
   		{
			printf("Error: out of range interpolation\n");
			exit(-1);
   		}

	    // The actual location of the lower left point of the grid square.
   	 	double x1 = (ix1 - ixm_zero)*xm_spacing;
   	 	double y1 = (iy1 - iym_zero)*ym_spacing;

    	// The power gain at the four grid square points.
    	double pg1 = h_gain[ix1*Nym + iy1];
    	double pg2 = h_gain[(ix1+1)*Nym + iy1];
    	double pg3 = h_gain[(ix1+1)*Nym + iy1+1];
    	double pg4 = h_gain[ix1*Nym + iy1+1];

    	// The proportional location of the requested point in the grid square.
    	double t = (Em - x1) / xm_spacing;
    	double u = (phi - y1) / ym_spacing;

    	// The interpolated power gain.
		power_gainh[i][j] = (1-t)*(1-u)*pg1+t*(1-u)*pg2+t*u*pg3+(1-t)*u*pg4;

		// Add the peak gain value to the normalized pattern value.
		power_gainh[i][j] += G0H;

		// Convert to real gain values.
		power_gainh[i][j] = pow(power_gainh[i][j]/10.0,10.0);

		}
	}

	Beam beamv;
	beamv.SetBeamPattern(Nx,Ny,ix_zero,iy_zero,x_spacing,y_spacing,
								power_gainv);
	beamv.WriteBeamPattern("beamv.dat");

	Beam beamh;
	beamh.SetBeamPattern(Nx,Ny,ix_zero,iy_zero,x_spacing,y_spacing,
								power_gainh);
	beamh.WriteBeamPattern("beamh.dat");
	
	return (0);
}

//--------------//
// read_pattern //
//--------------//

int
read_pattern(
	const char*		filename,
	float*			el,
	float*			az,
	float*			gain,
	int				pol,
	int*			number_read)
{
	printf("Reading %s...\n", filename);
	FILE* fp = fopen(filename, "r");
	if (fp == NULL)
		return(0);

	char line[1024];

	// skip first two lines
	fgets(line, 1024, fp);
	fgets(line, 1024, fp);

	float elev, azim, efield[2];

	int idx = 0;
	while (fgets(line, 1024, fp) == line)
	{
		if (sscanf(line, " %g %g %g %g", &elev, &azim, &(efield[0]),
			&(efield[1])) != 4)
		{
			break;
		}
		el[idx] = elev * dtr;
		az[idx] = azim * dtr;
		gain[idx] = efield[pol];
		idx++;
	}

	*number_read = idx;

	int retval;
	if (feof(fp))
		retval = 1;
	else
		retval = 0;

	fclose(fp);

	printf("  Done.\n");

	return(retval);
}
