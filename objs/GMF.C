//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

static const char rcs_id_gmf_c[] =
	"@(#) $Id$";

#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <math.h>
#include "GMF.h"
#include "Meas.h"
#include "Interpolate.h"
#include "Constants.h"
#include "Beam.h"

//=====//
// GMF //
//=====//

GMF::GMF()
:	_spdTol(DEFAULT_SPD_TOL), _sepAngle(DEFAULT_SEP_ANGLE),
	_smoothAngle(DEFAULT_SMOOTH_ANGLE), _maxSolutions(DEFAULT_MAX_SOLUTIONS),
	_bestSpd(NULL), _bestObj(NULL), _copyObj(NULL)
{
	SetPhiCount(DEFAULT_PHI_COUNT);
	return;
}

GMF::~GMF()
{
	free(_bestSpd);
	free(_bestObj);
	free(_copyObj);

	return;
}

//------------------//
// GMF::SetPhiCount //
//------------------//

int
GMF::SetPhiCount(
	int		phi_count)
{
	_phiCount = phi_count;
	_phiStepSize = two_pi / _phiCount;

	if (_bestSpd)
		free(_bestSpd);
	_bestSpd = (float *)malloc(_phiCount * sizeof(float));
	if (_bestSpd == NULL)
		return(0);

	if (_bestObj)
		free(_bestObj);
	_bestObj = (float *)malloc(_phiCount * sizeof(float));
	if (_bestObj == NULL)
		return(0);

	if (_copyObj)
		free(_copyObj);
	_copyObj = (float *)malloc(_phiCount * sizeof(float));
	if (_copyObj == NULL)
		return(0);

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
	_incMin = 16.0 * dtr;
	_incMax = 66.0 * dtr;
	_incStep = 2.0 * dtr;

	_spdCount = 51;
	_spdMin = 0.0;
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
			for (int spd_idx = 1; spd_idx < _spdCount; spd_idx++)
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

	//----------------------//
	// zero the 0 m/s model //
	//----------------------//

	int spd_idx = 0;
	for (int pol_idx = 0; pol_idx < _polCount; pol_idx++)
	{
		for (int chi_idx = 0; chi_idx < _chiCount; chi_idx++)
		{
			for (int inc_idx = 0; inc_idx < _incCount; inc_idx++)
			{
				*(*(*(*(_value+pol_idx)+inc_idx)+spd_idx)+chi_idx) = 0.0;
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
	Kp*			kp)
{
	//------------------------------//
	// generate each solution curve //
	//------------------------------//

	MeasList new_list;
	for (Meas* meas = meas_list->GetHead(); meas; meas = meas_list->GetNext())
	{
		new_list.Append(meas);
		SolutionCurve(&new_list, kp);
		for (int i = 0; i < _phiCount; i++)
		{
			fprintf(ofp, "%g %g\n", (float)i * _phiStepSize * rtd,
				_bestSpd[i]);
		}
		fprintf(ofp, "&\n");
		new_list.GetHead();
		new_list.RemoveCurrent();
	}

	//----------------------------------//
	// generate combined solution curve //
	//----------------------------------//

	SolutionCurve(meas_list, kp);
	float min_obj = _bestObj[0];
	float max_obj = _bestObj[0];
	for (int i = 0; i < _phiCount; i++)
	{
		if (_bestObj[i] < min_obj)
			min_obj = _bestObj[i];
		if (_bestObj[i] > max_obj)
			max_obj = _bestObj[i];
	}
	float scale = 1.0 / (max_obj - min_obj);
	for (int i = 0; i < _phiCount; i++)
	{
		fprintf(ofp, "%g %g %g\n", (float)i * _phiStepSize * rtd,
			_bestSpd[i], (_bestObj[i] - min_obj) * scale);
	}

	//--------------------------------------//
	// generate smoothed objective function //
	//--------------------------------------//

	Smooth();
	fprintf(ofp, "&\n");
	for (int i = 0; i < _phiCount; i++)
	{
		fprintf(ofp, "%g %g\n", (float)i * _phiStepSize * rtd,
			(_bestObj[i] - min_obj) * scale);
	}

	//----------------------------//
	// write individual solutions //
	//----------------------------//

	WVC* wvc = new WVC();
	RetrieveWinds(meas_list, kp, wvc);
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

	return(1);
}

//----------------------------//
// GMF::CheckRetrieveCriteria //
//----------------------------//
// Returns 1 if wind retrieval should be performed, Otherwise 0

int
GMF::CheckRetrieveCriteria(
	MeasList*	meas_list)
{
	//------------------------------------------//
	// check for minimum number of measurements //
	//------------------------------------------//

	if (meas_list->NodeCount() < MINIMUM_WVC_MEASUREMENTS)
		return(0);

	//-------------------------------------//
	// check for minimum azimuth diversity //
	//-------------------------------------//

	Node<Meas>* current;

	for (Meas* meas1 = meas_list->GetHead(); meas1;
		meas1 = meas_list->GetNext())
	{
		// remember the current
		current = meas_list->GetCurrentNode();

		for (Meas* meas2 = meas_list->GetHead(); meas2;
			meas2 = meas_list->GetNext())
		{
			float azdiv = ANGDIF(meas1->eastAzimuth, meas2->eastAzimuth);
			if (azdiv > MINIMUM_AZIMUTH_DIVERSITY)
			{
				goto passed;
				break;
			}
		}

		// restore the current
		meas_list->SetCurrentNode(current);
	}
	return(0);		// failed diversity check

	passed:			// passed diversity check

	return(1);
}

//--------------------//
// GMF::RetrieveWinds //
//--------------------//

int
GMF::RetrieveWinds(
	MeasList*	meas_list,
	Kp*			kp,
	WVC*		wvc)
{
	//-------------------------//
	// find the solution curve //
	//-------------------------//

	if (! SolutionCurve(meas_list, kp))
	{
		fprintf(stderr, "can't find solution curve\n");
		return(0);
	}

	//---------------------------//
	// smooth the solution curve //
	//---------------------------//

	if (! Smooth())
	{
//		fprintf(stderr, "can't smooth solution curve\n");
		return(0);
	}

	//--------------------------------//
	// find maxima and add to the wvc //
	//--------------------------------//

	if (! FindMaxima(wvc))
	{
		fprintf(stderr, "can't find maxima\n");
		return(0);
	}

	//------------------------------------------//
	// sort the solutions by objective function //
	//------------------------------------------//

	wvc->SortByObj();

	return(1);
}

//--------------------//
// GMF::SolutionCurve //
//--------------------//
// For each of phi_count directions, find the speed that maximizes the
// objective function.  Fills in the _bestSpd and _bestObj arrays

int
GMF::SolutionCurve(
	MeasList*	meas_list,
	Kp*			kp)
{
	//-------------------------------//
	// bracket maxima with certainty //
	//-------------------------------//

	float ax = _spdMin;
	float cx = _spdMax;

	//-----------------------//
	// for each direction... //
	//-----------------------//

	for (int phi_idx = 0; phi_idx < _phiCount; phi_idx++)
	{
		float bx = ax + (cx - ax) * golden_r;
		float phi = (float)phi_idx * _phiStepSize;

		//------------------------------------------//
		// ...widen so that the maxima is bracketed //
		//------------------------------------------//

		if (_ObjectiveFunction(meas_list, bx, phi, kp) <
			_ObjectiveFunction(meas_list, ax, phi, kp) )
		{
			ax = _spdMin;
		}
		if (_ObjectiveFunction(meas_list, bx, phi, kp) <
			_ObjectiveFunction(meas_list, cx, phi, kp) )
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
			x2 = bx + golden_c * (cx - bx);
		}
		else
		{
			x2 = bx;
			x1 = bx - golden_c * (bx - ax);
		}
		float f1 = _ObjectiveFunction(meas_list, x1, phi, kp);
		float f2 = _ObjectiveFunction(meas_list, x2, phi, kp);

		while (x3 - x0 > _spdTol)
		{
			if (f2 > f1)
			{
				x0 = x1;
				x1 = x2;
				x2 = x2 + golden_c * (x3 - x2);
				f1 = f2;
				f2 = _ObjectiveFunction(meas_list, x2, phi, kp);
			}
			else
			{
				x3 = x2;
				x2 = x1;
				x1 = x1 - golden_c * (x1 - x0);
				f2 = f1;
				f1 = _ObjectiveFunction(meas_list, x1, phi, kp);
			}
		}

		if (f1 > f2)
		{
			_bestSpd[phi_idx] = x1;
			_bestObj[phi_idx] = f1;
		}
		else
		{
			_bestSpd[phi_idx] = x2;
			_bestObj[phi_idx] = f2;
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
GMF::Smooth()
{
	//---------------------------//
	// calculate some parameters //
	//---------------------------//

	int max_delta_phi = (int)(_sepAngle / _phiStepSize + 0.5);
	int max_smoothing_phi = (int)(_smoothAngle / _phiStepSize + 0.5);
	int delta_phi = 0;		// start with no smoothing

	int flag = 0;
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
		for (int phi_idx = 0; phi_idx < _phiCount; phi_idx++)
		{
			//--------------------------//
			// check for 3 point maxima //
			//--------------------------//

			int idx_minus = (phi_idx - 1 + _phiCount) % _phiCount;
			int idx_plus = (phi_idx + 1) % _phiCount;
			if (_bestObj[phi_idx] > _bestObj[idx_minus] &&
				_bestObj[phi_idx] > _bestObj[idx_plus])
			{
				maxima_count++;

				//------------------------//
				// check for local maxima //
				//------------------------//

				int isa_max = 1;
				for (int offset = 2; offset <= max_delta_phi; offset++)
				{
					idx_minus = (phi_idx - offset + _phiCount) % _phiCount;
					idx_plus = (phi_idx + offset) % _phiCount;
					if (_bestObj[phi_idx] < _bestObj[idx_minus] ||
						_bestObj[phi_idx] < _bestObj[idx_plus])
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
			maxima_count <= _maxSolutions)
		{
			//----------------------------------//
			// done -- scale objective function //
			//----------------------------------//

			if (flag)
			{
				// scaling was performed
				float scale = (float)(1 + delta_phi * 2);
				for (int phi_idx = 0; phi_idx < _phiCount; phi_idx++)
				{
					_bestObj[phi_idx] /= scale;
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
			if (delta_phi > max_smoothing_phi)
				break;

			//--------//
			// smooth //
			//--------//

			if (! flag)
			{
				memcpy(_copyObj, _bestObj, _phiCount * sizeof(float));
				flag = 1;
			}

			for (int phi_idx = 0; phi_idx < _phiCount; phi_idx++)
			{
				int phi_idx_plus = (phi_idx + delta_phi) % _phiCount;
				int phi_idx_minus = (phi_idx - delta_phi + _phiCount) %
					_phiCount;
				_bestObj[phi_idx] += _copyObj[phi_idx_minus];
				_bestObj[phi_idx] += _copyObj[phi_idx_plus];
			}
		}
	}
	return(return_value);
}

//-----------------//
// GMF::FindMaxima //
//-----------------//

int
GMF::FindMaxima(
	WVC*		wvc)
{
	for (int phi_idx = 0; phi_idx < _phiCount; phi_idx++)
	{
		//--------------------------//
		// check for 3 point maxima //
		//--------------------------//

		int idx_minus = (phi_idx - 1 + _phiCount) % _phiCount;
		int idx_plus = (phi_idx + 1) % _phiCount;
		if (_bestObj[phi_idx] > _bestObj[idx_minus] &&
			_bestObj[phi_idx] > _bestObj[idx_plus])
		{
			//------------//
			// add to wvc //
			//------------//

			WindVectorPlus* wvp = new WindVectorPlus();
			if (! wvp)
				return(0);
			wvp->spd = _bestSpd[phi_idx];
			wvp->dir = (float)phi_idx * _phiStepSize;
			wvp->obj = _bestObj[phi_idx];
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
	float		phi,
	Kp*			kp)
{
	float fv = 0.0;
	for (Meas* meas = meas_list->GetHead(); meas; meas = meas_list->GetNext())
	{
		float chi = phi - meas->eastAzimuth + pi;
		float gmf_value;
		GetInterpolatedValue(meas->pol, meas->incidenceAngle, spd, chi,
			&gmf_value);
		float s = gmf_value - meas->value;

		double var;
		if (kp)
		{
			if (! kp->GetVp(meas, gmf_value, meas->pol, spd, &var))
				return(0);
		}
		else
		{
			var = 1.0;
		}

		fv += s*s / var + log(var);
	}
	return(-fv);
}
