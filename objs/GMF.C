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
:	_polCount(0), _incCount(0), _incMin(0.0), _incMax(0.0),
	_incStep(0.0), _spdCount(0), _spdMin(0.0), _spdMax(0.0), _spdStep(0.0),
	_chiCount(0), _chiStep(0.0), _value(0)
{
	return;
}

GMF::~GMF()
{
	_Deallocate();
	return;
}

//-----------//
// GMF::Read //
//-----------//

int
GMF::Read(
	const char*		filename)
{
	int fd = open(filename, O_RDONLY);
	if (fd == -1)
		return(0);

	if (! _ReadHeader(fd))
		return(0);

	if (! _Allocate())
		return(0);

	if (! _ReadTable(fd))
		return(0);

	close(fd);
	return(1);
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

//----------------------//
// GMF::GetNearestValue //
//----------------------//

int
GMF::GetNearestValue(
	PolE		pol,
	double		inc,
	double		spd,
	double		chi,
	double*		value)
{
	//-------------------//
	// round to indicies //
	//-------------------//

	int pol_idx = _PolToIndex(pol);
	int inc_idx = _IncToIndex(inc);
	int spd_idx = _SpdToIndex(spd);
	int chi_idx = _ChiToIndex(chi);

	//------------------------//
	// keep indicies in range //
	//------------------------//

	pol_idx = _ClipPolIndex(pol_idx);
	inc_idx = _ClipIncIndex(inc_idx);
	spd_idx = _ClipSpdIndex(spd_idx);

	//--------------//
	// access table //
	//--------------//

	*value = *(*(*(*(_value + pol_idx) + inc_idx) + spd_idx) + chi_idx);
	return(1);
}

//---------------------------//
// GMF::GetInterpolatedValue //
//---------------------------//

#define ORDER_PLUS_ONE	4

int
GMF::GetInterpolatedValue(
	PolE		pol,
	double		inc,
	double		spd,
	double		chi,
	double*		value)
{
	//---------------------------------------//
	// determine real and truncated indicies //
	//---------------------------------------//

	int pol_idx = _PolToIndex(pol);

	double inc_ridx = _IncToRealIndex(inc);
	double spd_ridx = _SpdToRealIndex(spd);
	double chi_ridx = _ChiToRealIndex(chi);

	int inc_idx = (int)inc_ridx;
	int spd_idx = (int)spd_ridx;
	int chi_idx = (int)chi_ridx;

	//-------------------------//
	// set up temporary arrays //
	//-------------------------//

	double val_x[ORDER_PLUS_ONE];
	double val_y[ORDER_PLUS_ONE];
	double val_spd_chi[ORDER_PLUS_ONE][ORDER_PLUS_ONE];

	//---------------------------------//
	// interpolate out incidence angle //
	//---------------------------------//

	// offsets transform the index to the starting index
	int inc_offset = inc_idx - (ORDER_PLUS_ONE / 2) + 1;
	if (inc_offset < 0)
		inc_offset = 0;
	if (inc_offset > _incCount - ORDER_PLUS_ONE)
		inc_offset = _incCount - ORDER_PLUS_ONE;
	double inc_subtable_ridx = inc_ridx - (double)inc_offset;

	int spd_offset = spd_idx - (ORDER_PLUS_ONE / 2) + 1;
	if (spd_offset < 0)
		spd_offset = 0;
	if (spd_offset > _spdCount - ORDER_PLUS_ONE)
		spd_offset = _spdCount - ORDER_PLUS_ONE;
	double spd_subtable_ridx = spd_ridx - (double)spd_offset;

	int chi_offset = chi_idx - (ORDER_PLUS_ONE / 2) + 1;
	double chi_subtable_ridx = chi_ridx - (double)chi_offset;
	if (chi_subtable_ridx < 0.0)
		chi_subtable_ridx += (double)_chiCount;		// keep chi in array

	int sidx, cidx, iidx;
	int use_sidx, use_cidx, use_iidx;
	for (sidx = 0; sidx < ORDER_PLUS_ONE; sidx++)
	{
		use_sidx = sidx + spd_offset;
		for (cidx = 0; cidx < ORDER_PLUS_ONE; cidx++)
		{
			use_cidx = cidx + chi_offset;
			use_cidx = (use_cidx + _chiCount) % _chiCount;
			for (iidx = 0; iidx < ORDER_PLUS_ONE; iidx++)
			{
				use_iidx = iidx + inc_offset;
				val_x[iidx] = (double)iidx;
				val_y[iidx] = *(*(*(*(_value + pol_idx) + use_iidx) +
					use_sidx) + use_cidx);
			}
			polint(val_x, val_y, ORDER_PLUS_ONE, inc_subtable_ridx,
				&(val_spd_chi[sidx][cidx]));
		}
	}

	//-----------------------//
	// interpolate out speed //
	//-----------------------//

	double val_chi[ORDER_PLUS_ONE];
	for (cidx = 0; cidx < ORDER_PLUS_ONE; cidx++)
	{
		for (sidx = 0; sidx < ORDER_PLUS_ONE; sidx++)
		{
			val_x[sidx] = (double)sidx;
			val_y[sidx] = val_spd_chi[sidx][cidx];
		}
		polint(val_x, val_y, ORDER_PLUS_ONE, spd_subtable_ridx,
			&(val_chi[cidx]));
	}

	//---------------------------//
	// interpolate out direciton //
	//---------------------------//

	double val;
	for (cidx = 0; cidx < ORDER_PLUS_ONE; cidx++)
	{
		val_x[cidx] = (double)cidx;
		val_y[cidx] = val_chi[cidx];
	}
	polint(val_x, val_y, ORDER_PLUS_ONE, chi_subtable_ridx, &val);

	*value = val;

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

#define INITIAL_SPEED		8.0

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

	int phi_count = (int)(two_pi / phi_step);
	double dphi = two_pi / (double)phi_count;

	int spd_count = (int)(_spdMax / spd_step);
	double dspd = _spdMax / (double)spd_count;

	//-------------------------//
	// allocate storage arrays //
	//-------------------------//

	int* best_spd_idx = new int[phi_count];
	double* best_obj = new double[phi_count];

	//-----------------//
	// for each phi... //
	//-----------------//

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
// printf("%g %g\n", phi * rtd, spd);
	}

	//--------------------------------//
	// find local maxima in phi strip //
	//--------------------------------//

	int count = 0;
	for (phi_idx = 0; phi_idx < phi_count; phi_idx++)
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
// GMF::_Allocate //
//----------------//

int
GMF::_Allocate()
{
	_value = (double ****)malloc(_polCount * sizeof(double ***));
	if (_value == NULL)
		return(0);

	for (int i = 0; i < _polCount; i++)
	{
		double*** dppp = (double ***)malloc(_incCount * sizeof(double **));
		if (dppp == NULL)
			return(0);
		*(_value + i) = dppp;

		for (int j = 0; j < _incCount; j++)
		{
			double** dpp = (double **)malloc(_spdCount * sizeof(double *));
			if (dpp == NULL)
				return(0);
			*(*(_value + i) + j) = dpp;

			for (int k = 0; k < _spdCount; k++)
			{
				double* dp = (double *)malloc(_chiCount * sizeof(double));
				if (dp == NULL)
					return(0);
				*(*(*(_value + i) + j) + k) = dp;
			}
		}
	}
	return(1);
}

//------------------//
// GMF::_Deallocate //
//------------------//

int
GMF::_Deallocate()
{
	for (int i = 0; i < _polCount; i++)
	{
		for (int j = 0; j < _incCount; j++)
		{
			for (int k = 0; k < _spdCount; k++)
			{
				free(*(*(*(_value + i) + j) + k));
			}
			free(*(*(_value + i) + j));
		}
		free(*(_value + i));
	}
	free(_value);
	_value = NULL;
	return(1);
}

//------------------//
// GMF::_ReadHeader //
//------------------//

int
GMF::_ReadHeader(
	int		fd)
{
	read(fd, &_polCount, sizeof(int));

	read(fd, &_incCount, sizeof(int));
	read(fd, &_incMin, sizeof(double));
	read(fd, &_incMax, sizeof(double));

	read(fd, &_spdCount, sizeof(int));
	read(fd, &_spdMin, sizeof(double));
	read(fd, &_spdMax, sizeof(double));

	read(fd, &_chiCount, sizeof(int));

	return(1);
}

//-----------------//
// GMF::_ReadTable //
//-----------------//

int
GMF::_ReadTable(
	int		fd)
{
	//----------------//
	// read the array //
	//----------------//

	for (int i = 0; i < _polCount; i++)
	{
		for (int j = 0; j < _incCount; j++)
		{
			for (int k = 0; k < _spdCount; k++)
			{
				read(fd, *(*(*(_value + i) + j) + k),
					sizeof(double) * _chiCount);
			}
		}
	}

	//----------------------//
	// calculate step sizes //
	//----------------------//

	_incStep = (_incMax - _incMin) / (double)(_incCount - 1);
	_spdStep = (_spdMax - _spdMin) / (double)(_spdCount - 1);
	_chiStep = two_pi / (double)_chiCount;

	return(1);
}

//------------------//
// GMF::_PolToIndex //
//------------------//

int
GMF::_PolToIndex(
	PolE	pol)
{
	// this should be done better later
	int idx = 0;
	switch(pol)
	{
	case V_POL:
		idx = 0;
		break;
	case H_POL:
		idx = 1;
		break;
	}
	return(idx);
}

//------------------//
// GMF::_IncToIndex //
//------------------//

int
GMF::_IncToIndex(
	double		inc)
{
	double inc_ridx = _IncToRealIndex(inc);
	int inc_idx = (int)(inc_ridx + 0.5);
	return(inc_idx);
}

//------------------//
// GMF::_SpdToIndex //
//------------------//

int
GMF::_SpdToIndex(
	double		spd)
{
	double spd_ridx = _SpdToRealIndex(spd);
	int spd_idx = (int)(spd_ridx + 0.5);
	return(spd_idx);
}

//------------------//
// GMF::_ChiToIndex //
//------------------//

int
GMF::_ChiToIndex(
	double		chi)
{
	double chi_ridx = _ChiToRealIndex(chi);
	int chi_idx = (int)(chi_ridx + 0.5);
	return(chi_idx);
}

//----------------------//
// GMF::_IncToRealIndex //
//----------------------//

double
GMF::_IncToRealIndex(
	double		inc)
{
	return((inc - _incMin) / _incStep);
}

//----------------------//
// GMF::_SpdToRealIndex //
//----------------------//

double
GMF::_SpdToRealIndex(
	double		spd)
{
	return((spd - _spdMin) / _spdStep);
}

//----------------------//
// GMF::_ChiToRealIndex //
//----------------------//

double
GMF::_ChiToRealIndex(
	double		chi)
{
	if (chi < 0.0)
	{
		int chi_idx = (chi / two_pi) + 1;
		chi += chi_idx * two_pi;
	}
	chi = fmod(chi, two_pi);
	return(chi / _chiStep);
}

//--------------------//
// GMF::_ClipPolIndex //
//--------------------//

int
GMF::_ClipPolIndex(
	int		pol_idx)
{
	if (pol_idx < 0)
		return(0);
	else if (pol_idx >= _polCount)
		return(_polCount - 1);
	else
		return(pol_idx);
}

//--------------------//
// GMF::_ClipIncIndex //
//--------------------//

int
GMF::_ClipIncIndex(
	int		inc_idx)
{
	if (inc_idx < 0)
		return(0);
	else if (inc_idx >= _incCount)
		return(_incCount - 1);
	else
		return(inc_idx);
}

//--------------------//
// GMF::_ClipSpdIndex //
//--------------------//

int
GMF::_ClipSpdIndex(
	int		spd_idx)
{
	if (spd_idx < 0)
		return(0);
	else if (spd_idx >= _spdCount)
		return(_spdCount - 1);
	else
		return(spd_idx);
}

//--------------------//
// GMF::_ClipChiIndex //
//--------------------//

int
GMF::_ClipChiIndex(
	int		chi_idx)
{
	if (chi_idx < 0)
		return(0);
	else if (chi_idx >= _chiCount)
		return(_chiCount - 1);
	else
		return(chi_idx);
}

//------------------//
// GMF::_IndexToSpd //
//------------------//

double
GMF::_IndexToSpd(
	int		spd_idx)
{
	return((double)spd_idx * _spdStep + _spdMin);
}

//------------------//
// GMF::_IndexToChi //
//------------------//

double
GMF::_IndexToChi(
	int		chi_idx)
{
	return((double)chi_idx * _chiStep);
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
//		double chi = meas->eastAzimuth - phi;
		double chi = phi - (meas->eastAzimuth + pi);
		double gmf_value;
		GetInterpolatedValue(meas->pol, meas->incidenceAngle, spd, chi,
			&gmf_value);
		double s = gmf_value - meas->value;
		fv += s*s / meas->estimatedKp + log10(meas->estimatedKp);
	}
	return(-fv);
}
