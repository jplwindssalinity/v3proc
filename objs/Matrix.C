//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

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
:   _mSize(0), _vector(NULL)
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
    int  m_size)
{
    if (_mSize == m_size)
        return(1);

    Free();
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
    if (_vector != NULL)
    {
        free((char *)_vector);
        _vector = NULL;
    }
    _mSize = 0;
    return;
}

//--------------------//
// Vector::WriteAscii //
//--------------------//

int
Vector::WriteAscii(
    FILE*  ofp)
{
    fprintf(ofp, "Size = %d\n", _mSize);
    for (int i = 0; i < _mSize; i++)
    {
        fprintf(ofp, " %9g", _vector[i]);
    }
    fprintf(ofp, "\n");
    return(1);
}

//--------------------//
// Vector::GetElement //
//--------------------//

int
Vector::GetElement(
    int      index,
    double*  value)
{
    if (index < 0 || index >= _mSize)
        return(0);
    *value = _vector[index];
    return(1);
}

//--------------//
// Vector::Fill //
//--------------//

void
Vector::Fill(
    double  value)
{
    for (int i = 0; i < _mSize; i++)
        _vector[i] = value;
    return;
}

//----------------------//
// Vector::CopyContents //
//----------------------//

int
Vector::CopyContents(
    Vector*  vector)
{
    int use_m = MIN(_mSize, vector->_mSize);
    for (int i = 0; i < use_m; i++)
    {
        _vector[i] = vector->_vector[i];
    }
    return(1);
}

//--------------------//
// Vector::SetElement //
//--------------------//

int
Vector::SetElement(
    int     index,
    double  value)
{
    if (index < 0 || index >= _mSize)
        return(0);
    _vector[index] = value;
    return(1);
}

//-----------------//
// Vector::SetSize //
//-----------------//

int 
Vector::SetSize(
    int  m_size)
{
  if (m_size > _mSize)
    return(0);

  _mSize = m_size;
  return(1);
}

//------------------//
// Vector::Multiply //
//------------------//

int
Vector::Multiply(
    Matrix*  a,
    Vector*  b)
{
    if (a->_nSize != b->_mSize)
        return(0);

    if (! Allocate(a->_mSize))
        return(0);

    double** am = a->_matrix;
    double* bv = b->_vector;

    for (int i = 0; i < _mSize; i++)
    {
        _vector[i] = 0.0;
        for (int ii = 0; ii < a->_nSize; ii++)
        {
            _vector[i] += am[i][ii] * bv[ii];
        }
    }
    return(1);
}


//========//
// Matrix //
//========//

Matrix::Matrix()
:   _mSize(0), _nSize(0), _matrix(NULL)
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
    int  m_size,
    int  n_size)
{
    if (_mSize == m_size && _nSize == n_size)
        return(1);

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
            {
                if (*(_matrix + j) != NULL)
                    free((char *)*(_matrix + j));
            }
            if (_matrix != NULL)
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
        if (*(_matrix + i) != NULL)
            free((char *)*(_matrix + i));
    }
 
    //----------------------------//
    // free the array of pointers //
    //----------------------------//
 
    if (_matrix != NULL)
        free((char *)_matrix);
 
    //---------------//
    // set variables //
    //---------------//
 
    _matrix = NULL;
    _mSize = 0;
    _nSize = 0;
 
    return;
}

//--------------------//
// Matrix::WriteAscii //
//--------------------//

int
Matrix::WriteAscii(
    FILE*  ofp)
{
    fprintf(ofp, "Size = %d x %d\n", _mSize, _nSize);
    for (int i = 0; i < _mSize; i++)
    {
        for (int j = 0; j < _nSize; j++)
        {
            fprintf(ofp, " %9g", _matrix[i][j]);
        }
        fprintf(ofp, "\n");
    }
    return(1);
}

//--------------//
// Matrix::Fill //
//--------------//

void
Matrix::Fill(
    double  fill_value)
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
    Matrix*  a)
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
// NOTE: Rows >= columns

