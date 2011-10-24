#include <iostream>
#include <cstdlib>
#include <blitz/array.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_errno.h>
#include "Blitz2GSL.h"

//#include "Blitz2GSL.C"

using namespace std;

//! Test the memory mapping between GSL and Blitz

void test_mapping() {
  blitz::Array<double, 1> Y(5);
  Y = 1.08, 1.07, 0.97, 0.77, 0.84;
  cout << "A : " << Y << endl;
  cout << "A data: " << Y.data() << endl;

//   gsl_vector_view m = gsl_vector_view_array (Y.data(), Y.extent(blitz::firstDim));
  gsl_vector_view m = blitz2gsl_vector(Y);

  cout << &m.vector.data << endl;
  for( size_t i = 0; i < m.vector.size ; ++i ){
    cout << gsl_vector_get(&m.vector, i) << endl;
  }

  gsl_vector_set(&m.vector,0,8.);

  for( size_t i = 0; i < m.vector.size ; ++i ){
    cout << gsl_vector_get(&m.vector, i) << endl;
  }
  cout << "A : " << Y << endl;

  Y(2) = 7.;
  cout << "A : " << Y << endl;
  for( size_t i = 0; i < m.vector.size ; ++i ){
    cout << gsl_vector_get(&m.vector, i) << endl;
  }

  // Try a matrix view

  blitz::Array<double, 2> M(3,3);
  M = 
    1., 2., 3., 
    4., 5., 6., 
    7., 8., 9.;
  cout<< M;

//   gsl_matrix_view mv = gsl_matrix_view_array(M.data(), 3, 3);
  gsl_matrix_view mv = blitz2gsl_matrix(M);
  for( size_t i = 0; i < mv.matrix.size1 ; i++ ){
    for( size_t j = 0; j < mv.matrix.size2 ; j++ ){
      cout << gsl_matrix_get(&mv.matrix, i, j) << "\t";
    }
    cout<<endl;
  }

  gsl_matrix_set(&mv.matrix,0,0,8.);
  cout<< M;
  M(0,0) = 0;

  for( size_t i = 0; i < mv.matrix.size1 ; i++ ){
    for( size_t j = 0; j < mv.matrix.size2 ; j++ ){
      cout << gsl_matrix_get(&mv.matrix, i, j) << "\t";
    }
    cout<<endl;
  }
}


//! Test the SVD routines

void test_svd() {
  blitz::Array<double, 2> A(4,4);
  A = 
    0.18, 0.60, 0.57, 0.96,
    0.41, 0.24, 0.99, 0.58,
    0.14, 0.30, 0.97, 0.66,
    0.51, 0.13, 0.19, 0.85;
  
  blitz::Array<double, 2> V(4,4);
  blitz::Array<double, 1> S(4);
  
  int status = SV_decomp(A,V,S);

  cout<<"SV_decom test"<<endl;
  cout<<"exit status: "<<gsl_strerror(status)<<endl;

  cout<<A;
  cout<<V;
  cout<<S;
  cout<<endl;

  A = 
    0.18, 0.60, 0.57, 0.96,
    0.41, 0.24, 0.99, 0.58,
    0.14, 0.30, 0.97, 0.66,
    0.51, 0.13, 0.19, 0.85;
  V = 0.;
  S = 0.;

  status = SV_decomp_mod(A,V,S);

  cout<<"SV_decom_mod test"<<endl;
  cout<<"exit status: "<<gsl_strerror(status)<<endl;

  cout<<A;
  cout<<V;
  cout<<S;
  cout<<endl;

  A = 
    0.18, 0.60, 0.57, 0.96,
    0.41, 0.24, 0.99, 0.58,
    0.14, 0.30, 0.97, 0.66,
    0.51, 0.13, 0.19, 0.85;
  V = 0.;
  S = 0.;

  status = SV_decomp_jacobi(A,V,S);

  cout<<"SV_decom_jacobi test"<<endl;
  cout<<"exit status: "<<gsl_strerror(status)<<endl;

  cout<<A;
  cout<<V;
  cout<<S;
  cout<<endl;

  // Test the solver

  blitz::Array<double, 1> X(4);
  blitz::Array<double, 1> B(4);
  B = 1.0, 2.0, 3.0, 4.0;

  SV_solve(A,V,S,B,X);

  cout<<"SV_solve test"<<endl;
  cout<<X;
  cout<<"\nShould be: [ -4.05205, -12.6056, 1.66091, 8.69377]"<<endl;
}

