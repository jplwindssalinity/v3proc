//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_matrix_c[] =
	"@(#) $Id$";

#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include "Matrix.h"
#include "Misc.h"

//========//
// Vector //
//========//

Vector::Vector()
:	_mSize(0), _vector(NULL)
{
	return;
}

Vector::~Vector()
{
	Free();
	return;
}

//------------------//
// Vector::Allocate //
//------------------//

int
Vector::Allocate(
	int		m_size)
{
	Free();		// free just in case

    _vector = (double *)malloc(m_size * sizeof(double));
    if (_vector == NULL)
        return(0);
    _mSize = m_size;
    return(1);
}

//--------------//
// Vector::Free //
//--------------//

void
Vector::Free()
{
	free((char *)_vector);
	_vector = NULL;
	_mSize = 0;
	return;
}

//--------------------//
// Vector::GetElement //
//--------------------//

int
Vector::GetElement(
	int			index,
	double*		value)
{
	if (index < 0 || index >= _mSize)
		return(0);
	*value = _vector[index];
	return(1);
}

//========//
// Matrix //
//========//

Matrix::Matrix()
:	_mSize(0), _nSize(0), _matrix(NULL)
{
	return;
}

Matrix::~Matrix()
{
	Free();
	return;
}

//------------------//
// Matrix::Allocate //
//------------------//

int
Matrix::Allocate(
	int		m_size,
	int		n_size)
{
	//-------------------//
	// free just in case //
	//-------------------//

	Free();

	//------------------------------------//
	// allocate for the array of pointers //
	//------------------------------------//
 
	_matrix = (double **)malloc(m_size * sizeof(double *));
	if (_matrix == NULL)
		return(0);
 
	//------------------------------//
	// allocate memory for each row //
	//------------------------------//
 
	unsigned int row_size = sizeof(double) * n_size;
	for (int i = 0; i < m_size; i++)
	{
		double* row = (double *)malloc(row_size);
		if (row == NULL)
		{
			for (int j = 0; j < i; j++)
				free((char *)*(_matrix + j));
			free((char *)_matrix);
			return(0);
		}
		*(_matrix + i) = row;
	}
 
	//---------------//
	// set variables //
	//---------------//
 
	_mSize = m_size;
	_nSize = n_size;
 
	return(1);
}

//--------------//
// Matrix::Free //
//--------------//

void
Matrix::Free()
{
	if (_matrix == NULL)
		return;
 
	//---------------//
	// free each row //
	//---------------//
 
	for (int i = 0; i < _mSize; i++)
	{
		free((char *)*(_matrix + i));
	}
 
	//----------------------------//
	// free the array of pointers //
	//----------------------------//
 
	free((char *)_matrix);
 
	//---------------//
	// set variables //
	//---------------//
 
	_matrix = NULL;
	_mSize = 0;
	_nSize = 0;
 
	return;
}

//--------------//
// Matrix::Fill //
//--------------//

void
Matrix::Fill(
	double		fill_value)
{
	for (int i = 0; i < _mSize; i++)
	{
		for (int j = 0; j < _nSize; j++)
		{
			_matrix[i][j] = fill_value;
		}
	}
	return;
}

//----------------------//
// Matrix::CopyContents //
//----------------------//
// copies as much of the contents as possible

void
Matrix::CopyContents(
	Matrix*		a)
{
	int min_msize = MIN(_mSize, a->_mSize);
	int min_nsize = MIN(_nSize, a->_nSize);
	for (int i = 0; i < min_msize; i++)
	{
		for (int j = 0; j < min_nsize; j++)
		{
			_matrix[i][j] = a->_matrix[i][j];
		}
	}
	return;
}

//-------------//
// Matrix::SVD //
//-------------//
// decomposes the matrix (A) into U, W, and V such that UWV' = A
// returns 1 on success, 0 on failure
// the diagonal matrix of singular values is placed in W.
// NOTE: A must have more rows than columns

/* computes sqrt(a*a + b*b) without destructive overflow or underflow */
static double at, bt, ct;
#define pythag(A, B) ((at=fabs(A)) > (bt=fabs(B)) ? \
  (ct=bt/at, at*sqrt(1.0+ct*ct)) : (bt ? (ct=at/bt, bt*sqrt(1.0+ct*ct)): 0.0))

