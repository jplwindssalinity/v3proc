//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// OBJECTS
//		Matrix
//
// DESCRIPTION
//		The Matrix object is used to do cool matrix things.
//
// AUTHOR
//		James N. Huddleston
//		hudd@acid.jpl.nasa.gov
//----------------------------------------------------------------------

#ifndef MATRIX_H
#define MATRIX_H

static const char rcs_id_matrix_h[] =
	"@(#) $Id$";

//========//
// Vector //
//========//

class Matrix;

class Vector
{
public:

	//--------------//
	// construction //
	//--------------//

	Vector();
	~Vector();

	int		Allocate(int m_size);
	void	Free();

	//--------//
	// access //
	//--------//

	int		GetElement(int index, double* value);

	//---------//
	// friends //
	//---------//

	friend	class Matrix;

protected:

	//-----------//
	// variables //
	//-----------//

	int			_mSize;
	double*		_vector;
};

//========//
// Matrix //
//========//

class Matrix
{
public:

	//--------------//
	// construction //
	//--------------//

	Matrix();
	~Matrix();

	int		Allocate(int m_size, int n_size);
	void	Free();

	//--------------//
	// manipulation //
	//--------------//

	void	Fill(double fill_value);
	void	CopyContents(Matrix* a);

	int		SVD(Matrix* u, Vector* w, Matrix* v);
	int		SolveSVD(Vector* b, Vector* x);
	int		BackSubSVD(Matrix* u, Vector* w, Matrix* v, Vector* b, Vector* x);
	void	SVDFit(float* x, float* y, float* std_dev, int number_of_points,
				Vector* coefficients, int number_of_coef, Matrix* u, Matrix* v,
				Vector* w);

/*
	int		NonlinearFit(float* x, float* y, float* std_dev,
				int number_of_points, Vector* coefficients, int number_of_coef,
				Matrix* covar, Matrix* alpha, void (*funcs)());
	int		Marquardt();
	int		Inverse();
*/

	//-----------//
	// variables //
	//-----------//

protected:

	//-----------//
	// variables //
	//-----------//

	int			_mSize;
	int			_nSize;
	double**	_matrix;
};

#endif
