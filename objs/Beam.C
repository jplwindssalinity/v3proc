//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_beam_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <math.h>
#include "Beam.h"
#include "Array.h"
#include "Constants.h"
#include "Attitude.h"

//======//
// Beam //
//======//

const char* beam_map[] = { "None", "V", "H" };

Beam::Beam()
:	polarization(NONE), pulseWidth(0.0), rxGateWidth(0.0),
	timeOffset(0.0), useRangeTracker(0), useDopplerTracker(0),
	_elecBoresightLook(0.0), _elecBoresightAzim(0.0),
	_electrical_boresight_Em(0.0), _electrical_boresight_Am(0.0), _Nx(0),
	_Ny(0), _ix_zero(0), _iy_zero(0), _x_spacing(0.0), _y_spacing(0.0),
	_power_gain(NULL)
{
	return;
}

Beam::~Beam()
{
	if (_power_gain != NULL)
	{
		free_array(_power_gain,2,_Nx,_Ny);
	}

	return;
}

//------------------------------//
// Beam::SetElectricalBoresight //
//------------------------------//

//
// This method orients a beam (already loaded with ReadBeamPattern) so that
// the electrical boresight points at the indicated look angle and azimuth
// angle in the antenna frame.  The beam pattern is stored in the beam
// reference frame (defined by the mechanical boresight).
//

int
Beam::SetElectricalBoresight(
	double	desired_electrical_look_angle,
	double	desired_electrical_azimuth_angle)
{
	//------------------------------------------------//
	// check to see if a beam pattern has been loaded //
	//------------------------------------------------//

	if (_power_gain == NULL)
	{
		printf("Error: SetElectricalBoresight found no loaded beam pattern\n");
 		return(0);
	}

	//--------------------------//
	// create coordinate switch //
	//--------------------------//

	_elecBoresightLook = desired_electrical_look_angle;
	_elecBoresightAzim = desired_electrical_azimuth_angle;

	CoordinateSwitch total;

	Attitude attitude;
	attitude.Set(0.0, _elecBoresightLook - pi / 2.0, _elecBoresightAzim,
		1, 2, 3);
	CoordinateSwitch antennaFrameToBoreFrame;
	antennaFrameToBoreFrame.SetRotation(attitude);

	CoordinateSwitch boreFrameToBeamFrame;
	attitude.Set(0.0, _electrical_boresight_Em, -_electrical_boresight_Am,
		1, 2, 3);
	boreFrameToBeamFrame.SetRotation(attitude);

	_antennaFrameToBeamFrame = antennaFrameToBoreFrame;
	_antennaFrameToBeamFrame.Append(&boreFrameToBeamFrame);

	return(1);
}

//------------------------------//
// Beam::GetElectricalBoresight //
//------------------------------//

//
// This method determines the electrical boresight in the antenna frame
// with standard spherical angles (called look_angle and azimuth_angle).
//

int
Beam::GetElectricalBoresight(
	double*		look_angle,
	double*		azimuth_angle)
{
	// Check to see if a beam pattern has been loaded.
	if (_power_gain == NULL)
	{
		printf("Error: GetElectricalBoresight found no loaded beam pattern\n");
 		return(0);
	}

	*look_angle = _elecBoresightLook;
	*azimuth_angle = _elecBoresightAzim;
	return(1);
}

//------------------------------//
// Beam::SetMechanicalBoresight //
//------------------------------//

//
// This method orients the beam (already loaded with ReadBeamPattern) so that
// the mechanical boresight points at the indicated look angle and azimuth
// angle in the antenna frame.  The electrical boresight is then determined
// by the beam pattern which was measured in the mechanical boresight frame.
// With this method, the two SeaWinds beams can be placed consistently by
// using the same mechanical boresight direction for each beam.
//

int
Beam::SetMechanicalBoresight(
	double	look_angle,
	double	azimuth_angle)
{
	// Check to see if a beam pattern has been loaded.
	if (_power_gain == NULL)
	{
		printf("Error: SetMechanicalBoresight found no loaded beam pattern\n");
 		return(0);
	}

	// dummy line
	look_angle = azimuth_angle;

	fprintf(stderr, "Beam::SetMechanicalBoresight is not available yet!\n");
	exit(1);

	return(1);
}

//----------------------//
// Beam::SetBeamPattern //
//----------------------//

// Note that a pointer to a previously allocated and filled pattern array
// is passed in and stored.  This function does not allocate its own array
// and copy the passed array.  Therefore, the caller should not explicitely
// deallocate the array.

