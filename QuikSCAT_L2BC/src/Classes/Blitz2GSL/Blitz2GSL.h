/*!
  \file Blitz2GSL.h
  \author E. Rodriguez
  \brief Give access to gsl routines of Blitz arrays by providing vector and matrix views.

  Give access to gsl routines of Blitz arrays by providing vector and matrix views.
  Also implement linear algebra and least squares routines.
*/
#ifndef _ER_BLITZ2GSL_H_
#define _ER_BLITZ2GSL_H_

#include <blitz/array.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_multifit.h>

//! Get a GSL vector view from a blitz vector

inline gsl_vector_view blitz2gsl_vector(blitz::Array<double, 1>& v) {
  return gsl_vector_view_array(v.data(), v.extent(blitz::firstDim));
}

//! Get a GSL matrix view from a blitz 2D array

inline gsl_matrix_view blitz2gsl_matrix(blitz::Array<double, 2>& m) {
  return gsl_matrix_view_array(m.data(), m.extent(blitz::firstDim), 
			       m.extent(blitz::secondDim));
}

/*!
  Solve by LU decompostion of AX = B. The solution is not in place.
*/

int LU_solve(blitz::Array<double, 2>& A, //!< Matrix to invert
	     blitz::Array<double, 1>& B, //!< RHS
 	     blitz::Array<double, 1>& X  //!< Solution vector
	     );

/*!
  Solve by LU decompostion of AX = B. The solution is in place.
*/

int LU_svx(blitz::Array<double, 2>& A, //!< Matrix to invert
	   blitz::Array<double, 1>& X //!< On input B, on output X
	   );

/*!
  Compute the inverse of a matrix by LUD.
*/

int LU_inverse(blitz::Array<double, 2>& A, //!< Matrix to invert
	       blitz::Array<double, 2>& Inverse //!< Inverse matrix
	       );

/*!
  Compute the determinant by LUD. The input matrix is change to the LUD.
*/

double LU_det(blitz::Array<double, 2>& A); //!< Matrix to find determinat

/*!
  This function factorizes the M-by-N matrix A into the singular value decomposition 
  A = U S V^T for M >= N. On output the matrix A is replaced by U. The diagonal elements 
  of the singular value matrix S are stored in the vector S. The singular values are 
  non-negative and form a non-increasing sequence from S_1 to S_N. The matrix V contains 
  the elements of V in untransposed form. To form the product U S V^T it is necessary to 
  take the transpose of V. A workspace of length N is required in work.
  This routine uses the Golub-Reinsch SVD algorithm.
*/

int SV_decomp(blitz::Array<double, 2>& A, //!< Matrix to decompose 
	      blitz::Array<double, 2>& V, //!< Rotation matrix
	      blitz::Array<double, 1>& S  //!< Singular values
	      );

/*!
  This function computes the SVD using the modified Golub-Reinsch algorithm, which is 
  faster for M>>N. It requires the vector work of length N and the N-by-N matrix X as 
  additional working space.
*/

int SV_decomp_mod(blitz::Array<double, 2>& A, //!< Matrix to decompose 
		  blitz::Array<double, 2>& V, //!< Rotation matrix
		  blitz::Array<double, 1>& S  //!< Singular values
		  );

/*!
  This function computes the SVD of the M-by-N matrix A using one-sided Jacobi 
  orthogonalization for M >= N. The Jacobi method can compute singular values to 
  higher relative accuracy than Golub-Reinsch algorithms (see references for details).
*/

int SV_decomp_jacobi(blitz::Array<double, 2>& A, //!< Matrix to decompose 
		     blitz::Array<double, 2>& V, //!< Rotation matrix
		     blitz::Array<double, 1>& S  //!< Singular values
		     );

/*!
  This function solves the system A x = b using the singular value decomposition 
  (U, S, V) of A given by gsl_linalg_SV_decomp.
  Only non-zero singular values are used in computing the solution. The parts of the 
  solution corresponding to singular values of zero are ignored. Other singular values 
  can be edited out by setting them to zero before calling this function.

  In the over-determined case where A has more rows than columns the system is solved 
  in the least squares sense, returning the solution x which minimizes ||A x - b||_2.
*/

int SV_solve(blitz::Array<double, 2>& U, //!< U as stored in A on return of SV_decomp
	     blitz::Array<double, 2>& V, //!< Rotation matrix
	     blitz::Array<double, 1>& S, //!< Singular values
	     blitz::Array<double, 1>& B, //!< right-hand side vector 
	     blitz::Array<double, 1>& X  //!< solution vector 
	     );

