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
	_incMin = 16.0;
	_incMax = 66.0;
	_incStep = 2.0;

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
						(double)value;

					int chi_idx_2 = (_chiCount - chi_idx) % _chiCount;
					*(*(*(*(_value+pol_idx)+inc_idx)+spd_idx)+chi_idx_2) =
						(double)value;
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
	double		inc,
	double		spd,
	double*		A0,
	double*		A1,
	double*		A1_phase,
	double*		A2,
	double*		A2_phase,
	double*		A3,
	double*		A3_phase,
	double*		A4,
	double*		A4_phase)
{
	double real[5], imag[5];
	int n = _chiCount - 1;
	double wn = two_pi / n;

	for (int i = 0; i < 5; i++)
	{
		real[i] = 0.0;
		imag[i] = 0.0;

		// assumes single point overlap in chi
		for (int chi_idx = 0; chi_idx < n; chi_idx++)
		{
			double arg = wn * (double)i * (double)chi_idx;
			double c = cos(arg);
			double s = sin(arg);
			double chi = (double)chi_idx * _chiStep;
			double val;
			GetInterpolatedValue(pol, inc, spd, chi, &val);
			real[i] += val * c;
			imag[i] += val * s;
		}
	}

	*A0 = real[0] / (double)n;
	*A1 = 2.0 * sqrt(real[1] * real[1] + imag[1] * imag[1]) / (double)n;
	*A1_phase = -atan2(imag[1], real[1]);
	*A2 = 2.0 * sqrt(real[2] * real[2] + imag[2] * imag[2]) / (double)n;
	*A2_phase = -atan2(imag[2], real[2]);
	*A3 = 2.0 * sqrt(real[3] * real[3] + imag[3] * imag[3]) / (double)n;
	*A3_phase = -atan2(imag[3], real[3]);
	*A4 = 2.0 * sqrt(real[4] * real[4] + imag[4] * imag[4]) / (double)n;
	*A4_phase = -atan2(imag[4], real[4]);

	return(1);
}

//--------------------//
// GMF::FindSolutions //
//--------------------//

int
GMF::FindSolutions(
	MeasurementList*	measurement_list,
	WVC*				wvc,
	double				spd_step,
	double				phi_step)		// in radians
{
	//---------------------------------------//
	// determine index ranges and step sizes //
	//---------------------------------------//

	int phi_count = (int)(two_pi / phi_step + 0.5);
	double dphi = two_pi / (double)phi_count;

	int spd_count = (int)(_spdMax / spd_step + 0.5);
	double dspd = _spdMax / (double)spd_count;

	//-------------------------//
	// allocate storage arrays //
	//-------------------------//

	int* best_spd_idx = new int[phi_count];
	double* best_obj = new double[phi_count];

	//-------------------------//
	// find the solution curve //
	//-------------------------//

	_FindSolutionCurve(measurement_list, dspd, dphi, phi_count, best_spd_idx,
		best_obj);

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
			WindVector* new_wv = new WindVector();
			if (! new_wv)
				return(0);
			new_wv->SetSpdDir(best_spd_idx[phi_idx] * dspd, phi_idx * dphi);
			if (! wvc->ambiguities.Append(new_wv))
			{
				delete best_spd_idx;
				delete best_obj;
				return(0);
			}
			count++;
		}
	}

	delete best_spd_idx;
	delete best_obj;
	return(count);
}

//----------------------//
// GMF::RefineSolutions //
//----------------------//

enum { DOWN = 0, UP };

