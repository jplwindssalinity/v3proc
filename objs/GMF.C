//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_gmf_c[] =
	"@(#) $Id$";

#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <math.h>
#include "GMF.h"
#include "Interpolate.h"
#include "Constants.h"
#include "Beam.h"

//=====//
// GMF //
//=====//

GMF::GMF()
{
	return;
}

GMF::~GMF()
{
	return;
}

//------------------//
// GMF:ReadOldStyle //
//------------------//

int GMF::ReadOldStyle(
	const char*		filename)
{
	int fd = open(filename, O_RDONLY);
	if (fd == -1)
		return(0);

	int dummy;
	read(fd, &dummy, sizeof(int));

	_polCount = 2;

	_incCount = 26;
	_incMin = 16.0 * dtr;
	_incMax = 66.0 * dtr;
	_incStep = 2.0 * dtr;

	_spdCount = 50;
	_spdMin = 1.0;
	_spdMax = 50.0;
	_spdStep = 1.0;

	int file_chi_count = 37;
	_chiCount = 72;
	_chiStep = two_pi / _chiCount;		// 5 degrees

	if (! _Allocate())
		return(0);

	float value;
	for (int pol_idx = 0; pol_idx < _polCount; pol_idx++)
	{
		for (int chi_idx = 0; chi_idx < file_chi_count; chi_idx++)
		{
			for (int spd_idx = 0; spd_idx < _spdCount; spd_idx++)
			{
				for (int inc_idx = 0; inc_idx < _incCount; inc_idx++)
				{
					if (read(fd, &value, sizeof(float)) != sizeof(float))
					{
						close(fd);
						return(0);
					}
					*(*(*(*(_value+pol_idx)+inc_idx)+spd_idx)+chi_idx) =
						(float)value;

					int chi_idx_2 = (_chiCount - chi_idx) % _chiCount;
					*(*(*(*(_value+pol_idx)+inc_idx)+spd_idx)+chi_idx_2) =
						(float)value;
				}
			}
		}
	}

	close(fd);
	return(1);
}

//---------------//
// GMF::GetCoefs //
//---------------//

int
GMF::GetCoefs(
	PolE		pol,
	float		inc,
	float		spd,
	float*		A0,
	float*		A1,
	float*		A1_phase,
	float*		A2,
	float*		A2_phase,
	float*		A3,
	float*		A3_phase,
	float*		A4,
	float*		A4_phase)
{
	float real[5], imag[5];
	int n = _chiCount - 1;
	float wn = two_pi / n;

	for (int i = 0; i < 5; i++)
	{
		real[i] = 0.0;
		imag[i] = 0.0;

		// assumes single point overlap in chi
		for (int chi_idx = 0; chi_idx < n; chi_idx++)
		{
			float arg = wn * (float)i * (float)chi_idx;
			float c = cos(arg);
			float s = sin(arg);
			float chi = (float)chi_idx * _chiStep;
			float val;
			GetInterpolatedValue(pol, inc, spd, chi, &val);
			real[i] += val * c;
			imag[i] += val * s;
		}
	}

	*A0 = real[0] / (float)n;
	*A1 = 2.0 * sqrt(real[1] * real[1] + imag[1] * imag[1]) / (float)n;
	*A1_phase = -atan2(imag[1], real[1]);
	*A2 = 2.0 * sqrt(real[2] * real[2] + imag[2] * imag[2]) / (float)n;
	*A2_phase = -atan2(imag[2], real[2]);
	*A3 = 2.0 * sqrt(real[3] * real[3] + imag[3] * imag[3]) / (float)n;
	*A3_phase = -atan2(imag[3], real[3]);
	*A4 = 2.0 * sqrt(real[4] * real[4] + imag[4] * imag[4]) / (float)n;
	*A4_phase = -atan2(imag[4], real[4]);

	return(1);
}

//--------------------------//
// GMF::WriteSolutionCurves //
//--------------------------//