#define sign(A,B) ((B) >= 0.0 ? fabs(A) : -fabs(A))

int
Matrix::SVD(
	Matrix*		u,
	Vector*		w,
	Matrix*		v)
{
	//----------------//
	// check the size //
	//----------------//

	if (_mSize < _nSize)
		return(0);

	//---------------------------------//
	// create the r vector (temporary) //
	//---------------------------------//

	Vector rv;
	rv.Allocate(_nSize);
	double* rva = rv._vector;

	//----------------------------------------------------------//
	// create the w vector and the u and v matricies (returned) //
	//----------------------------------------------------------//

	w->Allocate(_nSize);
	double* wva = w->_vector;
	u->Allocate(this->_mSize, this->_nSize);
	u->CopyContents(this);
	double** uma = u->_matrix;
	v->Allocate(_nSize, _nSize);
	double** vma = v->_matrix;

	//---------------------------------------------//
	// householder reduction to bidirectional form //
	//---------------------------------------------//

	double scale = 0.0;
	double g = 0.0;
	double anorm = 0.0;
	double s, f, h;
	int i, j, k;
	int l = 0;
	for (i = 0; i < _nSize; i++)
	{
		l = i + 1;
		rva[i] = scale * g;
		g = s = scale = 0.0;
		if (i < _mSize)
		{
			for (k = i; k < _mSize; k++)
			{
				scale += fabs(uma[k][i]);
			}
			if (scale)
			{
				for (k = i; k < _mSize; k++)
				{
					uma[k][i] /= scale;
					s += uma[k][i] * uma[k][i];
				}
				f = uma[i][i];
				g = -sign(sqrt(s), f);
				h = f * g - s;
				uma[i][i] = f - g;
				if (i != _nSize - 1)
				{
					for (j = l; j < _nSize; j++)
					{
						for (s = 0.0, k = i; k < _mSize; k++)
						{
							s += uma[k][i] * uma[k][j];
						}
						f = s / h;
						for (k = i; k < _mSize; k++)
						{
							uma[k][j] += f * uma[k][i];
						}
					}
				}
				for (k = i; k < _mSize; k++)
				{
					uma[k][i] *= scale;
				}
			}
		}
		wva[i] = scale * g;
		g = s = scale = 0.0;
		if (i < _mSize && i != _nSize - 1)
		{
			for (k = l; k < _nSize; k++)
			{
				scale += fabs(uma[i][k]);
			}
			if (scale)
			{
				for (k = l; k < _nSize; k++)
				{
					uma[i][k] /= scale;
					s += uma[i][k] * uma[i][k];
				}
				f = uma[i][l];
				g = -sign(sqrt(s), f);
				h = f * g - s;
				uma[i][l] = f - g;
				for (k = l; k < _nSize; k++)
				{
					rva[k] = uma[i][k] / h;
				}
				if (i != _mSize - 1)
				{
					for (j = l; j < _mSize; j++)
					{
						for (s = 0.0, k = l; k < _nSize; k++)
						{
							s += uma[j][k] * uma[i][k];
						}
						for (k = l; k < _nSize; k++)
						{
							uma[j][k] += s * rva[k];
						}
					}
				}
				for (k = l; k < _nSize; k++)
				{
					uma[i][k] *= scale;
				}
			}
		}
		anorm = MAX(anorm, (fabs(wva[i]) + fabs(rva[i])));
	}

	//--------------------------------------------//
	// accumulation of right-hand transformations //
	//--------------------------------------------//

	for (i = _nSize - 1; i >= 0; i--)
	{
		if (i < _nSize - 1)
		{
			if (g)
			{
				for (j = l; j < _nSize; j++)
				{
					vma[j][i] = (uma[i][j] / uma[i][l]) / g;
				}
				for (j = l; j < _nSize; j++)
				{
					for (s = 0.0, k = l; k < _nSize; k++)
					{
						s += uma[i][k] * vma[k][j];
					}
					for (k = l; k < _nSize; k++)
					{
						vma[k][j] += s * vma[k][i];
					}
				}
			}
			for (j = l; j < _nSize; j++)
			{
				vma[i][j] = vma[j][i] = 0.0;
			}
		}
		vma[i][i] = 1.0;
		g = rva[i];
		l = i;
	}

	//-------------------------------------------//
	// accumulation of left-hand transformations //
	//-------------------------------------------//

	for (i = _nSize - 1; i >= 0; i--)
	{
		l = i + 1;
		g = wva[i];
		if (i < _nSize - 1)
		{
			for (j = l; j < _nSize; j++)
			{
				uma[i][j] = 0.0;
			}
		}
		if (g)
		{
			g = 1.0 / g;
			if (i != _nSize - 1)
			{
				for (j = l; j < _nSize; j++)
				{
					for (s = 0.0, k = l; k < _mSize; k++)
					{
						s += uma[k][i] * uma[k][j];
					}
					f = (s / uma[i][i]) * g;
					for (k = i; k < _mSize; k++)
					{
						uma[k][j] += f * uma[k][i];
					}
				}
			}
			for (j = i; j < _mSize; j++)
			{
				uma[j][i] *= g;
			}
		}
		else
		{
			for (j = i; j < _mSize; j++)
			{
				uma[j][i] = 0.0;
			}
		}
		++uma[i][i];
	}

	//----------------------------------------//
	// diagonalization of the bidiagonal form //
	//----------------------------------------//

	int nm = 0;
	double c, x, y, z;
	for (k = _nSize - 1; k >= 0; k--)
	{
		for (int its = 0; its < 30; its++)
		{
			int flag = 1;
			for (l = k; l >= 0; l--)
			{
				nm = l - 1;
				if ((double) (fabs(rva[l]) + anorm) == anorm)
				{
					flag = 0;
					break;
				}
				if ((double) (fabs(wva[nm]) + anorm) == anorm)
				{
					break;
				}
			}
			if (flag)
			{
				c = 0.0;
				s = 1.0;
				for (i = l; i <= k; i++)
				{
					f = s * rva[i];
					rva[i] = c * rva[i];
					if ((double) (fabs(f) + anorm) == anorm)
					{
						break;
					}
					g = wva[i];
					h = pythag(f, g);
					wva[i] = h;
					h = 1.0 / h;
					c = g * h;
					s = (-f * h);
					for (j = 0; j < _mSize; j++)
					{
						y = uma[j][nm];
						z = uma[j][i];
						uma[j][nm] = y * c + z * s;
						uma[j][i] = z * c - y * s;
					}
				}
			}
			z = wva[k];
			if (l == k)
			{
				if (z < 0.0)
				{
					wva[k] = -z;
					for (j = 0; j < _nSize; j++)
					{
						vma[j][k] = (-vma[j][k]);
					}
				}
				break;
			}
			if (its == 29)
			{
				return(0);
			}
			x = wva[l];
			nm = k - 1;
			y = wva[nm];
			g = rva[nm];
			h = rva[k];
			f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
			g = pythag(f, 1.0);
			f = ((x - z) * (x + z) + h * ((y / (f + sign(g, f))) - h)) / x;

			//------------------------//
			// next QR transformation //
			//------------------------//

			c = s = 1.0;
			for (j = l; j <= nm; j++)
			{
				i = j + 1;
				g = rva[i];
				y = wva[i];
				h = s * g;
				g = c * g;
				z = pythag(f, h);
				rva[j] = z;
				c = f / z;
				s = h / z;
				f = x * c + g * s;
				g = g * c - x * s;
				h = y * s;
				y = y * c;
				for (int jj = 0; jj < _nSize; jj++)
				{
					x = vma[jj][j];
					z = vma[jj][i];
					vma[jj][j] = x * c + z * s;
					vma[jj][i] = z * c - x * s;
				}
				z = pythag(f, h);
				wva[j] = z;
				if (z)
				{
					z = 1.0 / z;
					c = f * z;
					s = h * z;
				}
				f = (c * g) + (s * y);
				x = (c * y) - (s * g);
				for (int jj = 0; jj < _mSize; jj++)
				{
					y = uma[jj][j];
					z = uma[jj][i];
					uma[jj][j] = y * c + z * s;
					uma[jj][i] = z * c - y * s;
				}
			}
			rva[l] = 0.0;
			rva[k] = f;
			wva[k] = x;
		}
	}
	return(1);
}

