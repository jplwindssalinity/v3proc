#include "Blitz2GSL.h"

using namespace blitz;

int LU_solve(blitz::Array<double, 2>& A, //!< Matrix to invert
	     blitz::Array<double, 1>& B, //!< RHS
 	     blitz::Array<double, 1>& X  //!< Solution vector
	     ) {

  // Check that the sizes are appropriate

  int m = A.extent(blitz::firstDim);
  int n = A.extent(blitz::secondDim);
  if(m != n) return GSL_EINVAL;
  if(B.extent(blitz::firstDim) < n) return GSL_EINVAL;
  if(X.extent(blitz::firstDim) < n) return GSL_EINVAL;

  // Allocate the work vector and views

  gsl_matrix_view a = blitz2gsl_matrix(A);
  gsl_vector_view b = blitz2gsl_vector(B);
  gsl_vector_view x = blitz2gsl_vector(X);

  gsl_permutation *p = gsl_permutation_alloc(n);

  // Do the LUD

  int signum;
  int status = gsl_linalg_LU_decomp(&a.matrix, p,  &signum);
  if(status != 0) return status;

  // Solve the equations

  status = gsl_linalg_LU_solve(&a.matrix, p, &b.vector, &x.vector);
  
  gsl_permutation_free(p);

  return status;
}

int LU_svx(blitz::Array<double, 2>& A, //!< Matrix to invert
	   blitz::Array<double, 1>& X //!< On input B, on output X
	   ) {

  // Check that the sizes are appropriate

  int m = A.extent(blitz::firstDim);
  int n = A.extent(blitz::secondDim);
  if(m != n) return GSL_EINVAL;
  if(X.extent(blitz::firstDim) < n) return GSL_EINVAL;

  // Allocate the work vector and views

  gsl_matrix_view a = blitz2gsl_matrix(A);
  gsl_vector_view x = blitz2gsl_vector(X);

  gsl_permutation *p = gsl_permutation_alloc(n);

  // Do the LUD

  int signum;
  int status = gsl_linalg_LU_decomp(&a.matrix, p,  &signum);
  if(status != 0) return status;

  // Solve the equations

  status = gsl_linalg_LU_svx(&a.matrix, p,  &x.vector);
  
  gsl_permutation_free(p);

  return status;
}

int LU_inverse(blitz::Array<double, 2>& A, //!< Matrix to invert
	       blitz::Array<double, 2>& Inverse //!< Inverse matrix
	       ) {

  // Check that the sizes are appropriate

  int m = A.extent(blitz::firstDim);
  int n = A.extent(blitz::secondDim);
  if(m != n) return GSL_EINVAL;
  if(Inverse.extent(blitz::firstDim) < n) return GSL_EINVAL;
  if(Inverse.extent(blitz::secondDim) < n) return GSL_EINVAL;

  // Allocate the work vector and views

  gsl_matrix_view a = blitz2gsl_matrix(A);
  gsl_matrix_view inverse = blitz2gsl_matrix(Inverse);

  gsl_permutation *p = gsl_permutation_alloc(n);

  // Do the LUD

  int signum;
  int status = gsl_linalg_LU_decomp(&a.matrix, p,  &signum);
  if(status != 0) return status;

  // Get the inverse

  status = gsl_linalg_LU_invert(&a.matrix, p,  &inverse.matrix);
  
  gsl_permutation_free(p);

  return status;
}

double LU_det(blitz::Array<double, 2>& A) {

  // Check that the sizes are appropriate

  int m = A.extent(blitz::firstDim);
  int n = A.extent(blitz::secondDim);
  if(m != n) return GSL_EINVAL;

  // Allocate the work vector and views

  gsl_matrix_view a = blitz2gsl_matrix(A);
  gsl_permutation *p = gsl_permutation_alloc(n);

  // Do the LUD

  int signum;
  int status = gsl_linalg_LU_decomp(&a.matrix, p,  &signum);
  if(status != 0) return status;

  // Get the determinant

  double det = gsl_linalg_LU_det(&a.matrix, signum);
  
  gsl_permutation_free(p);

  return det;
}

