//==============================================================//
// Copyright (C) 1997-1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

static const char rcs_id_misctable_c[] =
    "@(#) $Id$";

#include <fcntl.h>
#include <unistd.h>
#include "MiscTable.h"
#include "Interpolate.h"
#include "Constants.h"
#include "Array.h"

#define INC_TO_REAL_IDX(A) ((A - _incMin) / _incStep)
#define SPD_TO_REAL_IDX(A) ((A - _spdMin) / _spdStep)
#define CHI_TO_REAL_IDX(A) (A / _chiStep)

//===========//
// MiscTable //
//===========//

MiscTable::MiscTable()
:   _metCount(0), _incCount(0), _incMin(0.0), _incMax(0.0), _incStep(0.0),
    _spdCount(0), _spdMin(0.0), _spdMax(0.0), _spdStep(0.0), _chiCount(0),
    _chiStep(0.0), _value(0)
{
    return;
}

MiscTable::~MiscTable()
{
    _Deallocate();
    return;
}

//-----------------//
// MiscTable::Read //
//-----------------//

int
MiscTable::Read(
    const char*  filename)
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
// MiscTable:Write //
//-----------------//

int MiscTable::Write(
    const char*  filename)
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
// MiscTable::GetNearestValue //
//----------------------------//

int
MiscTable::GetNearestValue(
    Meas::MeasTypeE  met,
    float            inc,
    float            spd,
    float            chi,
    float*           value)
{
    //-------------------//
    // round to indicies //
    //-------------------//

    int met_idx = _MetToIndex(met);
    int inc_idx = _IncToIndex(inc);
    int spd_idx = _SpdToIndex(spd);
    int chi_idx = _ChiToIndex(chi);

    //------------------------//
    // keep indicies in range //
    //------------------------//

    met_idx = _ClipMetIndex(met_idx);
    inc_idx = _ClipIncIndex(inc_idx);
    spd_idx = _ClipSpdIndex(spd_idx);

    //--------------//
    // access table //
    //--------------//

    *value = *(*(*(*(_value + met_idx) + inc_idx) + spd_idx) + chi_idx);
    return(1);
}

//---------------------------------//
// MiscTable::GetInterpolatedValue //
//---------------------------------//

