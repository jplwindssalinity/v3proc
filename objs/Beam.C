//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_beam_c[] =
	"@(#) $Id$";

#include "Beam.h"
#include "Array.h"

//======//
// Beam //
//======//

Beam::Beam()
:	polarization(NONE), timeOffset(0.0),
	_lookAngle(0.0), _azimuthAngle(0.0),
	_Nx(0), _Ny(0), _ix_zero(0), _iy_zero(0),
	_x_spacing(0.0), _y_spacing(0.0), _out_of_range_value(0.0),
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

//-----------------------//
// Beam::SetBeamGeometry //
//-----------------------//

int
Beam::SetBeamGeometry(
	double	look_angle,
	double	azimuth_angle)
{
	//------------------------//
	// copy passed parameters //
	//------------------------//

	_lookAngle = look_angle;
	_azimuthAngle = azimuth_angle;

	//----------------------------------------------------//
	// generate forward and reverse coordinate transforms //
	//----------------------------------------------------//

	Attitude beam_frame;
	beam_frame.Set(0.0, _lookAngle, _azimuthAngle, 3, 2, 1);

	_antFrameToBeamFrame.SetRotation(beam_frame);
	_beamFrameToAntFrame = _antFrameToBeamFrame.ReverseDirection();

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
	double x_spacing, double y_spacing, float **power_gain)
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

	// Some sanity checking.
	if ((_Nx <= 0) ||
		(_Ny <= 0) ||
		(_ix_zero < 0) ||
		(_iy_zero < 0) ||
		(_ix_zero > _Nx-1) ||
		(_iy_zero > _Ny-1))
    {
        return(0);
    }

	if (power_gain == NULL) return(0);
	_power_gain = power_gain;

	return(1);

}

//--------------------//
// Beam::GetPowerGain //
//--------------------//

//
// Bilinear interpolation of the measured antenna gain for this beam.
// The measured antenna pattern is stored in memory in the variable
// power_gain[Nx][Ny].  The first index refers to steps along the x-axis,
// and the second index refers to steps along the y-axis of the beam frame.
//
// Inputs:
//  unitx,unity = the x and y components of the unit vector that points in
//    the desired direction in the beam frame.  The units should be consistent
//    with the units of x_spacing and y_spacing.
//

double
Beam::GetPowerGain(
	double	unitx,
	double	unity)
{

	// Check for loaded pattern data.
	if (_power_gain == NULL)
	{	// show stopper
		printf("Error: No pattern data loaded for interpolation\n");
		exit(-1);
	}

	// Compute 2-D indices for the lower left point in the grid square around
	// the desired point.
	int ix1 = (int)(unitx/_x_spacing) + _ix_zero;
	int iy1 = (int)(unity/_y_spacing) + _iy_zero;

	if ((ix1 < 0) ||
    	(ix1 > _Nx - 2) ||
    	(iy1 < 0) ||
    	(iy1 > _Ny - 2))
	{
		return(_out_of_range_value);
	}

	// The actual location of the lower left point of the grid square.
	double x1 = (ix1 - _ix_zero)*_x_spacing;
	double y1 = (iy1 - _iy_zero)*_y_spacing;

	// The power gain at the four grid square points.
	double pg1 = _power_gain[ix1][iy1];
	double pg2 = _power_gain[ix1+1][iy1];
	double pg3 = _power_gain[ix1+1][iy1+1];
	double pg4 = _power_gain[ix1][iy1+1];

	// The proportional location of the requested point in the grid square.
	double t = (unitx - x1) / _x_spacing;
	double u = (unity - y1) / _y_spacing;

	// The interpolated power gain.
	return( (1-t)*(1-u)*pg1 + t*(1-u)*pg2 + t*u*pg3 + (1-t)*u*pg4 );

}

//-----------------------//
// Beam::ReadBeamPattern //
//-----------------------//

int
Beam::ReadBeamPattern(char* filename, double out_of_range_value)
{

	// Set the out of range value which is returned by GetPowerGain for any
	// requested points that lie outside the range covered by the beam pattern.
	_out_of_range_value = out_of_range_value;

    FILE* fp = fopen(filename,"r");
    if (fp == NULL) return(0);

	// Read header info which specifies the pattern size and spacing.
    if (fread(&_Nx, sizeof(int), 1, fp) != 1 ||
        fread(&_Ny, sizeof(int), 1, fp) != 1 ||
        fread(&_ix_zero, sizeof(int), 1, fp) != 1 ||
        fread(&_iy_zero, sizeof(int), 1, fp) != 1 ||
        fread(&_x_spacing, sizeof(double), 1, fp) != 1 ||
        fread(&_y_spacing, sizeof(double), 1, fp) != 1)
    {
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
        return(0);
    }

	// Allocate an array to hold the pattern.
	_power_gain = (float**)make_array(sizeof(float),2,_Nx,_Ny);
	if (_power_gain == NULL)
	{
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
       		return(0);
    	}
	}

    return(1);
}

//-----------------------//
// Beam::WriteBeamPattern //
//-----------------------//

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
        return(0);
    }

	if (_power_gain == NULL)
	{
		return(0);
	}

	// Write header info which specifies the pattern size and spacing.
    if (fwrite(&_Nx, sizeof(int), 1, fp) != 1 ||
        fwrite(&_Ny, sizeof(int), 1, fp) != 1 ||
        fwrite(&_ix_zero, sizeof(int), 1, fp) != 1 ||
        fwrite(&_iy_zero, sizeof(int), 1, fp) != 1 ||
        fwrite(&_x_spacing, sizeof(double), 1, fp) != 1 ||
        fwrite(&_y_spacing, sizeof(double), 1, fp) != 1)
    {
        return(0);
    }

	// Write out the pattern. (stored with x varying most rapidly)
	for (int j=0; j < _Ny; j++)
	for (int i=0; i < _Nx; i++)
	{
		if (fwrite(&(_power_gain[i][j]), sizeof(float), 1, fp) != 1)
    	{
       		return(0);
    	}
	}

    return(1);
}
