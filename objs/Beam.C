//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_beam_c[] =
    "@(#) $Id$";

#include <stdio.h>
#include <math.h>
#include "Beam.h"
#include "Array.h"
#include "Constants.h"
#include "Misc.h"

//======//
// Beam //
//======//

const char* beam_map[] = { "V", "H", "None" };

Beam::Beam()
:   polarization(NONE), _elecBoresightLook(0.0), _elecBoresightAzim(0.0),
    _electrical_boresight_Em(0.0), _electrical_boresight_Am(0.0), _Nx(0),
    _Ny(0), _ix_zero(0), _iy_zero(0), _x_spacing(0.0), _y_spacing(0.0),
    _power_gain(NULL), peakGain(0.0)
{
    return;
}

Beam::Beam(const Beam& from)
:   polarization(NONE), _elecBoresightLook(0.0), _elecBoresightAzim(0.0),
    _electrical_boresight_Em(0.0), _electrical_boresight_Am(0.0), _Nx(0),
    _Ny(0), _ix_zero(0), _iy_zero(0), _x_spacing(0.0), _y_spacing(0.0),
    _power_gain(NULL), peakGain(0.0)
{
    *this=from;
    return;
}

Beam::~Beam()
{
    if (_power_gain != NULL)
    {
        free_array(_power_gain, 2, _Nx, _Ny);
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
        fprintf(stderr,
            "Error: SetElectricalBoresight found no loaded beam pattern\n");
 		return(0);
	}

	//--------------------------//
	// create coordinate switch //
	//--------------------------//

	_elecBoresightLook = desired_electrical_look_angle;
	_elecBoresightAzim = desired_electrical_azimuth_angle;

	// Rotate so the x-axis of the antenna frame points along the boresight.
	Attitude attitude;
	attitude.Set(0.0, _elecBoresightLook - pi / 2.0, _elecBoresightAzim,
		1, 2, 3);
	CoordinateSwitch antennaFrameToBoreFrame;
	antennaFrameToBoreFrame.SetRotation(attitude);

	// Remember that positive Em deflections go from the x-axis towards
	// the z-axis of the bore frame.
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
        fprintf(stderr,
            "Error: GetElectricalBoresight found no loaded beam pattern\n");
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
// by the beam pattern which was measured in the mechanical boresight frame
// (ie., the beam frame).
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
		fprintf(stderr,
            "Error: SetMechanicalBoresight found no loaded beam pattern\n");
 		return(0);
	}

	//--------------------------//
	// create coordinate switch //
	//--------------------------//

	// Rotate so the x-axis of the antenna frame points along the boresight.
	Attitude attitude;
	attitude.Set(0.0, look_angle - pi / 2.0, azimuth_angle,
		1, 2, 3);
	_antennaFrameToBeamFrame.SetRotation(attitude);

	//--------------------------------//
	// Determine electrical boresight //
	//--------------------------------//

	// Point a vector along the electrical boresight in the beam frame
	Vector3 vector;
	vector.SphericalSet(1.0, pi/2.0 - _electrical_boresight_Em,
		_electrical_boresight_Am);

	// Transform to the antenna frame and get the spherical angles.
	vector = _antennaFrameToBeamFrame.Backward(vector);
	double r;
	vector.SphericalGet(&r, &_elecBoresightLook, &_elecBoresightAzim);

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
		fprintf(stderr,"Invalid beam pattern header info in SetBeamPattern\n");
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
		fprintf(stderr,"Error reading beam pattern header info from %s\n",filename);
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
		fprintf(stderr,"Invalid beam pattern header info in %s\n",filename);
		fclose(fp);
        return(0);
    }

	// Allocate an array to hold the pattern.
	_power_gain = (float**)make_array(sizeof(float),2,_Nx,_Ny);
	if (_power_gain == NULL)
	{
		fprintf(stderr,"Can't allocate a pattern array\n");
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
			fprintf(stderr,"Error reading pattern data from %s\n",filename);
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
		fprintf(stderr,"Error in header parameters to be written to %s\n",filename);
		fclose(fp);
        return(0);
    }

	if (_power_gain == NULL)
	{
		fprintf(stderr,"No pattern data to write to %s\n",filename);
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
		fprintf(stderr,"Error writing header data to %s\n",filename);
		fclose(fp);
        return(0);
    }

	// Write out the pattern. (stored with x varying most rapidly)
	for (int j=0; j < _Ny; j++)
	for (int i=0; i < _Nx; i++)
	{
		if (fwrite(&(_power_gain[i][j]), sizeof(float), 1, fp) != 1)
    	{
			fprintf(stderr,"Error writing pattern data to %s\n",filename);
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

//**********************************************************//
//        RETURN VALUES                                     //
//**********************************************************//
// Returns a 1 if it successfully retrieves a gain from     //
// the table.                                               //
//                                                          //
// Returns a 2 if the look and azimuth angle are for a      //
// location outside the domain of the tabulated gain values //
//                                                          //
// Returns a 0 for any other error condition                //
//                                                          //
// This is the general return structure used by a number    //
// of functions.                                            //
//***********************************************************/

int
Beam::GetPowerGain(
	double		look_angle,
	double		azimuth_angle,
	float*		gain)
{
	// Check for loaded pattern data.
	if (_power_gain == NULL)
	{	// show stopper
		fprintf(stderr,"Error: No pattern data loaded for interpolation\n");
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
		fprintf(stderr,"GetPowerGain: requested point out of range (%g, %g)\n",
			look_angle, azimuth_angle);
		*gain=0;
		return(2);
	}

	// The actual location of the lower left point of the grid square.
	double ax1 = (ix1 - _ix_zero) * _x_spacing;
	double ay1 = (iy1 - _iy_zero) * _y_spacing;

	// The power gain at the four grid square points.
	double pg1 = _power_gain[ix1][iy1];
	double pg2 = _power_gain[ix1+1][iy1];
	double pg3 = _power_gain[ix1+1][iy1+1];
	double pg4 = _power_gain[ix1][iy1+1];

	// The proportional location of the requested point in the grid square.
	double t = (Em - ax1) / _x_spacing;
	double u = (Am - ay1) / _y_spacing;

	// The interpolated power gain.
	*gain = (1-t)*(1-u)*pg1 + t*(1-u)*pg2 + t*u*pg3 + (1-t)*u*pg4;
        *gain*=peakGain;
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
	int retval=GetPowerGain(look_angle, azimuth_angle, &x);
	*gain = (double)x;
	return(retval);
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
//**********************************************************//
//        RETURN VALUES                                     //
//**********************************************************//
// Returns a 1 if it successfully retrieves a gain from     //
// the table.                                               //
//                                                          //
// Returns a 2 if the look and azimuth angle are for a      //
// location outside the domain of the tabulated gain values //
//                                                          //
// Returns a 0 for any other error condition                //
//                                                          //
// This is the general return structure used by a number    //
// of functions.                                            //
//**********************************************************//

int
Beam::GetPowerGainProduct(
	double	look_angle,
	double	azimuth_angle,
	double	round_trip_time,
	double	azimuth_rate,
	double	*gain_product)
{
	float xmit_gain;
	int retval1, retval2;
	retval1=GetPowerGain(look_angle, azimuth_angle, &xmit_gain);
	azimuth_angle -= azimuth_rate * round_trip_time;
	float recv_gain;
	retval2=GetPowerGain(look_angle, azimuth_angle, &recv_gain);
	*gain_product = (double)(xmit_gain * recv_gain);
        if(retval1==0 || retval2==0) return(0);
        if(retval1==2 || retval2==2) return(2);
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
        int retval;
        retval=GetPowerGainProduct(look_angle, azimuth_angle, round_trip_time,
		azimuth_rate, &tmp);
	*gain_product = (float)tmp;
	return(retval);
}

// Computes the Spatial Response
int
Beam::GetSpatialResponse(
	double	look_angle,
	double	azimuth_angle,
	double	round_trip_time,
	double	azimuth_rate,
	double	*response)
{
	double tmp;
        int retval;
        retval=GetPowerGainProduct(look_angle, azimuth_angle, round_trip_time,
		azimuth_rate, &tmp);

        // Calculate the range to spot on the ground.
	// This should be iterative but using the range
        // at the transmitted azimuth angle should be sufficient

        float range=round_trip_time*speed_light_kps*0.5;
	*response = (float)tmp/(range*range*range*range);
	return(retval);
}

//
// Same as above, but returns a float
//

int
Beam::GetSpatialResponse(
	double	look_angle,
	double	azimuth_angle,
	double	round_trip_time,
	double	azimuth_rate,
	float	*response)
{
	double tmp;
        int retval;
        retval=GetSpatialResponse(look_angle, azimuth_angle, round_trip_time,
		azimuth_rate, &tmp);
	*response = (float)tmp;
	return(retval);
}

        //------------------//
        // Operators        //
        //------------------//
Beam& 
Beam::operator=(
     const Beam& from)
{
        polarization=from.polarization;
	_elecBoresightLook=from._elecBoresightLook;
	_elecBoresightAzim=from._elecBoresightAzim;
        _electrical_boresight_Em=from._electrical_boresight_Em;
        _electrical_boresight_Am=from._electrical_boresight_Am;
	_antennaFrameToBeamFrame=from._antennaFrameToBeamFrame;
	if(_power_gain!=NULL) free_array((void*)_power_gain,2,_Nx,_Ny);
	_Nx=from._Nx;
	_Ny=from._Ny;
        _ix_zero=from._ix_zero;
        _iy_zero=from._iy_zero;
        _x_spacing=from._x_spacing;
        _y_spacing=from._y_spacing;
        peakGain=from.peakGain;
        if(from._power_gain==NULL) _power_gain=from._power_gain;
	else{
	  _power_gain = (float**)make_array(sizeof(float),2,_Nx,_Ny);
	  if (_power_gain == NULL)
	    {
	      fprintf(stderr,"Beam::operator=:Can't allocate a pattern array\n");
	      exit(1);
	    }
	  for(int i=0;i<_Nx;i++){
	    for(int j=0;j<_Ny;j++){
	      _power_gain[i][j]=from._power_gain[i][j];
	    }
	  }
	}
	return(*this);
}
