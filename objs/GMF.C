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

//----------------------------//
// GMF::GenerateSolutionCurve //
//----------------------------//

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

//--------------------//
// GMF::FindSolutions //
//--------------------//

int
GMF::FindSolutions(
	MeasList*	meas_list,
	WVC*		wvc,
	float		spd_step,
	float		phi_step)		// in radians
{
	//---------------------------------------//
	// determine index ranges and step sizes //
	//---------------------------------------//

	int phi_count = (int)(two_pi / phi_step + 0.5);
	float dphi = two_pi / (float)phi_count;

	int spd_count = (int)(_spdMax / spd_step + 0.5);
	float dspd = _spdMax / (float)spd_count;

	//-------------------------//
	// allocate storage arrays //
	//-------------------------//

	float* best_spd = new float[phi_count];
	float* best_obj = new float[phi_count];

	//-------------------------//
	// find the solution curve //
	//-------------------------//

	_FindSolutionCurve(meas_list, dspd, dphi, phi_count, best_spd, best_obj);

	//--------------------------------//
	// find local maxima in phi strip //
	//--------------------------------//

	int count = 0;
	for (int phi_idx = 0; phi_idx < phi_count; phi_idx++)
	{
		int phi_idx_plus = (phi_idx + 1) % phi_count;
		int phi_idx_minus = (phi_idx - 1 + phi_count) % phi_count;
		if (best_obj[phi_idx] > best_obj[phi_idx_plus] &&
			best_obj[phi_idx] > best_obj[phi_idx_minus])
		{
			// maximum found -> add to list
			WindVectorPlus* new_wvp = new WindVectorPlus();
			if (! new_wvp)
				return(0);
			new_wvp->SetSpdDir(best_spd[phi_idx], phi_idx * dphi);
			if (! wvc->ambiguities.Append(new_wvp))
			{
				delete best_spd;
				delete best_obj;
				return(0);
			}
			count++;
		}
	}

	delete best_spd;
	delete best_obj;
	return(count);
}

//----------------------//
// GMF::RefineSolutions //
//----------------------//

enum { DOWN = 0, UP };

int
GMF::RefineSolutions(
	MeasList*	meas_list,
	WVC*		wvc,
	float		initial_spd_step,
	float		initial_phi_step,
	float		final_spd_step,
	float		final_phi_step)
{
	//----------------------//
	// for each solution... //
	//----------------------//

	for (WindVectorPlus* wvp = wvc->ambiguities.GetHead(); wvp;
		wvp = wvc->ambiguities.GetNext())
	{
		//--------------------------------------//
		// start with initial search resolution //
		//--------------------------------------//

		float spd_step = initial_spd_step;
		float phi_step = initial_phi_step;

		//--------------------------------------------//
		// quantize speed and direction to step sizes //
		//--------------------------------------------//

		int spd_idx = (int)(wvp->spd / spd_step + 0.5);
		wvp->spd = (float)spd_idx * spd_step;

		int phi_idx = (int)(wvp->dir / phi_step + 0.5);
		wvp->dir = (float)phi_idx * phi_step;

		//-----------------------------------------------------//
		// search until both step sizes are sufficiently small //
		//-----------------------------------------------------//

		while (spd_step > final_spd_step || phi_step > final_phi_step)
		{
			//-------------------------------------------------//
			// find maximum objective function for 9-neighbors //
			//-------------------------------------------------//

			int max_dspd = 0;
			int max_dphi = 0;
			float max_obj = -9e99;		// I hope this is low enough <g>
			for (int dspd = -1; dspd <= 1; dspd++)
			{
				for (int dphi = -1; dphi <= 1; dphi++)
				{
					float obj = _ObjectiveFunction(meas_list, wvp->spd +
						dspd * spd_step, wvp->dir + dphi * phi_step);
					if (obj > max_obj)
					{
						max_obj = obj;
						max_dspd = dspd;
						max_dphi = dphi;
					}
				}
			}

			//--------------------------------//
			// reduce step sizes if necessary //
			//--------------------------------//

			if (max_dspd == -1 && wvp->spd <= _spdMin)
				max_dspd = 0;
			if (max_dspd == 1 && wvp->spd >= _spdMax)
				max_dspd = 0;

			if (max_dspd == 0 && max_dphi == 0)
			{
				if (spd_step > final_spd_step)
					spd_step /= 2.0;
				if (spd_step < final_spd_step)
					spd_step = final_spd_step;

				if (phi_step > final_phi_step)
					phi_step /= 2.0;
				if (phi_step < final_phi_step)
					phi_step = final_phi_step;
			}

			//-----------------------------------------//
			// update speed and direction for solution //
			//-----------------------------------------//

			spd_idx = (int)(wvp->spd / spd_step + 0.5) + max_dspd;
			phi_idx = (int)(wvp->dir / phi_step + 0.5) + max_dphi;
			if (phi_idx < 0)
				phi_idx = (int)((wvp->dir + pi) / phi_step + 0.5) + max_dphi;

			wvp->spd = (float)spd_idx * spd_step;
			wvp->dir = (float)phi_idx * phi_step;
			wvp->obj = max_obj;
		}
	}

	return(1);
}

//----------------//
// GMF::ModCurves //
//----------------//