int
GMF::WriteSolutionCurves(
	FILE*		ofp,
	MeasList*	meas_list,
	float		phi_step_size,
	float		phi_buffer,
	float		spd_tolerance,
	int			desired_solutions)
{
	//--------------------------//
	// refine the phi step size //
	//--------------------------//

	int phi_count = (int)(two_pi / phi_step_size + 0.5);
	phi_step_size = two_pi / (float)phi_count;

	//-------------------------//
	// allocate storage arrays //
	//-------------------------//

	float* best_spd = new float[phi_count];
	float* best_obj = new float[phi_count];

	//------------------------------//
	// generate each solution curve //
	//------------------------------//

	MeasList new_list;
	for (Meas* meas = meas_list->GetHead(); meas;
			meas = meas_list->GetNext())
	{
		new_list.Append(meas);
		SolutionCurve(&new_list, phi_count, phi_step_size, spd_tolerance,
			best_spd, best_obj);
		for (int i = 0; i < phi_count; i++)
		{
			fprintf(ofp, "%g %g\n", (float)i * phi_step_size * rtd,
				best_spd[i]);
		}
		fprintf(ofp, "&\n");
		new_list.GetHead();
		new_list.RemoveCurrent();
	}

	//----------------------------------//
	// generate combined solution curve //
	//----------------------------------//

	SolutionCurve(meas_list, phi_count, phi_step_size, spd_tolerance,
		best_spd, best_obj);
	float min_obj = best_obj[0];
	float max_obj = best_obj[0];
	for (int i = 0; i < phi_count; i++)
	{
		if (best_obj[i] < min_obj)
			min_obj = best_obj[i];
		if (best_obj[i] > max_obj)
			max_obj = best_obj[i];
	}
	float scale = 1.0 / (max_obj - min_obj);
	for (int i = 0; i < phi_count; i++)
	{
		fprintf(ofp, "%g %g %g\n", (float)i * phi_step_size * rtd,
			best_spd[i], (best_obj[i] - min_obj) * scale);
	}

	//--------------------------------------//
	// generate smoothed objective function //
	//--------------------------------------//

	WVC* wvc = new WVC();
	Smooth(phi_count, phi_step_size, phi_buffer, best_obj, desired_solutions);
	fprintf(ofp, "&\n");
	for (int i = 0; i < phi_count; i++)
	{
		fprintf(ofp, "%g %g\n", (float)i * phi_step_size * rtd,
			(best_obj[i] - min_obj) * scale);
	}

	//----------------------------//
	// write individual solutions //
	//----------------------------//

	for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
		wvp = wvc->ambiguities.GetNext())
	{
		fprintf(ofp, "&\n");
		fprintf(ofp, "%g %g\n", wvp->dir * rtd, wvp->spd);
	}

	//---------------//
	// delete things //
	//---------------//

	delete wvc;
	delete best_spd;
	delete best_obj;

	return(1);
}

//--------------------//
// GMF::RetrieveWinds //
//--------------------//
// phi_step_size is the step size for slices in the phi/spd space
// phi_buffer is the minimum angle between maxima and the widest
//  the smooth operation will average
// spd_tolerance is the tolerance for the speed maxima for each phi step
// desired_solutions is the maximum number of desired solutions

int
GMF::RetrieveWinds(
	MeasList*	meas_list,
	WVC*		wvc,
	float		phi_step_size,
	float		phi_buffer,
	float		spd_tolerance,
	int			desired_solutions)
{
	//--------------------------//
	// refine the phi step size //
	//--------------------------//

	int phi_count = (int)(two_pi / phi_step_size + 0.5);
	phi_step_size = two_pi / (float)phi_count;

	//-------------------------------------//
	// determine the buffer width in steps //
	//-------------------------------------//

	int phi_buffer_steps = (int)(phi_buffer / phi_step_size);

	//-------------------------//
	// allocate storage arrays //
	//-------------------------//

	float* best_spd = new float[phi_count];
	float* best_obj = new float[phi_count];

	//-------------------------//
	// find the solution curve //
	//-------------------------//

	if (! SolutionCurve(meas_list, phi_count, phi_step_size, spd_tolerance,
		best_spd, best_obj))
	{
		return(0);
	}

	//---------------------------//
	// smooth the solution curve //
	//---------------------------//

	if (! Smooth(phi_count, phi_step_size, phi_buffer_steps, best_obj,
		desired_solutions))
	{
		return(0);
	}

	//--------------------------------//
	// find maxima and add to the wvc //
	//--------------------------------//

	if (! FindMaxima(wvc, phi_count, phi_buffer_steps, best_spd, best_obj))
		return(0);

	delete best_spd;
	delete best_obj;

	//------------------------------------------//
	// sort the solutions by objective function //
	//------------------------------------------//

	wvc->SortByObj();

	return(1);
}