void test_lud() {
  blitz::Array<double, 2> A(4,4);
  A = 
    0.18, 0.60, 0.57, 0.96,
    0.41, 0.24, 0.99, 0.58,
    0.14, 0.30, 0.97, 0.66,
    0.51, 0.13, 0.19, 0.85;

  blitz::Array<double, 1> X(4);
  blitz::Array<double, 1> B(4);
  B = 1.0, 2.0, 3.0, 4.0;
  
  int status = LU_solve(A,B,X);

  cout<<"LU_solve test"<<endl;
  cout<<"exit status: "<<gsl_strerror(status)<<endl;

  cout<<B;
  cout<<X;
  cout<<"\nShould be: [ -4.05205, -12.6056, 1.66091, 8.69377]"<<endl;

  A = 
    0.18, 0.60, 0.57, 0.96,
    0.41, 0.24, 0.99, 0.58,
    0.14, 0.30, 0.97, 0.66,
    0.51, 0.13, 0.19, 0.85;

  B = 1.0, 2.0, 3.0, 4.0;

  status = LU_svx(A,B);

  cout<<"LU_svx test"<<endl;
  cout<<"exit status: "<<gsl_strerror(status)<<endl;

  cout<<B;
  cout<<"\nShould be: [ -4.05205, -12.6056, 1.66091, 8.69377]"<<endl;

  // Compute the inverse

  A = 
    0.18, 0.60, 0.57, 0.96,
    0.41, 0.24, 0.99, 0.58,
    0.14, 0.30, 0.97, 0.66,
    0.51, 0.13, 0.19, 0.85;

  B = 1.0, 2.0, 3.0, 4.0;
  blitz::Array<double, 2> Inverse(4,4);

  LU_inverse(A,Inverse);

  blitz::firstIndex i;
  blitz::secondIndex j;
  blitz::thirdIndex k;

  A = 
    0.18, 0.60, 0.57, 0.96,
    0.41, 0.24, 0.99, 0.58,
    0.14, 0.30, 0.97, 0.66,
    0.51, 0.13, 0.19, 0.85;

  blitz::Array<double, 2> I(4,4);
  I = blitz::sum(Inverse(i,k)*A(k,j),k);
  cout<<I<<endl;

  X = blitz::sum(Inverse(i,j)*B(j),j);
  cout<<X<<endl;
  cout<<"Should be: [ -4.05205, -12.6056, 1.66091, 8.69377]"<<endl;
}

void test_cholesky() {
  blitz::Array<double, 2> A(4,4);
  A = 
    2.0, 0., 0., 0.,
    0.0, 2., 0., 0.,
    0.0, 0., 2., 0.,
    0.0, 0., 0., 2.;

  blitz::Array<double, 1> X(4);
  blitz::Array<double, 1> B(4);
  B = 1.0, 2.0, 3.0, 4.0;
  
  int status = Cholesky_solve(A,B,X);

  cout<<"Cholesky_solve test"<<endl;
  cout<<"exit status: "<<gsl_strerror(status)<<endl;

  cout<<B;
  cout<<X<<endl;

  A = 
    2.0, 0., 0., 0.,
    0.0, 2., 0., 0.,
    0.0, 0., 2., 0.,
    0.0, 0., 0., 2.;

  status = Cholesky_svx(A,B);

  cout<<"Cholesky_svx test"<<endl;
  cout<<"exit status: "<<gsl_strerror(status)<<endl;

  cout<<B<<endl;
}