int
GMF::ModCurves(
	FILE*		ofp,
	MeasList*	meas_list,
	float		spd_step,
	float		phi_step)
{
	//---------------------------------------//
	// determine index ranges and step sizes //
	//---------------------------------------//

	int phi_count = (int)(two_pi / phi_step + 0.5);
	float dphi = two_pi / (float)phi_count;

	int spd_count = (int)(_spdMax / spd_step + 0.5);
	float dspd = _spdMax / (float)spd_count;

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
		SolutionCurve(&new_list, dspd, phi_count, best_spd, best_obj);
		for (int i = 0; i < phi_count; i++)
		{
			fprintf(ofp, "%g %g\n", (float)i * dphi * rtd, best_spd[i]);
		}
		fprintf(ofp, "&\n");
		new_list.GetHead();
		new_list.RemoveCurrent();
	}

	//----------------------------------//
	// generate combined solution curve //
	//----------------------------------//

	SolutionCurve(meas_list, dspd, phi_count, best_spd, best_obj);
	for (int i = 0; i < phi_count; i++)
	{
		fprintf(ofp, "%g %g %g\n", (float)i * dphi * rtd,
			best_spd[i], best_obj[i] * 100000.0 + 5.0);
	}

	//-----------------------//
	// delete storage arrays //
	//-----------------------//

	delete best_spd;
	delete best_obj;

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

//-------------------------//
// GMF::_FindSolutionCurve //
//-------------------------//
// For each direction, find the speed that maximizes the objective function

#define INITIAL_SPEED		8.0

int
GMF::_FindSolutionCurve(
	MeasList*	meas_list,
	float		dspd,
	float		dphi,
	int			phi_count,
	float*		best_spd,
	float*		best_obj)
{
	int spd_idx = (int)(INITIAL_SPEED / dspd + 0.5);
	float spd = (float)spd_idx * dspd;
	int max_spd_idx = (int)(_spdMax / dspd);
	int min_spd_idx = (int)(_spdMin / dspd);

	for (int phi_idx = 0; phi_idx < phi_count; phi_idx++)
	{
		//------------------------//
		// ...find the best speed //
		//------------------------//

		float phi = (float)phi_idx * dphi;
		float obj = _ObjectiveFunction(meas_list, spd, phi);
		float obj_minus = _ObjectiveFunction(meas_list, spd - dspd,
			phi);
		float obj_plus = _ObjectiveFunction(meas_list, spd + dspd,
			phi);
		do
		{
			if (obj > obj_minus && obj > obj_plus)
			{
				// peak
				best_spd[phi_idx] = spd;
				best_obj[phi_idx] = obj;
				break;
			}
			else if (obj_plus >= obj && obj_plus >= obj_minus)
			{
				// move up
				spd_idx++;
				if (spd_idx > max_spd_idx)
				{
					spd_idx = max_spd_idx;
					spd = (float)spd_idx * dspd;
					best_spd[phi_idx] = spd;
					best_obj[phi_idx] = _ObjectiveFunction(meas_list,
						spd, phi);
					break;
				}
				spd = (float)spd_idx * dspd;

				obj_minus = obj;
				obj = obj_plus;
				obj_plus = _ObjectiveFunction(meas_list, spd + dspd,
					phi);
			}
			else if (obj_minus >= obj && obj_minus >= obj_plus)
			{
				// move down
				spd_idx--;
				if (spd_idx < min_spd_idx)
				{
					spd_idx = min_spd_idx;
					spd = (float)spd_idx * dspd;
					best_spd[phi_idx] = spd;
					best_obj[phi_idx] = _ObjectiveFunction(meas_list,
						spd, phi);
					break;
				}

				spd = (float)spd_idx * dspd;
				obj_plus = obj;
				obj = obj_minus;
				obj_minus = _ObjectiveFunction(meas_list, spd - dspd,
					phi);
			}
			else
			{
				fprintf(stderr,
					"GMF::_FindSolutionCurve: unexpected solution curve\n");
				// treat like a peak
				best_spd[phi_idx] = spd;
				best_obj[phi_idx] = obj;
				break;
			}
		} while (1);
	}
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
#define SHFT2(a,b,c)		(a)=(b);(b)=(c)
#define SHFT3(a,b,c,d)		(a)=(b);(b)=(c);(c)=(d)

int
GMF::SolutionCurve(
	MeasList*	meas_list,
	float		spd_step,
	int			phi_count,
	float*		best_spd,
	float*		best_obj)
{
	//------------------------------------//
	// initialize tolerance and step size //
	//------------------------------------//

	float spd_tol = spd_step / 2.0;
	float phi_step = two_pi / (float)phi_count;

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
		float phi = (float)phi_idx * phi_step;

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

		while (x3 - x0 > spd_tol)
		{
			if (f2 > f1)
			{
				float d = R*x2 + C*x3;
				SHFT3(x0, x1, x2, d);
				d = _ObjectiveFunction(meas_list, x2, phi);
				SHFT2(f1, f2, d);
			}
			else
			{
				float d = R*x1 + C*x0;
				SHFT3(x3, x2, x1, d);
				d = _ObjectiveFunction(meas_list, x1, phi);
				SHFT2(f2, f1, d);
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