//--------------------//
// GMF::SolutionCurve //
//--------------------//
// For each of phi_count directions, find the speed (in steps of
// spd_step) that maximizes the objective function.  Fills in the best_spd
// and best_obj arrays


#define R	0.61803399
#define C	(1.0-R)

int
GMF::SolutionCurve(
	MeasList*	meas_list,
	int			phi_count,
	float		phi_step_size,
	float		spd_tolerance,
	float*		best_spd,
	float*		best_obj)
{
	//-------------------------------//
	// bracket maxima with certainty //
	//-------------------------------//

	float ax = _spdMin;
	float cx = _spdMax;

	//-----------------------//
	// for each direction... //
	//-----------------------//

	for (int phi_idx = 0; phi_idx < phi_count; phi_idx++)
	{
		float bx = ax + (cx - ax) * R;
		float phi = (float)phi_idx * phi_step_size;

		//------------------------------------------//
		// ...widen so that the maxima is bracketed //
		//------------------------------------------//

		if (_ObjectiveFunction(meas_list, bx, phi) <
			_ObjectiveFunction(meas_list, ax, phi) )
		{
			ax = _spdMin;
		}
		if (_ObjectiveFunction(meas_list, bx, phi) <
			_ObjectiveFunction(meas_list, cx, phi) )
		{
			cx = _spdMax;
		}

		//------------------------//
		// ...find the best speed //
		//------------------------//

		float x0, x1, x2, x3;
		x0 = ax;
		x3 = cx;
		if (cx - bx > bx - ax)
		{
			x1 = bx;
			x2 = bx + C * (cx - bx);
		}
		else
		{
			x2 = bx;
			x1 = bx - C * (bx - ax);
		}
		float f1 = _ObjectiveFunction(meas_list, x1, phi);
		float f2 = _ObjectiveFunction(meas_list, x2, phi);

		while (x3 - x0 > spd_tolerance)
		{
			if (f2 > f1)
			{
				x0 = x1;
				x1 = x2;
				x2 = x2 + C * (x3 - x2);
				f1 = f2;
				f2 = _ObjectiveFunction(meas_list, x2, phi);
			}
			else
			{
				x3 = x2;
				x2 = x1;
				x1 = x1 - C * (x1 - x0);
				f2 = f1;
				f1 = _ObjectiveFunction(meas_list, x1, phi);
			}
		} 

		if (f1 > f2)
		{
			best_spd[phi_idx] = x1;
			best_obj[phi_idx] = f1;
		}
		else
		{
			best_spd[phi_idx] = x2;
			best_obj[phi_idx] = f2;
		}
		ax = x1 - (x1 * 0.05);
		if (ax < _spdMin)
			ax = _spdMin;
		cx = x1 + (x2 * 0.05);
		if (cx > _spdMax)
			cx = _spdMax;
	}
	return(1);
}

//-------------//
// GMF::Smooth //
//-------------//
// Smooths the solution curve until the desired number (or less) of
// solutions are obtained.  Returns the number of solutions found.
// phi_buffer is one sided (i.e. 5 degrees means 5 degrees to either side)

