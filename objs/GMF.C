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

//=====//
// GMF //
//=====//

GMF::GMF()
:	_polCount(0), _incCount(0), _incMin(0.0), _incMax(0.0), _incStep(0.0),
	_spdCount(0), _spdMin(0.0), _spdMax(0.0), _spdStep(0.0), _chiCount(0),
	_chiMin(0.0), _chiMax(0.0), _chiStep(0.0), _sigma0(0)
{
	return;
}

GMF::~GMF()
{
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

	if (! ReadHeader(fd))
		return(0);

	if (! _Allocate())
		return(0);

	if (! ReadTable(fd))
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
	_chiCount = 73;
	_chiMin = 0.0;
	_chiMax = 360.0;
	_chiStep = 5.0;

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
					*(*(*(*(_sigma0+pol_idx)+inc_idx)+spd_idx)+chi_idx) =
						(double)value;

					int chi_idx_2 = (_chiCount - 1) - chi_idx;
					*(*(*(*(_sigma0+pol_idx)+inc_idx)+spd_idx)+chi_idx_2) =
						(double)value;
				}
			}
		}
	}

	close(fd);
	return(1);
}

//-----------------//
// GMF::ReadHeader //
//-----------------//

int
GMF::ReadHeader(
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
	read(fd, &_chiMin, sizeof(double));
	read(fd, &_chiMax, sizeof(double));

	return(1);
}

//----------------//
// GMF::_Allocate //
//----------------//

int
GMF::_Allocate()
{
	_sigma0 = (double ****)malloc(_polCount * sizeof(double ***));
	if (_sigma0 == NULL)
		return(0);

	for (int i = 0; i < _polCount; i++)
	{
		double*** dppp = (double ***)malloc(_incCount * sizeof(double **));
		if (dppp == NULL)
			return(0);
		*(_sigma0 + i) = dppp;

		for (int j = 0; j < _incCount; j++)
		{
			double** dpp = (double **)malloc(_spdCount * sizeof(double *));
			if (dpp == NULL)
				return(0);
			*(*(_sigma0 + i) + j) = dpp;

			for (int k = 0; k < _spdCount; k++)
			{
				double* dp = (double *)malloc(_chiCount * sizeof(double));
				if (dp == NULL)
					return(0);
				*(*(*(_sigma0 + i) + j) + k) = dp;
			}
		}
	}
	return(1);
}

//----------------//
// GMF::ReadTable //
//----------------//

int
GMF::ReadTable(
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
				read(fd, *(*(*(_sigma0 + i) + j) + k),
					sizeof(double) * _chiCount);
			}
		}
	}

	//----------------------//
	// calculate step sizes //
	//----------------------//

	_incStep = (_incMax - _incMin) / (double)(_incCount - 1);
	_spdStep = (_spdMax - _spdMin) / (double)(_spdCount - 1);
	_chiStep = (_chiMax - _chiMin) / (double)(_chiCount - 1);

	return(1);
}

//-----------------------//
// GMF::GetNearestSigma0 //
//-----------------------//

int
GMF::GetNearestSigma0(
	int		pol,
	double	inc,
	double	spd,
	double	chi,
	double*	sigma_0)
{
	// make sure that pol is within table range
	if (pol < 0 || pol >= _polCount)
		return(0);

	// make sure that chi is positive and within table range
	while (chi < 0.0)
		chi += 360.0;
	while (chi >= 360.0)
		chi -= 360.0;
	if (chi < _chiMin - _chiStep || chi > _chiMax + _chiStep)
		return(0);

	// take nearest value
	// this needs to be changed to interpolation
	double inc_didx = (inc - _incMin) / _incStep;
	double spd_didx = (spd - _spdMin) / _spdStep;
	double chi_didx = (chi - _chiMin) / _chiStep;

	int inc_idx = (int)(inc_didx + 0.5);
	int spd_idx = (int)(spd_didx + 0.5);
	int chi_idx = (int)(chi_didx + 0.5);

	if (inc_idx < 0) inc_idx = 0;
	if (inc_idx >= _incCount) inc_idx = _incCount;
	if (spd_idx < 0) spd_idx = 0;
	if (spd_idx >= _spdCount) spd_idx = _spdCount;
	if (chi_idx < 0) chi_idx = 0;
	if (chi_idx >= _chiCount) chi_idx = _chiCount;

	*sigma_0 = *(*(*(*(_sigma0 + pol) + inc_idx) + spd_idx) + chi_idx);
	return(1);
}

//---------------------//
// GMF::GetInterSigma0 //
//---------------------//

#define ORDER_PLUS_ONE	4