int SV_decomp(blitz::Array<double, 2>& A, //!< Matrix to decompose 
	      blitz::Array<double, 2>& V, //!< Rotation matrix
	      blitz::Array<double, 1>& S  //!< Singular values
	      ) {

  // Check that the sizes are appropriate

//   int m = A.extent(blitz::firstDim);
  int n = A.extent(blitz::secondDim);
  if((V.extent(blitz::firstDim) != n) || (V.extent(blitz::secondDim) != n) ) {
    return GSL_EINVAL;
  }
  if(S.extent(blitz::firstDim) < n) return GSL_EINVAL;

  // Allocate the work vector and views

  gsl_matrix_view a = blitz2gsl_matrix(A);
  gsl_matrix_view v = blitz2gsl_matrix(V);
  gsl_vector_view s = blitz2gsl_vector(S);
  gsl_vector *work = gsl_vector_alloc(n);

  // Do the SVD inversion

  int status = gsl_linalg_SV_decomp(&a.matrix, &v.matrix, &s.vector, work);
  
  gsl_vector_free(work);

  return status;
}

int SV_decomp_mod(blitz::Array<double, 2>& A, //!< Matrix to decompose 
		  blitz::Array<double, 2>& V, //!< Rotation matrix
		  blitz::Array<double, 1>& S  //!< Singular values
		  ) {

  // Check that the sizes are appropriate

//   int m = A.extent(blitz::firstDim);
  int n = A.extent(blitz::secondDim);
  if((V.extent(blitz::firstDim) != n) || (V.extent(blitz::firstDim) != n) ) {
    return GSL_EINVAL;
  }
  if(S.extent(blitz::firstDim) < n) return GSL_EINVAL;

  // Allocate the work vector and views

  gsl_matrix_view a = blitz2gsl_matrix(A);
  gsl_matrix_view v = blitz2gsl_matrix(V);
  gsl_vector_view s = blitz2gsl_vector(S);
  gsl_matrix *x = gsl_matrix_alloc(n,n);
  gsl_vector *work = gsl_vector_alloc(n);

  // Do the SVD inversion

  int status = gsl_linalg_SV_decomp_mod(&a.matrix, x, &v.matrix, &s.vector, work);
  
  gsl_matrix_free(x);
  gsl_vector_free(work);

  return status;
}

int SV_decomp_jacobi(blitz::Array<double, 2>& A, //!< Matrix to decompose 
		     blitz::Array<double, 2>& V, //!< Rotation matrix
		     blitz::Array<double, 1>& S  //!< Singular values
		     ) {

  // Check that the sizes are appropriate

//   int m = A.extent(blitz::firstDim);
  int n = A.extent(blitz::secondDim);
  if((V.extent(blitz::firstDim) != n) || (V.extent(blitz::firstDim) != n) ) {
    return GSL_EINVAL;
  }
  if(S.extent(blitz::firstDim) < n) return GSL_EINVAL;

  // Allocate the views

  gsl_matrix_view a = blitz2gsl_matrix(A);
  gsl_matrix_view v = blitz2gsl_matrix(V);
  gsl_vector_view s = blitz2gsl_vector(S);

  // Do the SVD inversion

  int status = gsl_linalg_SV_decomp_jacobi(&a.matrix, &v.matrix, &s.vector);
  
  return status;
}


int SV_solve(blitz::Array<double, 2>& U, //!< U as stored in A on return of SV_decomp
	     blitz::Array<double, 2>& V, //!< Rotation matrix
	     blitz::Array<double, 1>& S, //!< Singular values
	     blitz::Array<double, 1>& B, //!< right-hand side vector 
	     blitz::Array<double, 1>& X  //!< solution vector 
	     ) {

  // Check that the sizes are appropriate

//   int m = U.extent(blitz::firstDim);
  int n = U.extent(blitz::secondDim);
  if((V.extent(blitz::firstDim) != n) || (V.extent(blitz::firstDim) != n) ) {
    return GSL_EINVAL;
  }
  if(S.extent(blitz::firstDim) < n) return GSL_EINVAL;
  if(B.extent(blitz::firstDim) < n) return GSL_EINVAL;
  if(X.extent(blitz::firstDim) < n) return GSL_EINVAL;

  // Allocate the views

  gsl_matrix_view u = blitz2gsl_matrix(U);
  gsl_matrix_view v = blitz2gsl_matrix(V);
  gsl_vector_view s = blitz2gsl_vector(S);
  gsl_vector_view b = blitz2gsl_vector(B);
  gsl_vector_view x = blitz2gsl_vector(X);

  // Do the SVD solution

  int status = gsl_linalg_SV_solve(&u.matrix, &v.matrix, 
				   &s.vector, &b.vector, &x.vector);
  
  return status;
}
  