//----------//
// SolveSVD //
//----------//
// Returns the vector X where AX ~= b
// small W terms are zeroed

int
Matrix::SolveSVD(
	Vector*		b,
	Vector*		x)
{
	Matrix extendm;
	if (_mSize < _nSize)	// underdetermined
		extendm.Allocate(_nSize, _nSize);
	else
		extendm.Allocate(_mSize, _nSize);

	extendm.Fill(0.0);
	extendm.CopyContents(this);

	//-----------//
	// decompose //
	//-----------//

	Matrix u, v;
	Vector w;
	if (! extendm.SVD(&u, &w, &v))
		return(0);

	if (x->_mSize != _nSize)
		x->Allocate(_nSize);

	//------------------//
	// threshold values //
	//------------------//

	double wmax = 0.0;
	for (int i = 0; i < _nSize; i++)
	{
		if (w._vector[i] > wmax)
			wmax = w._vector[i];
	}
	double wmin = wmax * 1e-06;
	for (int i = 0; i < _nSize; i++)
	{
		if (w._vector[i] < wmin)
			w._vector[i] = 0.0;
	}

	//-----------------//
	// back-substitute //
	//-----------------//

	if (! extendm.BackSubSVD(&u, &w, &v, b, x))
		return(0);

	return(1);
}