int
GMF::GetInterSigma0(
	int		pol,
	double	inc,
	double	spd,
	double	chi,
	double*	sigma_0)
{
	// make sure that pol is within table range
	if (pol < 0 || pol >= _polCount)
		return(0);

	// make sure that chi is positive and within table range
	while (chi < 0.0)
		chi += 360.0;
	while (chi >= 360.0)
		chi -= 360.0;
	if (chi < _chiMin - _chiStep || chi > _chiMax + _chiStep)
		return(0);

	// set up tmp arrays
	double s0_x[ORDER_PLUS_ONE];
	double s0_y[ORDER_PLUS_ONE];
	double s0_spd_chi[ORDER_PLUS_ONE][ORDER_PLUS_ONE];

	//---------------------------------//
	// interpolate out incidence angle //
	//---------------------------------//

	double inc_didx = (inc - _incMin) / _incStep;
	int inc_idx = (int)inc_didx;
	int inc_offset = inc_idx - (ORDER_PLUS_ONE / 2) + 1;
	if (inc_offset < 0)
		inc_offset = 0;
	if (inc_offset > _incCount - ORDER_PLUS_ONE)
		inc_offset = _incCount - ORDER_PLUS_ONE;
	double inc_subtable_didx = inc_didx - (double)inc_offset;

	double spd_didx = (spd - _spdMin) / _spdStep;
	int spd_idx = (int)spd_didx;
	int spd_offset = spd_idx - (ORDER_PLUS_ONE / 2) + 1;
	if (spd_offset < 0)
		spd_offset = 0;
	if (spd_offset > _spdCount - ORDER_PLUS_ONE)
		spd_offset = _spdCount - ORDER_PLUS_ONE;
	double spd_subtable_didx = spd_didx - (double)spd_offset;

	double chi_didx = (chi - _chiMin) / _chiStep;
	int chi_idx = (int)chi_didx;
	int chi_offset = chi_idx - (ORDER_PLUS_ONE / 2) + 1;
	if (chi_offset < 0)
		chi_offset = 0;
	if (chi_offset > _chiCount - ORDER_PLUS_ONE)
		chi_offset = _chiCount - ORDER_PLUS_ONE;
	double chi_subtable_didx = chi_didx - (double)chi_offset;

	int sidx, cidx, iidx;
	for (sidx = 0; sidx < ORDER_PLUS_ONE; sidx++)
	{
		for (cidx = 0; cidx < ORDER_PLUS_ONE; cidx++)
		{
			for (iidx = 0; iidx < ORDER_PLUS_ONE; iidx++)
			{
				s0_x[iidx] = (double)iidx;
				s0_y[iidx] = *(*(*(*(_sigma0 + pol) + iidx + inc_offset) +
					sidx + spd_offset) + cidx + chi_offset);
			}
			polint(s0_x, s0_y, ORDER_PLUS_ONE, inc_subtable_didx,
				&(s0_spd_chi[sidx][cidx]));
		}
	}

	//-----------------------//
	// interpolate out speed //
	//-----------------------//

	double s0_chi[ORDER_PLUS_ONE];
	for (cidx = 0; cidx < ORDER_PLUS_ONE; cidx++)
	{
		for (sidx = 0; sidx < ORDER_PLUS_ONE; sidx++)
		{
			s0_x[sidx] = (double)sidx;
			s0_y[sidx] = s0_spd_chi[sidx][cidx];
		}
		polint(s0_x, s0_y, ORDER_PLUS_ONE, spd_subtable_didx,
			&(s0_chi[cidx]));
	}

	//---------------------------//
	// interpolate out direciton //
	//---------------------------//

	double s0;
	for (cidx = 0; cidx < ORDER_PLUS_ONE; cidx++)
	{
		s0_x[cidx] = (double)cidx;
		s0_y[cidx] = s0_chi[cidx];
	}
	polint(s0_x, s0_y, ORDER_PLUS_ONE, chi_subtable_didx, &s0);

	*sigma_0 = s0;

	return(1);
}

//---------------//
// GMF::GetCoefs //
//---------------//

int
GMF::GetCoefs(
	int			pol,
	double		inc,
	double		spd,
	double*		A0,
	double*		a1,
	double*		a2)
{
	double real[3], imag[3];
	int n = _chiCount - 1;
	double wn = M_PI * 2.0 / n;

	for (int i = 0; i < 3; i++)
	{
		real[i] = 0.0;
		imag[i] = 0.0;

		// assumes single point overlap in chi
		for (int chi_idx = 0; chi_idx < n; chi_idx++)
		{
			double arg = wn * (double)i * (double)chi_idx;
			double c = cos(arg);
			double s = sin(arg);
			double chi = ((double)chi_idx - _chiMin) * _chiStep;
			double s0;
			GetInterSigma0(pol, inc, spd, chi, &s0);
			real[i] += s0 * c;
			imag[i] += s0 * s;
		}
	}

	*A0 = real[0] / (double)n;
	*a1 = 2.0 * sqrt(real[1] * real[1] + imag[1] * imag[1]) / (double)n;
	*a2 = 2.0 * sqrt(real[2] * real[2] + imag[2] * imag[2]) / (double)n;

	return(1);
}