int
Beam::SetBeamPattern(
	int	Nx, int Ny, int ix_zero, int iy_zero,
	double x_spacing, double y_spacing,
	double electrical_boresight_Em, double electrical_boresight_Am,
	float **power_gain)
{
	//------------------------//
	// copy passed parameters //
	//------------------------//

	_Nx = Nx;
	_Ny = Ny;
	_ix_zero = ix_zero;
	_iy_zero = iy_zero;
	_x_spacing = x_spacing;
	_y_spacing = y_spacing;
	_electrical_boresight_Em = electrical_boresight_Em;
	_electrical_boresight_Am = electrical_boresight_Am;

	// Some sanity checking.
	if ((_Nx <= 0) ||
		(_Ny <= 0) ||
		(_ix_zero < 0) ||
		(_iy_zero < 0) ||
		(_ix_zero > _Nx-1) ||
		(_iy_zero > _Ny-1))
    {
		printf("Invalid beam pattern header info in SetBeamPattern\n");
        return(0);
    }

	if (power_gain == NULL) return(0);
	_power_gain = power_gain;

	return(1);
}

//-----------------------//
// Beam::ReadBeamPattern //
//-----------------------//

int
Beam::ReadBeamPattern(char* filename)
{

	// Check for an existing pattern, and remove if needed.
	if (_power_gain != NULL)
	{
		free_array(_power_gain,2,_Nx,_Ny);
	}

    FILE* fp = fopen(filename,"r");
    if (fp == NULL) return(0);

	// Read header info which specifies the pattern size and spacing.
    if (fread(&_Nx, sizeof(int), 1, fp) != 1 ||
        fread(&_Ny, sizeof(int), 1, fp) != 1 ||
        fread(&_ix_zero, sizeof(int), 1, fp) != 1 ||
        fread(&_iy_zero, sizeof(int), 1, fp) != 1 ||
        fread(&_x_spacing, sizeof(double), 1, fp) != 1 ||
        fread(&_y_spacing, sizeof(double), 1, fp) != 1 ||
		fread(&_electrical_boresight_Em, sizeof(double), 1, fp) != 1 ||
		fread(&_electrical_boresight_Am, sizeof(double), 1, fp) != 1)
    {
		printf("Error reading beam pattern header info from %s\n",filename);
		fclose(fp);
        return(0);
    }

	// Some sanity checking.
	if ((_Nx <= 0) ||
		(_Ny <= 0) ||
		(_ix_zero < 0) ||
		(_iy_zero < 0) ||
		(_ix_zero > _Nx-1) ||
		(_iy_zero > _Ny-1))
    {
		printf("Invalid beam pattern header info in %s\n",filename);
		fclose(fp);
        return(0);
    }

	// Allocate an array to hold the pattern.
	_power_gain = (float**)make_array(sizeof(float),2,_Nx,_Ny);
	if (_power_gain == NULL)
	{
		printf("Can't allocate a pattern array\n");
		fclose(fp);
		return(0);
	}

	// Read in the pattern. (stored with x varying most rapidly)
	for (int j=0; j < _Ny; j++)
	for (int i=0; i < _Nx; i++)
	{
		if (fread(&(_power_gain[i][j]), sizeof(float), 1, fp) != 1)
    	{
			free_array(_power_gain,2,_Nx,_Ny);
			_power_gain = NULL;
			printf("Error reading pattern data from %s\n",filename);
			fclose(fp);
       		return(0);
    	}
	}

	fclose(fp);
    return(1);
}

//------------------------//
// Beam::WriteBeamPattern //
//------------------------//

int
Beam::WriteBeamPattern(char* filename)
{

    FILE* fp = fopen(filename,"w");
    if (fp == NULL) return(0);

	// Some sanity checking.
	if ((_Nx <= 0) ||
		(_Ny <= 0) ||
		(_ix_zero < 0) ||
		(_iy_zero < 0) ||
		(_ix_zero > _Nx-1) ||
		(_iy_zero > _Ny-1))
    {
		printf("Error in header parameters to be written to %s\n",filename);
		fclose(fp);
        return(0);
    }

	if (_power_gain == NULL)
	{
		printf("No pattern data to write to %s\n",filename);
		fclose(fp);
		return(0);
	}

	// Write header info which specifies the pattern size and spacing.
    if (fwrite(&_Nx, sizeof(int), 1, fp) != 1 ||
        fwrite(&_Ny, sizeof(int), 1, fp) != 1 ||
        fwrite(&_ix_zero, sizeof(int), 1, fp) != 1 ||
        fwrite(&_iy_zero, sizeof(int), 1, fp) != 1 ||
        fwrite(&_x_spacing, sizeof(double), 1, fp) != 1 ||
        fwrite(&_y_spacing, sizeof(double), 1, fp) != 1 ||
		fwrite(&_electrical_boresight_Em, sizeof(double), 1, fp) != 1 ||
		fwrite(&_electrical_boresight_Am, sizeof(double), 1, fp) != 1)
    {
		printf("Error writing header data to %s\n",filename);
		fclose(fp);
        return(0);
    }

	// Write out the pattern. (stored with x varying most rapidly)
	for (int j=0; j < _Ny; j++)
	for (int i=0; i < _Nx; i++)
	{
		if (fwrite(&(_power_gain[i][j]), sizeof(float), 1, fp) != 1)
    	{
			printf("Error writing pattern data to %s\n",filename);
			fclose(fp);
       		return(0);
    	}
	}

	fclose(fp);
    return(1);
}

