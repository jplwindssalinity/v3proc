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

GMF::GMF(
	const char*		filename)
{
	filename;
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
	//-------------------//
	// open the GMF file //
	//-------------------//

	int ifd = open(filename, O_RDONLY);
	if (ifd == -1)
		return(0);

	//--------------------//
	// read in the header //
	//--------------------//

	read(ifd, &_polCount, sizeof(int));

	read(ifd, &_incCount, sizeof(int));
	read(ifd, &_incMin, sizeof(double));
	read(ifd, &_incMax, sizeof(double));

	read(ifd, &_spdCount, sizeof(int));
	read(ifd, &_spdMin, sizeof(double));
	read(ifd, &_spdMax, sizeof(double));

	read(ifd, &_chiCount, sizeof(int));
	read(ifd, &_chiMin, sizeof(double));
	read(ifd, &_chiMax, sizeof(double));

	//--------------------//
	// allocate the array //
	//--------------------//

	_sigma0 = (double ****)malloc(_polCount * sizeof(double ***));
	if (_sigma0 == NULL)
		return(0);

	int i, j, k;
	for (i = 0; i < _polCount; i++)
	{
		double*** dppp = (double ***)malloc(_incCount * sizeof(double **));
		if (dppp == NULL)
			return(0);
		*(_sigma0 + i) = dppp;

		for (j = 0; j < _incCount; j++)
		{
			double** dpp = (double **)malloc(_spdCount * sizeof(double *));
			if (dpp == NULL)
				return(0);
			*(*(_sigma0 + i) + j) = dpp;

			for (k = 0; k < _spdCount; k++)
			{
				double* dp = (double *)malloc(_chiCount * sizeof(double));
				if (dp == NULL)
					return(0);
				*(*(*(_sigma0 + i) + j) + k) = dp;
			}
		}
	}

	//----------------//
	// read the array //
	//----------------//

	for (i = 0; i < _polCount; i++)
	{
		for (j = 0; j < _incCount; j++)
		{
			for (k = 0; k < _spdCount; k++)
			{
				read(ifd, *(*(*(_sigma0 + i) + j) + k),
					sizeof(double) * _chiCount);
			}
		}
	}

	//----------------//
	// close the file //
	//----------------//

	close(ifd);

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