int Cholesky_decomp(blitz::Array<double, 2>& A) {
  // Check that the sizes are appropriate

  int m = A.extent(blitz::firstDim);
  int n = A.extent(blitz::secondDim);
  if(m != n) return GSL_EINVAL;

  // Allocate the views

  gsl_matrix_view a = blitz2gsl_matrix(A);

  // Do the Cholesky decomposition

  int status = gsl_linalg_cholesky_decomp(&a.matrix);
  
  return status;
}

int Cholesky_solve(blitz::Array<double, 2>& A,
		   blitz::Array<double, 1>& B,
		   blitz::Array<double, 1>& X
		   ) {
  // Check that the sizes are appropriate

  int m = A.extent(blitz::firstDim);
  int n = A.extent(blitz::secondDim);
  if(m != n) return GSL_EINVAL;
  if(B.extent(blitz::firstDim) < n) return GSL_EINVAL;
  if(X.extent(blitz::firstDim) < n) return GSL_EINVAL;

  // Allocate the views

  gsl_matrix_view a = blitz2gsl_matrix(A);
  gsl_vector_view b = blitz2gsl_vector(B);
  gsl_vector_view x = blitz2gsl_vector(X);

  // Do the Cholesky decomposition

  int status = gsl_linalg_cholesky_decomp(&a.matrix);
  if(status != 0) return status;

  status = gsl_linalg_cholesky_solve(&a.matrix, &b.vector, &x.vector);

  return status;
}

int Cholesky_svx(blitz::Array<double, 2>& A,
		 blitz::Array<double, 1>& B
		   ) {
  // Check that the sizes are appropriate

  int m = A.extent(blitz::firstDim);
  int n = A.extent(blitz::secondDim);
  if(m != n) return GSL_EINVAL;
  if(B.extent(blitz::firstDim) < n) return GSL_EINVAL;

  // Allocate the views

  gsl_matrix_view a = blitz2gsl_matrix(A);
  gsl_vector_view b = blitz2gsl_vector(B);

  // Do the Cholesky decomposition

  int status = gsl_linalg_cholesky_decomp(&a.matrix);
  if(status != 0) return status;

  status = gsl_linalg_cholesky_svx(&a.matrix, &b.vector);

  return status;
}

int HH_solve(blitz::Array<double, 2>& A,
	     blitz::Array<double, 1>& B,
	     blitz::Array<double, 1>& X
	     ) {

  // Check that the sizes are appropriate

  int m = A.extent(blitz::firstDim);
  int n = A.extent(blitz::secondDim);
  if(B.extent(blitz::firstDim) < n) return GSL_EINVAL;
  if(X.extent(blitz::firstDim) < m) return GSL_EINVAL;

  // Allocate the views

  gsl_matrix_view a = blitz2gsl_matrix(A);
  gsl_vector_view b = blitz2gsl_vector(B);
  gsl_vector_view x = blitz2gsl_vector(X);

  // Do the Householder decomposition

  int status = gsl_linalg_HH_solve(&a.matrix, &b.vector, &x.vector);

  return status;
}

int HH_svx(blitz::Array<double, 2>& A,
	   blitz::Array<double, 1>& X
	   ) {

  // Check that the sizes are appropriate

  int m = A.extent(blitz::firstDim);
//   int n = A.extent(blitz::secondDim);
  if(X.extent(blitz::firstDim) < m) return GSL_EINVAL;

  // Allocate the views

  gsl_matrix_view a = blitz2gsl_matrix(A);
  gsl_vector_view x = blitz2gsl_vector(X);

  // Do the Householder decomposition

  int status = gsl_linalg_HH_svx(&a.matrix, &x.vector);

  return status;
}

int
multifit_linear(blitz::Array<double, 2>& X,
		blitz::Array<double, 1>& Y,
		blitz::Array<double, 1>& C,
		blitz::Array<double, 2>& Cov,
		double& chisq) {

  // Check that the sizes are appropriate

  int m = X.extent(blitz::firstDim);
  int n = X.extent(blitz::secondDim);
  if(Y.extent(blitz::firstDim) < n) return GSL_EINVAL;
  if(C.extent(blitz::firstDim) < n) return GSL_EINVAL;

  // Allocate the views

  gsl_matrix_view x = blitz2gsl_matrix(X);
  gsl_matrix_view cov = blitz2gsl_matrix(Cov);
  gsl_vector_view y = blitz2gsl_vector(Y);
  gsl_vector_view c = blitz2gsl_vector(C);

  gsl_multifit_linear_workspace *work = gsl_multifit_linear_alloc(m,n);

  // Do the Householder decomposition

  int status = gsl_multifit_linear(&x.matrix, &y.vector, 
				   &c.vector, &cov.matrix,
				   &chisq, work);

  gsl_multifit_linear_free(work);

  return status;

}

