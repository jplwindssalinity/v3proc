//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// OBJECTS
//    Matrix
//
// DESCRIPTION
//    The Matrix object is used to do cool matrix things.
//
// AUTHOR
//    James N. Huddleston
//    hudd@casket.jpl.nasa.gov
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

    //--------------//
    // input/output //
    //--------------//

    int   WriteAscii(FILE* ofp);

	//--------//
	// access //
	//--------//

    int   GetElement(int index, double* value);
    void  Fill(double fill_value);
    int   CopyContents(Vector* vector);

    int   SetElement(int index, double value);
    int   GetSize()  { return(_mSize); };
	int   SetSize(int m_size);

    //------//
    // math //
    //------//

    int      Multiply(Matrix* a, Vector* b);

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
    // input/output //
    //--------------//

    int   WriteAscii(FILE* ofp);

	//--------------//
	// manipulation //
	//--------------//

    void  Fill(double fill_value);
    void  CopyContents(Matrix* a);

    int   SVD(Matrix* u, Vector* w, Matrix* v);
    int   SolveSVD(Vector* b, Vector* x);
    int   BackSubSVD(Matrix* u, Vector* w, Matrix* v, Vector* b, Vector* x);
    int   SVDFit(double* x, double* y, double* std_dev, int number_of_points,
              Vector* coefficients, int number_of_coef, Matrix* u, Matrix* v,
              Vector* w);

    int      NonlinearFit(double* x, double* y, double* std_dev,
                 int number_of_points, Vector* coefficients,
                 int* ia, Matrix* covar, double* chisq,
                 void (*funcs)(double, double *, double *, double *, int),
                 int passes);
    int      CovarianceSort(Matrix* covar, int ma, int* ia, int mfit);

    int      Marquardt(double* x, double* y, double* std_dev,
                 int number_of_points, Vector* coefficients, int* ia,
                 Matrix* alpha, Vector* beta, double* chisq,
                 void (*funcs)(double, double *, double *, double *, int));

    int      Multiply(Matrix* a, Matrix* b);
    int      Inverse(Matrix* matrix);
    int      Transpose(Matrix* matrix);

	//-----------//
	// variables //
	//-----------//

	//---------//
	// friends //
	//---------//

	friend	class Vector;

protected:

	//-----------//
	// variables //
	//-----------//

	int			_mSize;
	int			_nSize;
	double**	_matrix;
};

#endif