void test_householder() {
  blitz::Array<double, 2> A(4,4);
  A = 
    0.18, 0.60, 0.57, 0.96,
    0.41, 0.24, 0.99, 0.58,
    0.14, 0.30, 0.97, 0.66,
    0.51, 0.13, 0.19, 0.85;

  blitz::Array<double, 1> X(4);
  blitz::Array<double, 1> B(4);
  B = 1.0, 2.0, 3.0, 4.0;
  
  int status = HH_solve(A,B,X);

  cout<<"HH_solve test"<<endl;
  cout<<"exit status: "<<gsl_strerror(status)<<endl;

  cout<<B;
  cout<<X;
  cout<<"\nShould be: [ -4.05205, -12.6056, 1.66091, 8.69377]"<<endl;

  A = 
    0.18, 0.60, 0.57, 0.96,
    0.41, 0.24, 0.99, 0.58,
    0.14, 0.30, 0.97, 0.66,
    0.51, 0.13, 0.19, 0.85;

  B = 1.0, 2.0, 3.0, 4.0;

  status = HH_svx(A,B);

  cout<<"HH_svx test"<<endl;
  cout<<"exit status: "<<gsl_strerror(status)<<endl;

  cout<<B;
  cout<<"\nShould be: [ -4.05205, -12.6056, 1.66091, 8.69377]"<<endl;
}

void test_fit() {
  int i;
  int n = 40;
  double chisq;


  blitz::Array<double, 2> X(n, 3);
  blitz::Array<double, 2> cov(3, 3);
  blitz::Array<double, 1> y(n);
  blitz::Array<double, 1> w(n);
  blitz::Array<double, 1> c(3);

  blitz::Array<double, 1> xi(n);
  blitz::Array<double, 1> ei(n);
  
  y = .11019, 
    .21956,
    .32949,
    .43899,
    .54803,
    .65694,
    .76562,
    .87487,
    .98292,
    1.09146,
    1.20001,
    1.30822,
    1.41599,
    1.52399,
    1.63194,
    1.73947,
    1.84646,
    1.95392,
    2.06128,
    2.16844,
    .11052,
    .22018,
    .32939,
    .43886,
    .54798,
    .65739,
    .76596,
    .87474,
    .98300,
    1.09150,
    1.20004,
    1.30818,
    1.41613,
    1.52408,
    1.63159,
    1.73965,
    1.84696,
    1.95445,
    2.06177,
    2.16829;

  xi = 150000,
    300000,
    450000,
    600000,
    750000,
    900000,
    1050000,
    1200000,
    1350000,
    1500000,
    1650000,
    1800000,
    1950000,
    2100000,
    2250000,
    2400000,
    2550000,
    2700000,
    2850000,
    3000000,
    150000,
    300000,
    450000,
    600000,
    750000,
    900000,
    1050000,
    1200000,
    1350000,
    1500000,
    1650000,
    1800000,
    1950000,
    2100000,
    2250000,
    2400000,
    2550000,
    2700000,
    2850000,
    3000000;

  ei = 1.;

  for (i = 0; i < n; i++) {

    X(i,0) = 1.;
    X(i,1) = xi(i);
    X(i,2) = xi(i)*xi(i);
      
    w(i) = 1./(ei(i)*ei(i)); 
  }

  cout<<"weighted linear"<<endl;

  multifit_wlinear(X,w,y,c,cov,chisq);

  printf("Should be:Y = %g + %g X + %g X^2\n", 
	  0.673565789473684E-03, 
	 0.732059160401003E-06,
	 -0.316081871345029E-14);

  printf ("# best fit: Y = %g + %g X + %g X^2\n", 
	  c(0), c(1), c(2));

  printf ("# covariance matrix:\n");
  printf ("[ %+.5e, %+.5e, %+.5e  \n",
               cov(0,0), cov(0,1), cov(0,2));
  printf ("  %+.5e, %+.5e, %+.5e  \n", 
	  cov(1,0), cov(1,1), cov(1,2));
  printf ("  %+.5e, %+.5e, %+.5e ]\n", 
	  cov(2,0), cov(2,1), cov(2,2));
  printf ("# chisq = %g\n", chisq);
  printf ("# sqrt(chisq) = %g\n", sqrt(chisq));

  cout<<"weighted linear svd"<<endl;

  double tol = 1.e-16;
  size_t rank;
  multifit_wlinear_svd(X,w,y,tol,rank,c,cov,chisq);

  printf("Should be:Y = %g + %g X + %g X^2\n", 
	  0.673565789473684E-03, 
	 0.732059160401003E-06,
	 -0.316081871345029E-14);

  printf ("# best fit: Y = %g + %g X + %g X^2\n", 
	  c(0), c(1), c(2));

  printf ("# covariance matrix:\n");
  printf ("[ %+.5e, %+.5e, %+.5e  \n",
               cov(0,0), cov(0,1), cov(0,2));
  printf ("  %+.5e, %+.5e, %+.5e  \n", 
	  cov(1,0), cov(1,1), cov(1,2));
  printf ("  %+.5e, %+.5e, %+.5e ]\n", 
	  cov(2,0), cov(2,1), cov(2,2));
  printf ("# chisq = %g\n", chisq);
  printf ("# sqrt(chisq) = %g\n", sqrt(chisq));

  cout<<"unweighted linear"<<endl;

  multifit_linear(X,y,c,cov,chisq);

  printf("Should be:Y = %g + %g X + %g X^2\n", 
	  0.673565789473684E-03, 
	 0.732059160401003E-06,
	 -0.316081871345029E-14);

  printf ("# best fit: Y = %g + %g X + %g X^2\n", 
	  c(0), c(1), c(2));

  printf ("# covariance matrix:\n");
  printf ("[ %+.5e, %+.5e, %+.5e  \n",
               cov(0,0), cov(0,1), cov(0,2));
  printf ("  %+.5e, %+.5e, %+.5e  \n", 
	  cov(1,0), cov(1,1), cov(1,2));
  printf ("  %+.5e, %+.5e, %+.5e ]\n", 
	  cov(2,0), cov(2,1), cov(2,2));
  printf ("# chisq = %g\n", chisq);
  printf ("# sqrt(chisq) = %g\n", sqrt(chisq));

  cout<<"unweighted linear svd"<<endl;

  multifit_linear_svd(X,y,tol,rank,c,cov,chisq);

  printf("Should be:Y = %g + %g X + %g X^2\n", 
	  0.673565789473684E-03, 
	 0.732059160401003E-06,
	 -0.316081871345029E-14);

  printf ("# best fit: Y = %g + %g X + %g X^2\n", 
	  c(0), c(1), c(2));

  printf ("# covariance matrix:\n");
  printf ("[ %+.5e, %+.5e, %+.5e  \n",
               cov(0,0), cov(0,1), cov(0,2));
  printf ("  %+.5e, %+.5e, %+.5e  \n", 
	  cov(1,0), cov(1,1), cov(1,2));
  printf ("  %+.5e, %+.5e, %+.5e ]\n", 
	  cov(2,0), cov(2,1), cov(2,2));
  printf ("# chisq = %g\n", chisq);
  printf ("# sqrt(chisq) = %g\n", sqrt(chisq));

}

