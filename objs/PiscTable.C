//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_pisctable_c[] =
	"@(#) $Id$";

#include <fcntl.h>
#include <unistd.h>
#include "PiscTable.h"
#include "Interpolate.h"
#include "Constants.h"
#include "Array.h"

#define INC_TO_REAL_IDX(A) ((A - _incMin) / _incStep)
#define SPD_TO_REAL_IDX(A) ((A - _spdMin) / _spdStep)
#define CHI_TO_REAL_IDX(A) (A / _chiStep)

//===========//
// PiscTable //
//===========//

PiscTable::PiscTable()
:	_polCount(0), _incCount(0), _incMin(0.0), _incMax(0.0), _incStep(0.0),
	_spdCount(0), _spdMin(0.0), _spdMax(0.0), _spdStep(0.0), _chiCount(0),
	_chiStep(0.0), _value(0)
{
	return;
}

PiscTable::~PiscTable()
{
	_Deallocate();
	return;
}

//-----------------//
// PiscTable::Read //
//-----------------//

int
PiscTable::Read(
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

//-----------------//
// PiscTable:Write //
//-----------------//

int PiscTable::Write(
	const char*		filename)
{
	int fd = creat(filename, 0644);
	if (fd == -1)
		return(0);

	if (! _WriteHeader(fd))
		return(0);

	if (! _WriteTable(fd))
		return(0);

	close(fd);
	return(1);
}

//----------------------------//
// PiscTable::GetNearestValue //
//----------------------------//

int
PiscTable::GetNearestValue(
	PolE		pol,
	float		inc,
	float		spd,
	float		chi,
	float*		value)
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

//---------------------------------//
// PiscTable::GetInterpolatedValue //
//---------------------------------//

int
PiscTable::GetInterpolatedValue(
	PolE		pol,
	float		inc,
	float		spd,
	float		chi,
	float*		value)
{
	//-------------------------//
	// determine real indicies //
	//-------------------------//

	int pol_idx = _PolToIndex(pol);
	float*** table = *(_value + pol_idx);

	float inc_ridx = INC_TO_REAL_IDX(inc);
	float spd_ridx = SPD_TO_REAL_IDX(spd);
	while (chi < 0.0) chi += two_pi;
	while (chi >= two_pi) chi -= two_pi;
	float chi_ridx = CHI_TO_REAL_IDX(chi);

	//------------------------------------//
	// calculate upper and lower indicies //
	//------------------------------------//

	int li = (int)inc_ridx;
	if (li < 0) li = 0;
	int hi = li + 1;
	if (hi >= _incCount)
	{
		hi = _incCount - 1;
		li = hi - 1;
	}
	float lo_inc = _incMin + li * _incStep;

	int ls = (int)spd_ridx;
	if (ls < 0) ls = 0;
	int hs = ls + 1;
	if (hs >= _spdCount)
	{
		hs = _spdCount - 1;
		ls = hs - 1;
	}
	float lo_spd = _spdMin + ls * _spdStep;

	int lc = (int)chi_ridx;
	int hc = lc + 1;
	hc %= _chiCount;
	float lo_chi = lc * _chiStep;

	//--------------------------//
	// assign fractional values //
	//--------------------------//

	float ai = (inc - lo_inc) / _incStep;
	float as = (spd - lo_spd) / _spdStep;
	float ac = (chi - lo_chi) / _chiStep;

	float bi = 1.0 - ai;
	float bs = 1.0 - as;
	float bc = 1.0 - ac;

	//---------------//
	// interpolation //
	//---------------//

	float val =
		ai * as * ac * *(*(*(table + hi) + hs) + hc) +
		ai * as * bc * *(*(*(table + hi) + hs) + lc) +
		ai * bs * ac * *(*(*(table + hi) + ls) + hc) +
		ai * bs * bc * *(*(*(table + hi) + ls) + lc) +
		bi * as * ac * *(*(*(table + li) + hs) + hc) +
		bi * as * bc * *(*(*(table + li) + hs) + lc) +
		bi * bs * ac * *(*(*(table + li) + ls) + hc) +
		bi * bs * bc * *(*(*(table + li) + ls) + lc);

	//--------------//
	// return value //
	//--------------//

	*value = val;
	return(1);
}

//----------------------//
// PiscTable::_Allocate //
//----------------------//

int
PiscTable::_Allocate()
{
	//------------------------------------//
	// allocate the primary pointer array //
	//------------------------------------//

	_value = (float****)make_array(sizeof(float), 4, _polCount,
		_incCount, _spdCount, _chiCount);

	if (_value == NULL)
		return(0);

	return(1);
}

//------------------------//
// PiscTable::_Deallocate //
//------------------------//

int
PiscTable::_Deallocate()
{
	if (_value == NULL)
		return(1);

	free_array((void *)_value, 4, _polCount, _incCount, _spdCount, _chiCount);

	_value = NULL;
	return(1);
}

//------------------------//
// PiscTable::_ReadHeader //
//------------------------//

int
PiscTable::_ReadHeader(
	int		fd)
{
	if (read(fd, &_polCount, sizeof(int)) != sizeof(int) ||
		read(fd, &_incMin, sizeof(float)) != sizeof(float) ||
		read(fd, &_incMax, sizeof(float)) != sizeof(float) ||
		read(fd, &_spdCount, sizeof(int)) != sizeof(int) ||
		read(fd, &_spdMin, sizeof(float)) != sizeof(float) ||
		read(fd, &_spdMax, sizeof(float)) != sizeof(float) ||
		read(fd, &_chiCount, sizeof(int)) != sizeof(int))
	{
		return(0);
	}
	return(1);
}

//-----------------------//
// PiscTable::_ReadTable //
//-----------------------//

int
PiscTable::_ReadTable(
	int		fd)
{
	//----------------//
	// read the array //
	//----------------//

	int size = sizeof(float) * _chiCount;
	for (int i = 0; i < _polCount; i++)
	{
		for (int j = 0; j < _incCount; j++)
		{
			for (int k = 0; k < _spdCount; k++)
			{
				if (read(fd, *(*(*(_value + i) + j) + k), size) != size)
					return(0);
			}
		}
	}

	//----------------------//
	// calculate step sizes //
	//----------------------//

	_incStep = (_incMax - _incMin) / (float)(_incCount - 1);
	_spdStep = (_spdMax - _spdMin) / (float)(_spdCount - 1);
	_chiStep = two_pi / (float)_chiCount;

	return(1);
}

//-------------------------//
// PiscTable::_WriteHeader //
//-------------------------//

int
PiscTable::_WriteHeader(
	int		fd)
{
	if (write(fd, &_polCount, sizeof(int) != sizeof(int)) ||
		write(fd, &_incMin, sizeof(float)) != sizeof(float) ||
		write(fd, &_incMax, sizeof(float)) != sizeof(float) ||
		write(fd, &_spdCount, sizeof(int)) != sizeof(int) ||
		write(fd, &_spdMin, sizeof(float)) != sizeof(float) ||
		write(fd, &_spdMax, sizeof(float)) != sizeof(float) ||
		write(fd, &_chiCount, sizeof(int)) != sizeof(int))
	{
		return(0);
	}
	return(1);
}

//------------------------//
// PiscTable::_WriteTable //
//------------------------//

int
PiscTable::_WriteTable(
	int		fd)
{
	//-----------------//
	// write the array //
	//-----------------//

	int size = sizeof(float) * _chiCount;
	for (int i = 0; i < _polCount; i++)
	{
		for (int j = 0; j < _incCount; j++)
		{
			for (int k = 0; k < _spdCount; k++)
			{
				if (write(fd, *(*(*(_value + i) + j) + k), size) != size)
					return(0);
			}
		}
	}
	return(1);
}

//------------------------//
// PiscTable::_PolToIndex //
//------------------------//

int
PiscTable::_PolToIndex(
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
	case NONE:
		fprintf(stderr,
			"PiscTable::_PolToIndex: invalid polarization %d\n", pol);
		exit(1);
	}
	return(idx);
}