int
GMF::RefineSolutions(
	MeasurementList*	measurement_list,
	WVC*				wvc,
	double				initial_spd_step,
	double				initial_phi_step,
	double				final_spd_step,
	double				final_phi_step)
{
	//----------------------//
	// for each solution... //
	//----------------------//

	for (WindVector* wv = wvc->ambiguities.GetHead(); wv;
		wv = wvc->ambiguities.GetNext())
	{
		//--------------------------------------//
		// start with initial search resolution //
		//--------------------------------------//

		double spd_step = initial_spd_step;
		double phi_step = initial_phi_step;

		//--------------------------------------------//
		// quantize speed and direction to step sizes //
		//--------------------------------------------//

		int spd_idx = (int)(wv->spd / spd_step + 0.5);
		wv->spd = (double)spd_idx * spd_step;

		int phi_idx = (int)(wv->dir / phi_step + 0.5);
		wv->dir = (double)phi_idx * phi_step;

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
			double max_obj = -9e99;		// I hope this is low enough <g>
			for (int dspd = -1; dspd <= 1; dspd++)
			{
				for (int dphi = -1; dphi <= 1; dphi++)
				{
					double obj = _ObjectiveFunction(measurement_list,
						wv->spd + dspd * spd_step, wv->dir + dphi * phi_step);
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

			spd_idx = (int)(wv->spd / spd_step + 0.5) + max_dspd;
			phi_idx = (int)(wv->dir / phi_step + 0.5) + max_dphi;

			wv->spd = (double)spd_idx * spd_step;
			wv->dir = (double)phi_idx * phi_step;
		}
	}

	return(1);
}

//----------------//
// GMF::ModCurves //
//----------------//

int
GMF::ModCurves(
	FILE*				ofp,
	MeasurementList*	measurement_list,
	double				spd_step,
	double				phi_step)
{
	//---------------------------------------//
	// determine index ranges and step sizes //
	//---------------------------------------//

	int phi_count = (int)(two_pi / phi_step + 0.5);
	double dphi = two_pi / (double)phi_count;

	int spd_count = (int)(_spdMax / spd_step + 0.5);
	double dspd = _spdMax / (double)spd_count;

	//-------------------------//
	// allocate storage arrays //
	//-------------------------//

	int* best_spd_idx = new int[phi_count];
	double* best_obj = new double[phi_count];

	//------------------------------//
	// generate each solution curve //
	//------------------------------//

	MeasurementList new_list;
	for (Measurement* meas = measurement_list->GetHead(); meas;
			meas = measurement_list->GetNext())
	{
		new_list.Append(meas);
		_FindSolutionCurve(&new_list, dspd, dphi, phi_count,
			best_spd_idx, best_obj);
		for (int i = 0; i < phi_count; i++)
		{
			fprintf(ofp, "%g %g\n", (double)i * dphi * rtd,
				(double)best_spd_idx[i] * dspd);
		}
		fprintf(ofp, "&\n");
		new_list.GetHead();
		new_list.RemoveCurrent();
	}

	//----------------------------------//
	// generate combined solution curve //
	//----------------------------------//

	_FindSolutionCurve(measurement_list, dspd, dphi, phi_count,
            best_spd_idx, best_obj);
	for (int i = 0; i < phi_count; i++)
	{
		fprintf(ofp, "%g %g %g\n", (double)i * dphi * rtd,
			(double)best_spd_idx[i] * dspd, best_obj[i]);
	}

	//-----------------------//
	// delete storage arrays //
	//-----------------------//

	return(1);
}

//-------------------------//
// GMF::_ObjectiveFunction //
//-------------------------//

double
GMF::_ObjectiveFunction(
	MeasurementList*	measurement_list,
	double				spd,
	double				phi)
{
	double fv = 0.0;
	for (Measurement* meas = measurement_list->GetHead(); meas;
			meas = measurement_list->GetNext())
	{
		double chi = phi - (meas->eastAzimuth + pi);
		double gmf_value;
		GetInterpolatedValue(meas->pol, meas->incidenceAngle, spd, chi,
			&gmf_value);
		double s = gmf_value - meas->value;
		fv += s*s / meas->estimatedKp + log10(meas->estimatedKp);
	}
	return(-fv);
}

//-------------------------//
// GMF::_FindSolutionCurve //
//-------------------------//

#define INITIAL_SPEED		8.0

int
GMF::_FindSolutionCurve(
	MeasurementList*	measurement_list,
	double				dspd,
	double				dphi,
	int					phi_count,
	int*				best_spd_idx,
	double*				best_obj)
{
	int spd_idx = (int)(INITIAL_SPEED / dspd + 0.5);
	double spd = (double)spd_idx * dspd;

	for (int phi_idx = 0; phi_idx < phi_count; phi_idx++)
	{
		//------------------------//
		// ...find the best speed //
		//------------------------//

		double phi = (double)phi_idx * dphi;

		double obj = _ObjectiveFunction(measurement_list, spd, phi);
		double obj_minus = _ObjectiveFunction(measurement_list, spd - dspd,
			phi);
		double obj_plus = _ObjectiveFunction(measurement_list, spd + dspd,
			phi);
		do
		{
			if (obj > obj_minus && obj > obj_plus)
			{
				// peak
				best_spd_idx[phi_idx] = spd_idx;
				best_obj[phi_idx] = obj;
				break;
			}
			else if (obj_plus > obj && obj > obj_minus)
			{
				// move up
				spd_idx++;
				spd = (double)spd_idx * dspd;
				obj_minus = obj;
				obj = obj_plus;
				obj_plus = _ObjectiveFunction(measurement_list, spd + dspd,
					phi);
			}
			else if (obj_minus > obj && obj > obj_plus)
			{
				// move down
				spd_idx--;
				spd = (double)spd_idx * dspd;
				obj_plus = obj;
				obj = obj_minus;
				obj_minus = _ObjectiveFunction(measurement_list, spd - dspd,
					phi);
			}
		} while (1);
	}
	return(1);
}