int
multifit_linear_svd(blitz::Array<double, 2>& X,
		    blitz::Array<double, 1>& Y,
		    double tol,
		    size_t& Rank,
		    blitz::Array<double, 1>& C,
		    blitz::Array<double, 2>& Cov,
		    double& chisq) {
  // Check that the sizes are appropriate

  int m = X.extent(blitz::firstDim);
  int n = X.extent(blitz::secondDim);
  if(Y.extent(blitz::firstDim) < n) return GSL_EINVAL;
  if(C.extent(blitz::firstDim) < n) return GSL_EINVAL;

  // Allocate the views

  gsl_matrix_view x = blitz2gsl_matrix(X);
  gsl_matrix_view cov = blitz2gsl_matrix(Cov);
  gsl_vector_view y = blitz2gsl_vector(Y);
  gsl_vector_view c = blitz2gsl_vector(C);

  gsl_multifit_linear_workspace *work = gsl_multifit_linear_alloc(m,n);

  // Do the Householder decomposition

  int status = gsl_multifit_linear_svd(&x.matrix, &y.vector, 
				       tol, &Rank,
				       &c.vector, &cov.matrix,
				       &chisq, work);

  gsl_multifit_linear_free(work);

  return status;

}


int
multifit_wlinear(blitz::Array<double, 2>& X,
		 blitz::Array<double, 1>& W,
		 blitz::Array<double, 1>& Y,
		 blitz::Array<double, 1>& C,
		 blitz::Array<double, 2>& Cov,
		 double& chisq) {
  // Check that the sizes are appropriate

  int m = X.extent(blitz::firstDim);
  int n = X.extent(blitz::secondDim);
  if(W.extent(blitz::firstDim) < m) return GSL_EINVAL;
  if(Y.extent(blitz::firstDim) < m) return GSL_EINVAL;
  if(C.extent(blitz::firstDim) < n) return GSL_EINVAL;

  // Allocate the views

  gsl_matrix_view x = blitz2gsl_matrix(X);
  gsl_matrix_view cov = blitz2gsl_matrix(Cov);
  gsl_vector_view w = blitz2gsl_vector(W);
  gsl_vector_view y = blitz2gsl_vector(Y);
  gsl_vector_view c = blitz2gsl_vector(C);

  gsl_multifit_linear_workspace *work = gsl_multifit_linear_alloc(m,n);

  // Do the Householder decomposition

  int status = gsl_multifit_wlinear(&x.matrix, &w.vector,
				    &y.vector, 
				    &c.vector, &cov.matrix,
				    &chisq, work);

  gsl_multifit_linear_free(work);

  return status;

}

int
multifit_wlinear_svd(blitz::Array<double, 2>& X,
		     blitz::Array<double, 1>& W,
		     blitz::Array<double, 1>& Y,
		     double tol,
		     size_t& Rank,
		     blitz::Array<double, 1>& C,
		     blitz::Array<double, 2>& Cov,
		     double& chisq) {
  // Check that the sizes are appropriate

  int m = X.extent(blitz::firstDim);
  int n = X.extent(blitz::secondDim);
  if(W.extent(blitz::firstDim) < m) return GSL_EINVAL;
  if(Y.extent(blitz::firstDim) < n) return GSL_EINVAL;
  if(C.extent(blitz::firstDim) < n) return GSL_EINVAL;

  // Allocate the views

  gsl_matrix_view x = blitz2gsl_matrix(X);
  gsl_matrix_view cov = blitz2gsl_matrix(Cov);
  gsl_vector_view w = blitz2gsl_vector(W);
  gsl_vector_view y = blitz2gsl_vector(Y);
  gsl_vector_view c = blitz2gsl_vector(C);

  gsl_multifit_linear_workspace *work = gsl_multifit_linear_alloc(m,n);

  // Do the Householder decomposition

  int status = gsl_multifit_wlinear_svd(&x.matrix, &w.vector, &y.vector, 
				       tol, &Rank,
				       &c.vector, &cov.matrix,
				       &chisq, work);

  gsl_multifit_linear_free(work);

  return status;

}