int main() {

  // Test the translation between Blitz and GSL

  cout<<"///////////////////////////////////////////////////////////////"<<endl;
  cout<<"                         Test MemoryMapping                    "<<endl;
  cout<<"///////////////////////////////////////////////////////////////"<<endl;

  test_mapping();
  
  // test SVD decomposition routines

  cout<<"///////////////////////////////////////////////////////////////"<<endl;
  cout<<"                         Test SVD                              "<<endl;
  cout<<"///////////////////////////////////////////////////////////////"<<endl;

  test_svd();

  // Test LU decomposition

  cout<<"///////////////////////////////////////////////////////////////"<<endl;
  cout<<"                         Test LUD                              "<<endl;
  cout<<"///////////////////////////////////////////////////////////////"<<endl;

  test_lud();

  // Test Cholesky decomposition

  cout<<"///////////////////////////////////////////////////////////////"<<endl;
  cout<<"                         Test Cholesky                         "<<endl;
  cout<<"///////////////////////////////////////////////////////////////"<<endl;

  test_cholesky();

  // Householder decomposition

  cout<<"///////////////////////////////////////////////////////////////"<<endl;
  cout<<"                         Test Householder                      "<<endl;
  cout<<"///////////////////////////////////////////////////////////////"<<endl;

  test_householder();

  // Fitting

  cout<<"///////////////////////////////////////////////////////////////"<<endl;
  cout<<"                         Test Fit                              "<<endl;
  cout<<"///////////////////////////////////////////////////////////////"<<endl;

  test_fit();  

  return EXIT_SUCCESS;
}