//------------//
// BackSubSVD //
//------------//

int
Matrix::BackSubSVD(
	Matrix*		u,
	Vector*		w,
	Matrix*		v,
	Vector*		b,
	Vector*		x)
{
	Vector tmp;
	tmp.Allocate(_nSize);

	for (int j = 0; j < _nSize; j++)
	{
		double s = 0.0;
		if (w->_vector[j])
		{
			for (int i = 0; i < _mSize; i++)
			{
				s += u->_matrix[i][j] * b->_vector[i];
			}
			s /= w->_vector[j];
		}
		tmp._vector[j] = s;
	}

	for (int j = 0; j < _nSize; j++)
	{
		double s = 0.0;
		for (int jj = 0; jj < _nSize; jj++)
		{
			s += v->_matrix[j][jj] * tmp._vector[jj];
			x->_vector[j] = s;
		}
	}

	return(1);
}

//----------------//
// Matrix::SVDFit //
//----------------//

#define TOL		1.0E-5

void
Matrix::SVDFit(
	float*		x,
	float*		y,
	float*		std_dev,
	int			number_of_points,
	Vector*		coefficients,
	int			number_of_coef,
	Matrix*		u,
	Matrix*		v,
	Vector*		w)
{
	//--------------------------//
	// create temporary vectors //
	//--------------------------//

	Vector b;
	b.Allocate(number_of_points);

	Vector afunc;
	afunc.Allocate(number_of_coef);

	//-----------------------------------------------//
	// accumulate coefficients of the fitting matrix //
	//-----------------------------------------------//

	Matrix a;
	a.Allocate(number_of_points, number_of_coef);
	for (int i = 0; i < number_of_points; i++)
	{
		//---------------------//
		// polynomial function //
		//---------------------//

		afunc._vector[0] = 1.0;
		for (int j = 1; j < number_of_coef; j++)
			afunc._vector[j] = afunc._vector[j-1] * x[i];

		float tmp;
		if (std_dev)
			tmp = 1.0 / std_dev[i];
		else
			tmp = 1.0;

		for (int j = 0; j < number_of_coef; j++)
		{
			a._matrix[i][j] = afunc._vector[j] * tmp;
		}
		b._vector[i] = y[i] * tmp;
	}

	//------------------------------//
	// singular value decomposition //
	//------------------------------//

	a.SVD(u, w, v);

	//--------------------------//
	// zero the singular values //
	//--------------------------//

	double wmax = 0.0;
	for (int j = 0; j < number_of_coef; j++)
	{
		if (w->_vector[j] > wmax)
			wmax = w->_vector[j];
	}
	double thresh = TOL * wmax;
	for (int j = 0; j < number_of_coef; j++)
	{
		if (w->_vector[j] < thresh)
			w->_vector[j] = 0.0;
	}

	//----------------//
	// backsubstitute //
	//----------------//

	u->BackSubSVD(u, w, v, &b, coefficients);

	return;
}