/* computes sqrt(a*a + b*b) without destructive overflow or underflow */
static double at, bt, ct;
#define pythag(A, B) ((at=fabs(A)) > (bt=fabs(B)) ? \
  (ct=bt/at, at*sqrt(1.0+ct*ct)) : (bt ? (ct=at/bt, bt*sqrt(1.0+ct*ct)): 0.0))

#define sign(A,B) ((B) >= 0.0 ? fabs(A) : -fabs(A))

int
Matrix::SVD(
    Matrix*  u,
    Vector*  w,
    Matrix*  v)
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
    if (! rv.Allocate(_nSize))
        return(0);

    double* rva = rv._vector;

    //----------------------------------------------------------//
    // create the w vector and the u and v matricies (returned) //
    //----------------------------------------------------------//

    if (! w->Allocate(_nSize))
        return(0);
    double* wva = w->_vector;

    if (! u->Allocate(this->_mSize, this->_nSize))
        return(0);
    u->CopyContents(this);
    double** uma = u->_matrix;

    if (! v->Allocate(_nSize, _nSize))
        return(0);
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
    Vector*  b,
    Vector*  x)
{
    Matrix extendm;
    if (_mSize < _nSize)    // underdetermined
    {
        if (! extendm.Allocate(_nSize, _nSize))
            return(0);
    }
    else
    {
        if (! extendm.Allocate(_mSize, _nSize))
            return(0);
    }

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
    {
        if (! x->Allocate(_nSize))
            return(0);
    }

    //------------------//
    // threshold values //
    //------------------//

    double wmax = 0.0;
    for (int i = 0; i < _nSize; i++)
    {
        if (w._vector[i] > wmax)
            wmax = w._vector[i];
    }
    double wmin = wmax * 1E-06;
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
	if (! tmp.Allocate(_nSize))
        return(0);

	for (int j = 0; j < _nSize; j++)
	{
		double s = 0.0;
		if (w->_vector[j])
		{
			for (int i = 0; i < b->_mSize; i++)
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

int
Matrix::SVDFit(
    double*  x,
    double*  y,
    double*  std_dev,
    int      number_of_points,
    Vector*  coefficients,
    int      number_of_coef,
    Matrix*  u,
    Matrix*  v,
    Vector*  w)
{
	//--------------------------//
	// create temporary vectors //
	//--------------------------//

	Vector b;
	if (! b.Allocate(number_of_points))
        return(0);

	Vector afunc;
	if (! afunc.Allocate(number_of_coef))
        return(0);

	//-----------------------------------------------//
	// accumulate coefficients of the fitting matrix //
	//-----------------------------------------------//

	Matrix a;
	if (! a.Allocate(number_of_points, number_of_coef))
        return(0);
	for (int i = 0; i < number_of_points; i++)
	{
		//---------------------//
		// polynomial function //
		//---------------------//

		afunc._vector[0] = 1.0;
		for (int j = 1; j < number_of_coef; j++)
			afunc._vector[j] = afunc._vector[j-1] * x[i];

		double tmp;
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

	return(1);
}

//----------------------//
// Matrix::NonlinearFit //
//----------------------//

#define THRESHOLD_FRACTION  1E-3
#define LAMBDA_FACTOR		2.0

int
Matrix::NonlinearFit(
    double*  x,
    double*  y,
    double*  std_dev,
    int      number_of_points,
    Vector*  coefficients,
    int*     ia,
    Matrix*  covar,
    double*  chi_2,
    void     (*funcs)(double, double *, double *, double *, int),
    int      passes)
{
    //-------------------------------------//
    // create needed matricies and vectors //
    //-------------------------------------//

    int ma = coefficients->GetSize();
    int mfit = 0;
    int j;
    for (j = 0; j < ma; j++)
    {
        if (ia[j])
            mfit++;
    }

    Vector oneda;
    if (! oneda.Allocate(mfit))
        return(0);
    double* onedav = oneda._vector;

    Matrix alpha;
    if (! alpha.Allocate(ma, ma))
        return(0);
    double** alpham = alpha._matrix;

    Vector atry;
    if (! atry.Allocate(ma))
        return(0);
    double* atryv = atry._vector;

    Vector da;
    if (! da.Allocate(ma))
        return(0);
    double* dav = da._vector;

    Vector beta;
    if (! beta.Allocate(ma))
        return(0);
    double* betav = beta._vector;

    if (! covar->Allocate(ma, ma))
        return(0);
    double** covarm = covar->_matrix;

    double* coefv = coefficients->_vector;

    //-----------------------------//
    // initialize convergance loop //
    //-----------------------------//

    double lambda = 0.001;
    Marquardt(x, y, std_dev, number_of_points, coefficients, ia, &alpha,
        &beta, chi_2, funcs);
    double old_chi_2 = *chi_2;
    for (int i = 0; i < ma; i++)
    {
        atryv[i] = coefv[i];
    }

    //------------------//
    // convergance loop //
    //------------------//

    for (int pass = 0; pass < passes; pass++)
    {
        j = 0;
        for (int l = 0; l < ma; l++)
        {
            if (ia[l])
            {
                int k = 0;
                for (int m = 0; m < ma; m++)
                {
                    if (ia[m])
                    {
                        covarm[j][k] = alpham[j][k];
                        k++;
                    }
                }
                covarm[j][j] = alpham[j][j] * (1.0 + lambda);
                onedav[j] = betav[j];
                j++;
            }
        }

        Vector new_oneda;
        covar->SolveSVD(&oneda, &new_oneda);
        oneda.CopyContents(&new_oneda);
        covar->Inverse(covar);

        for (j = 0; j < mfit; j++)
        {
            dav[j] = onedav[j];
        }

        if (lambda == 0.0)
        {
            CovarianceSort(covar, ma, ia, mfit);
            return(1);
        }

        j = 0;
        for (int l = 0; l < ma; l++)
        {
            if (ia[l])
            {
                atryv[l] = coefv[l] + dav[j];
                j++;
            }
        }

        Marquardt(x, y, std_dev, number_of_points, &atry, ia, covar, &da,
            chi_2, funcs);

        if (*chi_2 < old_chi_2)
        {
            lambda /= LAMBDA_FACTOR;
            old_chi_2 = *chi_2;
            int j = 0;
            for (int l = 0; l < ma; l++)
            {
                if (ia[l])
                {
                    int k = 0;
                    for (int m = 0; m < ma; m++)
                    {
                        if (ia[m])
                        {
                            alpham[j][k] = covarm[j][k];
                            k++;
                        }
                    }
                    betav[j] = dav[j];
                    coefv[l] = atryv[l];
                    j++;
                }
            }
        }
        else
        {
            lambda *= LAMBDA_FACTOR;
            *chi_2 = old_chi_2;
        }
    }

    return(1);
}

//------------------------//
// Matrix::CovarianceSort //
//------------------------//

#define SWAP(a,b) { swap = (a); (a) = (b); (b) = swap; }

int
Matrix::CovarianceSort(
    Matrix*  covar,
    int      ma,
    int*     ia,
    int      mfit)
{
    double** covarm = covar->_matrix;

    for (int i = mfit; i < ma; i++)
    {
        for (int j = 0; j <= i; j++)
        {
            covarm[i][j] = covarm[j][i] = 0.0;
        }
    }

    int k = mfit - 1;
    double swap;
    for (int j = ma - 1; j >= 0; j--)
    {
        if (ia[j])
        {
            for (int i = 0; i < ma; i++)
            {
                SWAP(covarm[i][k], covarm[i][j]);
            }
            for (int i = 0; i < ma; i++)
            {
                SWAP(covarm[k][i], covarm[j][i]);
            }
            k--;
        }
    }

    return(1);
}

//-------------------//
// Matrix::Marquardt //
//-------------------//

int
Matrix::Marquardt(
    double*  x,
    double*  y,
    double*  std_dev,
    int      number_of_points,
    Vector*  coefficients,
    int*     ia,
    Matrix*  alpha,
    Vector*  beta,
    double*  chi_2,
    void     (*funcs)(double, double * , double *, double *, int))
{
    int ma = coefficients->_mSize;

    Vector dyda;
    if (! dyda.Allocate(ma))
        return(0);
    double* dydav = dyda._vector;

    double** alpham = alpha->_matrix;
    double* betav = beta->_vector;

    //---------------------------//
    // initialize alpha and beta //
    //---------------------------//

    int mfit = 0;
    for (int j = 0; j < ma; j++)
    {
        if (ia[j])
            mfit++;
    }
    for (int j = 0; j < mfit; j++)
    {
        for (int k = 0; k <= j; k++)
        {
            alpham[j][k] = 0.0;
        }
        betav[j] = 0.0;
    }

    //-----------//
    // summation //
    //-----------//

    *chi_2 = 0.0;
    double* av = coefficients->_vector;
    double ymod;
    for (int i = 0; i < number_of_points; i++)
    {
        (*funcs)(x[i], av, &ymod, dydav, ma);
        double sig2i;
        if (std_dev == NULL)
            sig2i = 1.0;
        else
            sig2i = 1.0 / (std_dev[i] * std_dev[i]);

        double dy = y[i] - ymod;
        int j = 0;
        for (int l = 0; l < ma; l++)
        {
            if (ia[l])
            {
                double wt = dydav[l] * sig2i;
                int k = 0;
                for (int m = 0; m <= l; m++)
                {
                    if (ia[m])
                    {
                        alpham[j][k] += wt * dydav[m];
                        k++;
                    }
                    betav[j] += dy * wt;
                }
                j++;
            }
        }
        (*chi_2) += dy * dy * sig2i;
    }
    for (int j = 1; j < mfit; j++)
    {
        for (int k = 0; k < j; k++)
        {
            alpham[k][j] = alpham[j][k];
        }
    }
    dyda.Free();

    return(1);
}

//-----------------//
// Matrix::Inverse //
//-----------------//

int
Matrix::Inverse(
    Matrix*  matrix)
{
    Matrix u, v;
    Vector w;

    if (! matrix->SVD(&u, &w, &v))
        return(0);

    for (int i = 0; i < w._mSize; i++)
    {
        if (fabs(w._vector[i]) < 1E-06)
            w._vector[i] = 0.0;
        else
            w._vector[i] = 1.0 / w._vector[i];
    }

    //--------------------------//
    // do the diagonal multiply //
    //--------------------------//

    Matrix vw;
    if (! vw.Allocate(v._mSize, w._mSize))
        return(0);

    double** am = v._matrix;
    double* bv = w._vector;
    double** vwm = vw._matrix;

    for (int i = 0; i < _mSize; i++)
    {
        for (int j = 0; j < _nSize; j++)
        {
            vwm[i][j] = am[i][j] * bv[j];
        }
    }

    Matrix utrans;
    utrans.Transpose(&u);

    Multiply(&vw, &utrans);
    return(1);
}

//------------------//
// Matrix::Multiply //
//------------------//

int
Matrix::Multiply(
    Matrix*  a,
    Matrix*  b)
{
    if (a->_nSize != b->_mSize)
        return(0);

    if (! Allocate(a->_mSize, b->_nSize))
        return(0);

    double** am = a->_matrix;
    double** bm = b->_matrix;

    for (int i = 0; i < _mSize; i++)
    {
        for (int j = 0; j < _nSize; j++) 
        {
            _matrix[i][j] = 0.0;
            for (int ii = 0; ii < a->_nSize; ii++)
            {
                _matrix[i][j] += am[i][ii] * bm[ii][j];
            }
        }
    }
    return(1);
}

//-------------------//
// Matrix::Transpose //
//-------------------//

int
Matrix::Transpose(
    Matrix*  matrix)
{
    //-----------------------------//
    // create the transpose matrix //
    //-----------------------------//

    if (! Allocate(matrix->_nSize, matrix->_mSize))
        return(0);

    //-----------//
    // transpose //
    //-----------//

    for (int i = 0; i < _mSize; i++)
    {
        for (int j = 0; j < _nSize; j++)
        {
            _matrix[i][j] = matrix->_matrix[j][i];
        }
    }

    return(1);
}