//--------------------//
// Beam::GetPowerGain //
//--------------------//

//
// Bilinear interpolation of the measured antenna gain for this beam.
// The measured antenna pattern is stored in memory in the variable
// power_gain[Nx][Ny].  The first index refers to steps along elevation (Em),
// and the second index refers to steps along azimuth (Am) in the beam frame.
//
// Inputs:
//  look,azimuth = the orientation of the unit vector that points in
//    the desired direction in the antenna frame.
//	  The units should be consistent with the units of x_spacing and y_spacing.
//	gain = pointer to space for the interpolated power gain (real units).
//

int
Beam::GetPowerGain(
	double		look_angle,
	double		azimuth_angle,
	float*		gain)
{
	// Check for loaded pattern data.
	if (_power_gain == NULL)
	{	// show stopper
		printf("Error: No pattern data loaded for interpolation\n");
		exit(-1);
	}

	// Transform antenna frame angles to beam reference frame.
	Vector3 vector;
	vector.SphericalSet(1.0, look_angle, azimuth_angle);
	vector = _antennaFrameToBeamFrame.Forward(vector);
	double r, theta, phi;
	vector.SphericalGet(&r, &theta, &phi);
	double Em = pi / 2.0 - theta;
	double Am = phi;

	// Compute 2-D indices for the lower left point in the grid square around
	// the desired point.
	int ix1 = (int)floor(Em/_x_spacing) + _ix_zero;
	int iy1 = (int)floor(Am/_y_spacing) + _iy_zero;

	if ((ix1 < 0) ||
    	(ix1 > _Nx - 2) ||
    	(iy1 < 0) ||
    	(iy1 > _Ny - 2))
	{
		printf("GetPowerGain: requested point out of range (%g, %g)\n",
			look_angle, azimuth_angle);
		return(0);
	}

	// The actual location of the lower left point of the grid square.
	double x1 = (ix1 - _ix_zero) * _x_spacing;
	double y1 = (iy1 - _iy_zero) * _y_spacing;

	// The power gain at the four grid square points.
	double pg1 = _power_gain[ix1][iy1];
	double pg2 = _power_gain[ix1+1][iy1];
	double pg3 = _power_gain[ix1+1][iy1+1];
	double pg4 = _power_gain[ix1][iy1+1];

	// The proportional location of the requested point in the grid square.
	double t = (Em - x1) / _x_spacing;
	double u = (Am - y1) / _y_spacing;

	// The interpolated power gain.
	*gain = (1-t)*(1-u)*pg1 + t*(1-u)*pg2 + t*u*pg3 + (1-t)*u*pg4;
	return(1);
}

//--------------------//
// Beam::GetPowerGain //
//--------------------//
// same as above, but returns a double

int
Beam::GetPowerGain(
	double	look_angle,
	double	azimuth_angle,
	double	*gain)
{
	float x;
	if (! GetPowerGain(look_angle, azimuth_angle, &x))
		return(0);
	*gain = (double)x;
	return(1);
}

//---------------------------//
// Beam::GetPowerGainProduct //
//---------------------------//
// This method determines the power gain at the input look direction,
// and also at the look direction appropriate for the beam orientation
// after the antenna rotates during the pulse flight time.
// The two gains (transmit gain and receive gain) are multiplied together
// to yield a 2-way gain product with scan loss approximately included.
// Rotation is assumed to be in the positive azimuth direction.

int
Beam::GetPowerGainProduct(
	double	look_angle,
	double	azimuth_angle,
	double	round_trip_time,
	double	azimuth_rate,
	double	*gain_product)
{
	float xmit_gain;
	if (! GetPowerGain(look_angle, azimuth_angle, &xmit_gain))
		return(0);
	azimuth_angle -= azimuth_rate * round_trip_time;
	float recv_gain;
	if (! GetPowerGain(look_angle, azimuth_angle, &recv_gain))
		return(0);
	*gain_product = (double)(xmit_gain * recv_gain);
	return(1);
}

//
// Same as above, but returns a float
//

int
Beam::GetPowerGainProduct(
	double	look_angle,
	double	azimuth_angle,
	double	round_trip_time,
	double	azimuth_rate,
	float	*gain_product)
{
	double tmp;
	if (! GetPowerGainProduct(look_angle, azimuth_angle, round_trip_time,
		azimuth_rate, &tmp))
	{
		return(0);
	}
	*gain_product = (float)tmp;
	return(1);
}