int
MiscTable::GetInterpolatedValue(
    Meas::MeasTypeE  met,
    float            inc,
    float            spd,
    float            chi,
    float*           value)
{
    //-------------------------//
    // determine real indicies //
    //-------------------------//

    int met_idx = _MetToIndex(met);
    float*** table = *(_value + met_idx);

    float inc_ridx = INC_TO_REAL_IDX(inc);
    float spd_ridx = SPD_TO_REAL_IDX(spd);

    while (chi < 0.0)
        chi += two_pi;

    while (chi >= two_pi)
        chi -= two_pi;

    float chi_ridx = CHI_TO_REAL_IDX(chi);
    /***** FIX to Floating point bug ******/
    while(chi_ridx >= _chiCount)
    {
        chi_ridx -= _chiCount;
        chi -= two_pi;
    }

    //------------------------------------//
    // calculate upper and lower indicies //
    //------------------------------------//

    int li = (int)inc_ridx;

    if (li < 0)
        li = 0;

    int hi = li + 1;
    if (hi >= _incCount)
    {
        hi = _incCount - 1;
        li = hi - 1;
    }
    float lo_inc = _incMin + li * _incStep;

    int ls = (int)spd_ridx;

    if (ls < 0)
        ls = 0;

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

//--------------------------------//
// MiscTable::GetMaxValueForSpeed //
//--------------------------------//

int
MiscTable::GetMaxValueForSpeed(
    Meas::MeasTypeE  met,
    float            inc,
    float            spd,
    float*           value)
{
    float max_value;
    GetInterpolatedValue(met, inc, spd, 0.0, &max_value);

    float dir;
    for (int dir_idx = 1; (dir = dir_idx * _chiStep) < two_pi; dir_idx++)
    {
        float trial_value;
        GetInterpolatedValue(met, inc, spd, dir, &trial_value);
        if (trial_value > max_value)
            max_value = trial_value;
    }
    *value = max_value;
    return(1);
}

//----------------------//
// MiscTable::_Allocate //
//----------------------//

int
MiscTable::_Allocate()
{
	//------------------------------------//
	// allocate the primary pointer array //
	//------------------------------------//

	_value = (float****)make_array(sizeof(float), 4, _metCount,
		_incCount, _spdCount, _chiCount);

	if (_value == NULL)
		return(0);

	return(1);
}

//------------------------//
// MiscTable::_Deallocate //
//------------------------//

int
MiscTable::_Deallocate()
{
	if (_value == NULL)
		return(1);

	free_array((void *)_value, 4, _metCount, _incCount, _spdCount, _chiCount);

	_value = NULL;
	return(1);
}

//------------------------//
// MiscTable::_ReadHeader //
//------------------------//

int
MiscTable::_ReadHeader(
	int		fd)
{
	if (read(fd, &_metCount, sizeof(int)) != sizeof(int) ||
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
// MiscTable::_ReadTable //
//-----------------------//

int
MiscTable::_ReadTable(
	int		fd)
{
	//----------------//
	// read the array //
	//----------------//

	int size = sizeof(float) * _chiCount;
	for (int i = 0; i < _metCount; i++)
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
// MiscTable::_WriteHeader //
//-------------------------//

int
MiscTable::_WriteHeader(
	int		fd)
{
	if (write(fd, &_metCount, sizeof(int) != sizeof(int)) ||
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
// MiscTable::_WriteTable //
//------------------------//

int
MiscTable::_WriteTable(
	int		fd)
{
	//-----------------//
	// write the array //
	//-----------------//

	int size = sizeof(float) * _chiCount;
	for (int i = 0; i < _metCount; i++)
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
// MiscTable::_MetToIndex //
//------------------------//

int
MiscTable::_MetToIndex(
    Meas::MeasTypeE  met)
{
	int idx = 0;
	switch(met)
	{
	case Meas::VV_MEAS_TYPE:
		idx = 0;
		break;
	case Meas::HH_MEAS_TYPE:
		idx = 1;
		break;
	case Meas::VV_HV_CORR_MEAS_TYPE:
	  idx = 2;
	  break;
	case Meas::HH_VH_CORR_MEAS_TYPE:
	  idx = 3;
	  break;
	case Meas::HV_MEAS_TYPE:
	  idx = 4;
	  break;
	case Meas::VH_MEAS_TYPE:
	  idx = 4;
	  break;
	default:
		fprintf(stderr,
			"MiscTable::_MetToIndex: invalid measurement type %d\n", met);
		exit(1);
	}
    if (idx >= _metCount)
    {
        fprintf(stderr,
        "MiscTable::_MetToIndex: measurement type %d not in model function\n",
            met);
        exit(1);
    }
	return(idx);
}

//------------------------//
// MiscTable::_IncToIndex //
//------------------------//

int
MiscTable::_IncToIndex(
	float		inc)
{
	float inc_ridx = INC_TO_REAL_IDX(inc);
	int inc_idx = (int)(inc_ridx + 0.5);
	return(inc_idx);
}

//------------------------//
// MiscTable::_SpdToIndex //
//------------------------//

int
MiscTable::_SpdToIndex(
	float		spd)
{
	float spd_ridx = SPD_TO_REAL_IDX(spd);
	int spd_idx = (int)(spd_ridx + 0.5);
	return(spd_idx);
}

//------------------------//
// MiscTable::_ChiToIndex //
//------------------------//

int
MiscTable::_ChiToIndex(
	float		chi)
{
	while (chi < 0.0) chi += two_pi;
	while (chi >= two_pi) chi -= two_pi;
	float chi_ridx = CHI_TO_REAL_IDX(chi);
        /***** FIX to Floating point bug ******/
        while(chi_ridx >= _chiCount) chi_ridx -= _chiCount;

	int chi_idx = (int)(chi_ridx + 0.5);
	chi_idx %= _chiCount;
	return(chi_idx);
}

//--------------------------//
// MiscTable::_ClipMetIndex //
//--------------------------//

int
MiscTable::_ClipMetIndex(
	int		met_idx)
{
	if (met_idx < 0)
		return(0);
	else if (met_idx >= _metCount)
		return(_metCount - 1);
	else
		return(met_idx);
}

//--------------------------//
// MiscTable::_ClipIncIndex //
//--------------------------//

int
MiscTable::_ClipIncIndex(
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
// MiscTable::_ClipSpdIndex //
//--------------------------//

int
MiscTable::_ClipSpdIndex(
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
// MiscTable::_ClipChiIndex //
//--------------------------//

int
MiscTable::_ClipChiIndex(
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
// MiscTable::_IndexToSpd //
//------------------------//

float
MiscTable::_IndexToSpd(
	int		spd_idx)
{
	return((float)spd_idx * _spdStep + _spdMin);
}

//------------------------//
// MiscTable::_IndexToChi //
//------------------------//

float
MiscTable::_IndexToChi(
	int		chi_idx)
{
	return((float)chi_idx * _chiStep);
}
