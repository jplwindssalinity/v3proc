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

//---------------------------------//
// PiscTable::GetInterpolatedValue //
//---------------------------------//

#define ORDER_PLUS_ONE	4

int
PiscTable::GetInterpolatedValue(
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

	_value = (double****)make_array(sizeof(double), 4, _polCount,
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
		read(fd, &_incMin, sizeof(double)) != sizeof(double) ||
		read(fd, &_incMax, sizeof(double)) != sizeof(double) ||
		read(fd, &_spdCount, sizeof(int)) != sizeof(int) ||
		read(fd, &_spdMin, sizeof(double)) != sizeof(double) ||
		read(fd, &_spdMax, sizeof(double)) != sizeof(double) ||
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

	int size = sizeof(double) * _chiCount;
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

	_incStep = (_incMax - _incMin) / (double)(_incCount - 1);
	_spdStep = (_spdMax - _spdMin) / (double)(_spdCount - 1);
	_chiStep = two_pi / (double)_chiCount;

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
		write(fd, &_incMin, sizeof(double)) != sizeof(double) ||
		write(fd, &_incMax, sizeof(double)) != sizeof(double) ||
		write(fd, &_spdCount, sizeof(int)) != sizeof(int) ||
		write(fd, &_spdMin, sizeof(double)) != sizeof(double) ||
		write(fd, &_spdMax, sizeof(double)) != sizeof(double) ||
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

	int size = sizeof(double) * _chiCount;
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
	double		inc)
{
	double inc_ridx = _IncToRealIndex(inc);
	int inc_idx = (int)(inc_ridx + 0.5);
	return(inc_idx);
}

//------------------------//
// PiscTable::_SpdToIndex //
//------------------------//

int
PiscTable::_SpdToIndex(
	double		spd)
{
	double spd_ridx = _SpdToRealIndex(spd);
	int spd_idx = (int)(spd_ridx + 0.5);
	return(spd_idx);
}

//------------------------//
// PiscTable::_ChiToIndex //
//------------------------//

int
PiscTable::_ChiToIndex(
	double		chi)
{
	double chi_ridx = _ChiToRealIndex(chi);
	int chi_idx = (int)(chi_ridx + 0.5);
	return(chi_idx);
}

//----------------------------//
// PiscTable::_IncToRealIndex //
//----------------------------//

double
PiscTable::_IncToRealIndex(
	double		inc)
{
	return((inc - _incMin) / _incStep);
}

//----------------------------//
// PiscTable::_SpdToRealIndex //
//----------------------------//

double
PiscTable::_SpdToRealIndex(
	double		spd)
{
	return((spd - _spdMin) / _spdStep);
}

//----------------------------//
// PiscTable::_ChiToRealIndex //
//----------------------------//

double
PiscTable::_ChiToRealIndex(
	double		chi)
{
	if (chi < 0.0)
	{
		int chi_idx = (int)(chi / two_pi) + 1;
		chi += (double)chi_idx * two_pi;
	}
	chi = fmod(chi, two_pi);
	return(chi / _chiStep);
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

double
PiscTable::_IndexToSpd(
	int		spd_idx)
{
	return((double)spd_idx * _spdStep + _spdMin);
}

//------------------------//
// PiscTable::_IndexToChi //
//------------------------//

double
PiscTable::_IndexToChi(
	int		chi_idx)
{
	return((double)chi_idx * _chiStep);
}
