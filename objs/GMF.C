//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_gmf_c[] =
	"@(#) $Id$";

#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include "GMF.h"

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

	_chiCount = 37;
	_chiMin = 0.0;
	_chiMax = 360.0;
	_chiStep = 10.0;

	if (! _Allocate())
		return(0);

	float value;
	for (int pol_idx = 0; pol_idx < _polCount; pol_idx++)
	{
		for (int chi_idx = 0; chi_idx < _chiCount; chi_idx++)
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

//----------------//
// GMF::GetSigma0 //
//----------------//

int
GMF::GetSigma0(
	int		pol,
	double	inc,
	double	wind_spd,
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
	double spd_didx = (wind_spd - _spdMin) / _spdStep;
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