int
GMF::Smooth(
	int			phi_count,
	float		phi_step_size,
	float		phi_buffer,
	float*		best_obj,
	int			desired_solutions)
{
	//---------------------------//
	// calculate some parameters //
	//---------------------------//

	int max_delta_phi = (int)(phi_buffer / phi_step_size + 0.5);
	int delta_phi = 0;		// start with no smoothing

	float* copy_obj = NULL;
	int return_value = 0;

	//----------------------------------------------------//
	// loop until desired number of solutions is obtained //
	//----------------------------------------------------//

	for (;;)
	{
		//-------------------------------//
		// count the number of solutions //
		//-------------------------------//

		int maxima_count = 0;
		int local_maxima_count = 0;
		for (int phi_idx = 0; phi_idx < phi_count; phi_idx++)
		{
			//--------------------------//
			// check for 3 point maxima //
			//--------------------------//

			int idx_minus = (phi_idx - 1 + phi_count) % phi_count;
			int idx_plus = (phi_idx + 1) % phi_count;
			if (best_obj[phi_idx] > best_obj[idx_minus] &&
				best_obj[phi_idx] < best_obj[idx_plus])
			{
				maxima_count++;

				//------------------------//
				// check for local maxima //
				//------------------------//

				int isa_max = 1;
				for (int offset = 2; offset <= max_delta_phi; offset++)
				{
					idx_minus = (phi_idx - offset + phi_count) % phi_count;
					idx_plus = (phi_idx + offset) % phi_count;
					if (best_obj[phi_idx] < best_obj[idx_plus] ||
						best_obj[phi_idx] < best_obj[idx_minus])
					{
						// not a maxima
						isa_max = 0;
						break;
					}
				}
				if (isa_max)
					local_maxima_count++;
			}
		}

		if (maxima_count == local_maxima_count &&
			maxima_count <= desired_solutions)
		{
			//----------------------------------//
			// done -- scale objective function //
			//----------------------------------//

			if (copy_obj)
			{
				// scaling was performed
				float scale = (float)(1 + delta_phi * 2);
				for (int phi_idx = 0; phi_idx < phi_count; phi_idx++)
				{
					best_obj[phi_idx] /= scale;
				}
			}
			return_value = 1;
			break;
		}
		else
		{
			//-----------------------------//
			// check if too much smoothing //
			//-----------------------------//

			delta_phi++;
			if (delta_phi > max_delta_phi)
				break;

			//--------//
			// smooth //
			//--------//

			if (copy_obj == NULL)
			{
				copy_obj = new float[phi_count];
				memcpy(copy_obj, best_obj, sizeof(float) * phi_count);
			}

			for (int phi_idx = 0; phi_idx < phi_count; phi_idx++)
			{
				int phi_idx_plus = (phi_idx + delta_phi) % phi_count;
				int phi_idx_minus = (phi_idx - delta_phi + phi_count) %
					phi_count;
				best_obj[phi_idx] += copy_obj[phi_idx_minus];
				best_obj[phi_idx] += copy_obj[phi_idx_plus];
			}
		}
	}
	if (copy_obj)
		delete copy_obj;
	return(return_value);
}

//-----------------//
// GMF::FindMaxima //
//-----------------//

int
GMF::FindMaxima(
	WVC*		wvc,
	int			phi_count,
	float		phi_step_size,
	float*		best_spd,
	float*		best_obj)
{
	for (int phi_idx = 0; phi_idx < phi_count; phi_idx++)
	{
		//--------------------------//
		// check for 3 point maxima //
		//--------------------------//

		int idx_minus = (phi_idx - 1 + phi_count) % phi_count;
		int idx_plus = (phi_idx + 1) % phi_count;
		if (best_obj[phi_idx] > best_obj[idx_minus] &&
			best_obj[phi_idx] < best_obj[idx_plus])
		{
			//------------//
			// add to wvc //
			//------------//

			WindVectorPlus* wvp = new WindVectorPlus();
			if (! wvp)
				return(0);
			wvp->spd = best_spd[phi_idx];
			wvp->dir = (float)phi_idx * phi_step_size;
			wvp->obj = best_obj[phi_idx];
			if (! wvc->ambiguities.Append(wvp))
			{
				delete wvp;
				return(0);
			}
		}
	}
	return(1);
}

//-------------------------//
// GMF::_ObjectiveFunction //
//-------------------------//

float
GMF::_ObjectiveFunction(
	MeasList*	meas_list,
	float		spd,
	float		phi)
{
	float fv = 0.0;
	for (Meas* meas = meas_list->GetHead(); meas;
			meas = meas_list->GetNext())
	{
		float chi = phi - meas->eastAzimuth + pi;
		float gmf_value;
		GetInterpolatedValue(meas->pol, meas->incidenceAngle, spd, chi,
			&gmf_value);
		float s = gmf_value - meas->value;
		fv += s*s / meas->estimatedKp + log10(meas->estimatedKp);
	}
	return(-fv);
}