/*!
  This function factorizes the positive-definite square matrix A into the 
  Cholesky decomposition A = L L^T. On output the diagonal and lower triangular
  part of the input matrix A contain the matrix L. The upper triangular part of
  the input matrix contains L^T, the diagonal terms being identical for both L 
  and L^T. If the matrix is not positive-definite then the decomposition will 
  fail, returning the error code GSL_EDOM.
*/

int Cholesky_decomp(blitz::Array<double, 2>& A);

/*!
  This function solves the system A x = b using the Cholesky decomposition of A.
  A is overwritten as its Cholesky decomposition.
*/

int Cholesky_solve(blitz::Array<double, 2>& A,
		   blitz::Array<double, 1>& B,
		   blitz::Array<double, 1>& X
		   );

/*!
  This function solves the system A x = b using the Cholesky decomposition of A.
  A is overwritten as its Cholesky decomposition, and B is replaced by X;
*/

int Cholesky_svx(blitz::Array<double, 2>& A,
		 blitz::Array<double, 1>& B
		 );

/*!
  This function solves the system A x = b directly using Householder 
  transformations. On output the solution is stored in x and b is not modified.
  The matrix A is destroyed by the Householder transformations.
*/

int HH_solve(blitz::Array<double, 2>& A,
	     blitz::Array<double, 1>& B,
	     blitz::Array<double, 1>& X
	     );

/*!
  This function solves the system A x = b in-place using Householder 
  transformations. On input x should contain the right-hand side b, which is 
  replaced by the solution on output. The matrix A is destroyed by the 
  Householder transformations.
*/

int HH_svx(blitz::Array<double, 2>& A,
	   blitz::Array<double, 1>& X
	   );

/*!  
  These functions compute the best-fit parameters c of the model y
  = X c for the observations y and the matrix of predictor variables
  X. The variance-covariance matrix of the model parameters cov is
  estimated from the scatter of the observations about the best-fit. The
  sum of squares of the residuals from the best-fit, \chi^2, is returned
  in chisq.  The best-fit is found by singular value decomposition of
  the matrix X using the preallocated workspace provided in work. The
  modified Golub-Reinsch SVD algorithm is used, with column scaling to
  improve the accuracy of the singular values. Any components which have
  zero singular value (to machine precision) are discarded from the
  fit. In the second form of the function the components are discarded
  if the ratio of singular values s_i/s_0 falls below the user-specified
  tolerance tol, and the effective rank is returned in rank.
*/

int
multifit_linear(blitz::Array<double, 2>& X,
		blitz::Array<double, 1>& Y,
		blitz::Array<double, 1>& C,
		blitz::Array<double, 2>& Cov,
		double& chisq);

int
multifit_linear_svd(blitz::Array<double, 2>& X,
		    blitz::Array<double, 1>& Y,
		    double tol,
		    size_t& Rank,
		    blitz::Array<double, 1>& C,
		    blitz::Array<double, 2>& Cov,
		    double& chisq);
		    
/*!
  This function computes the best-fit parameters c of the weighted model y = X c 
  for the observations y with weights w and the matrix of predictor variables X. 
  The covariance matrix of the model parameters cov is estimated from the weighted
  data. The weighted sum of squares of the residuals from the best-fit, \chi^2, 
  is returned in chisq.

  The best-fit is found by singular value decomposition of the matrix X
  using the preallocated workspace provided in work. Any components
  which have zero singular value (to machine precision) are discarded
  from the fit. In the second form of the function the components are
  discarded if the ratio of singular values s_i/s_0 falls below the
  user-specified tolerance tol, and the effective rank is returned in
  rank.
*/


int
multifit_wlinear(blitz::Array<double, 2>& X,
		 blitz::Array<double, 1>& W,
		 blitz::Array<double, 1>& Y,
		 blitz::Array<double, 1>& C,
		 blitz::Array<double, 2>& Cov,
		 double& chisq);

int
multifit_wlinear_svd(blitz::Array<double, 2>& X,
		     blitz::Array<double, 1>& W,
		     blitz::Array<double, 1>& Y,
		     double tol,
		     size_t& Rank,
		     blitz::Array<double, 1>& C,
		     blitz::Array<double, 2>& Cov,
		     double& chisq);

#endif