//------------------------//
// PiscTable::_IncToIndex //
//------------------------//

int
PiscTable::_IncToIndex(
	float		inc)
{
	float inc_ridx = INC_TO_REAL_IDX(inc);
	int inc_idx = (int)(inc_ridx + 0.5);
	return(inc_idx);
}

//------------------------//
// PiscTable::_SpdToIndex //
//------------------------//

int
PiscTable::_SpdToIndex(
	float		spd)
{
	float spd_ridx = SPD_TO_REAL_IDX(spd);
	int spd_idx = (int)(spd_ridx + 0.5);
	return(spd_idx);
}

//------------------------//
// PiscTable::_ChiToIndex //
//------------------------//

int
PiscTable::_ChiToIndex(
	float		chi)
{
	while (chi < 0.0) chi += two_pi;
	while (chi >= two_pi) chi -= two_pi;
	float chi_ridx = CHI_TO_REAL_IDX(chi);
	int chi_idx = (int)(chi_ridx + 0.5);
	chi_idx %= _chiCount;
	return(chi_idx);
}

//--------------------------//
// PiscTable::_ClipPolIndex //
//--------------------------//

int
PiscTable::_ClipPolIndex(
	int		pol_idx)
{
	if (pol_idx < 0)
		return(0);
	else if (pol_idx >= _polCount)
		return(_polCount - 1);
	else
		return(pol_idx);
}

//--------------------------//
// PiscTable::_ClipIncIndex //
//--------------------------//

int
PiscTable::_ClipIncIndex(
	int		inc_idx)
{
	if (inc_idx < 0)
		return(0);
	else if (inc_idx >= _incCount)
		return(_incCount - 1);
	else
		return(inc_idx);
}

//--------------------------//
// PiscTable::_ClipSpdIndex //
//--------------------------//

int
PiscTable::_ClipSpdIndex(
	int		spd_idx)
{
	if (spd_idx < 0)
		return(0);
	else if (spd_idx >= _spdCount)
		return(_spdCount - 1);
	else
		return(spd_idx);
}

//--------------------------//
// PiscTable::_ClipChiIndex //
//--------------------------//

int
PiscTable::_ClipChiIndex(
	int		chi_idx)
{
	if (chi_idx < 0)
		return(0);
	else if (chi_idx >= _chiCount)
		return(_chiCount - 1);
	else
		return(chi_idx);
}

//------------------------//
// PiscTable::_IndexToSpd //
//------------------------//

float
PiscTable::_IndexToSpd(
	int		spd_idx)
{
	return((float)spd_idx * _spdStep + _spdMin);
}

//------------------------//
// PiscTable::_IndexToChi //
//------------------------//

float
PiscTable::_IndexToChi(
	int		chi_idx)
{
	return((float)chi_idx * _chiStep);
}
