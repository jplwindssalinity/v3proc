/*!
  \file NC2Blitz.h
  \author E. Rodriguez
  \brief A class to read and write netcdf files using blitz++ arrays.
*/

#ifndef _ER_NC2BLITZ_H_
#define _ER_NC2BLITZ_H_

#include <netcdfcpp.h>
#include <blitz/array.h>
#include "Exception.h"

/*!
  \brief A class to read and write netcdf files using blitz++ arrays.
*/

class NC2Blitz {
public:

  //! Initialize with a pointer to an open netcdf file object

  NC2Blitz(NcFile* _ncFile):ncFile(_ncFile) {}

  //! The template get function retrieves 1D data given the variable name. The data type and dimension is
  //! inferred from the array passed. The array is resized to fit the data request, so it needs to be declared
  //! but no memory needs to be allocated.

  template <class Type>
  void get(const char* var_name, blitz::Array<Type, 1>& var, size_t n = 0, int start_index = 0);

  //! The template get functions retrieves 2D data given the variable name. The data type and dimension is
  //! inferred from the array passed. The array is resized to fit the data request, so it needs to be declared
  //! but no memory needs to be allocated.

  template <class Type>
  void get(const char* var_name, blitz::Array<Type, 2>& var, size_t nrow = 0, size_t ncol = 0, int row_index = 0, int col_index = 0);


  //! The template put function writes 1D data given the variable name. The data type and dimension is
  //! inferred from the array passed. 

  template <class Type>
  void put(const char* var_name, blitz::Array<Type, 1>& var, size_t n = 0, int start_index = 0);

  //! The template get functions writes 2D data given the variable name. The data type and dimension is
  //! inferred from the array passed. 

  template <class Type>
  void put(const char* var_name, blitz::Array<Type, 2>& var, size_t nrow = 0, size_t ncol = 0, int row_index = 0, int col_index = 0);

  //! The template put function writes a record stored in a 1D array giventhe variable name. The data type and dimension is
  //! inferred from the array passed. The shape of the array does not have to be the same as the variable record shape, it just
  //! has to have a size greater than or equal to the record size.

  template <class Type, int N>
  void put_rec(const char* var_name, blitz::Array<Type, N>& var, int rec);


  // Keep a copy of the netcdf file pointer

  NcFile* ncFile;

};

template <class Type>
void 
NC2Blitz::get(const char* var_name, blitz::Array<Type, 1>& var, size_t n, int start_index) {
  
  NcVar* ncvar = ncFile->get_var(var_name);

  // set the starting index

  if(!ncvar->set_cur(start_index)) {
    throw Exception("cannot move start_index to desired location","NC2Blitz::get");
  }

  // If the number of desired points is 0, set to the maximum allowed

  if( n == 0 ) {
    n = ncvar->num_vals() - start_index;
  }

  // Resize the array
  
  var.resize(n);

  // Read the data

  if( !ncvar->get(var.data(), n) ) {
    throw Exception("cannot read data","NC2Blitz::get");
  }

}

template <class Type>
void 
NC2Blitz::get(const char* var_name, blitz::Array<Type, 2>& var, size_t nrow, size_t ncol, int row_index, int col_index) {
  
  NcVar* ncvar = ncFile->get_var(var_name);

  if(ncvar->num_dims() != 2) {
    throw Exception("data dimension not equal to 2 (array dimension)","NC2Blitz::get");
  }
  // set the starting index

  if(!ncvar->set_cur(row_index, col_index)) {
    throw Exception("cannot move start_index to desired location","NC2Blitz::get");
  }

  // If the number of desired points is 0, set to the maximum allowed

  long* edges = ncvar->edges();
  if( nrow == 0 ) {
    nrow = edges[0] - row_index;
  }
  if( ncol == 0 ) {
    ncol = edges[1] - col_index;
  }
  delete [] edges;

  // Resize the array
  
  var.resize(nrow,ncol);

  // Read the data

  if( !ncvar->get(var.data(), nrow, ncol) ) {
    throw Exception("cannot read data","NC2Blitz::get");
  }
}

template <class Type>
void 
NC2Blitz::put(const char* var_name, blitz::Array<Type, 1>& var, size_t n, int start_index) {
  
  NcVar* ncvar = ncFile->get_var(var_name);

  // set the starting index

  if(!ncvar->set_cur(start_index)) {
    throw Exception("cannot move start_index to desired location","NC2Blitz::put");
  }

  // If the number of desired points is 0, set to the maximum allowed

  if( n == 0 ) {
    n = ncvar->num_vals() - start_index;
  }

  // write the data

  if( !ncvar->put(var.data(), n) ) {
    throw Exception("cannot write data","NC2Blitz::put");
  }
  ncFile->sync();
}

template <class Type>
void 
NC2Blitz::put(const char* var_name, blitz::Array<Type, 2>& var, size_t nrow, size_t ncol, int row_index, int col_index) {
  
  NcVar* ncvar = ncFile->get_var(var_name);

  if(ncvar->num_dims() != 2) {
    throw Exception("data dimension not equal to 2 (array dimension)","NC2Blitz::put");
  }
  // set the starting index

  if(!ncvar->set_cur(row_index, col_index)) {
    throw Exception("cannot move start_index to desired location","NC2Blitz::put");
  }

  // If the number of desired points is 0, set to the maximum allowed

  long* edges = ncvar->edges();
  if( nrow == 0 ) {
    nrow = edges[0] - row_index;
  }
  if( ncol == 0 ) {
    ncol = edges[1] - col_index;
  }
  delete [] edges;

  // Read the data

  if( !ncvar->put(var.data(), nrow, ncol) ) {
    throw Exception("cannot write data","NC2Blitz::put");
  }
  ncFile->sync();
}

template <class Type, int N>
void 
NC2Blitz::put_rec(const char* var_name, blitz::Array<Type, N>& var, int rec) {
  NcVar* ncvar = ncFile->get_var(var_name);

  if(var.size() < ncvar->rec_size()) {
    throw Exception("array size < record size","NC2Blitz::put_rec");
  }

  if( !ncvar->put_rec(var.data(), rec) ) {
    throw Exception("cannot write data","NC2Blitz::put_rec");
  }
  ncFile->sync();
}
#endif
