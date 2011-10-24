/*!
  \file HDF42Blitz.h
  \author E. Rodriguez
  \brief A class to read and write HDF4 files using blitz++ arrays.
*/

#ifndef _ER_HDF42BLITZ_H_
#define _ER_HDF42BLITZ_H_

#include <iostream>
#include <mfhdf.h>
#include <blitz/array.h>
#include "Exception.h"


/*!
  \brief A class to read and write HDF4 files using blitz++ arrays.
*/

class HDF42Blitz {
public:

  //! Initialize with an identifier to an HDF4 file that has beed opened with SDstart

  HDF42Blitz(int32 _sd_id):sd_id(_sd_id) {}

  //! The template get function retrieves 1D data given the variable name. The data type and dimension is
  //! inferred from the array passed. The array is resized to fit the data request, so it needs to be declared
  //! but no memory needs to be allocated.

  template <class Type>
  void get(const char* sds_name, blitz::Array<Type, 1>& var, size_t n = 0, int start_index = 0);

  //! The template get functions retrieves 2D data given the variable name. The data type and dimension is
  //! inferred from the array passed. The array is resized to fit the data request, so it needs to be declared
  //! but no memory needs to be allocated.

  template <class Type>
  void get(const char* sds_name, blitz::Array<Type, 2>& var, 
	   size_t nrow = 0, size_t ncol = 0, int row_index = 0, int col_index = 0);


  //! The template put function writes 1D data given the variable name. The data type and dimension is
  //! inferred from the array passed. 

  // template <class Type>
  // void put(const char* var_name, blitz::Array<Type, 1>& var, size_t n = 0, int start_index = 0);

  //! The template get functions writes 2D data given the variable name. The data type and dimension is
  //! inferred from the array passed. 

  // template <class Type>
  // void put(const char* var_name, blitz::Array<Type, 2>& var, size_t nrow = 0, size_t ncol = 0, int row_index = 0, int col_index = 0);

  //! The template put function writes a record stored in a 1D array giventhe variable name. The data type and dimension is
  //! inferred from the array passed. The shape of the array does not have to be the same as the variable record shape, it just
  //! has to have a size greater than or equal to the record size.

  // template <class Type, int N>
  // void put_rec(const char* var_name, blitz::Array<Type, N>& var, int rec);


  // Keep a copy of the identifier to an HDF4 file that has beed opened with SDstart

  int32 sd_id;

};

template <class Type>
void 
HDF42Blitz::get(const char* sds_name, blitz::Array<Type, 1>& var, size_t n, int start_index) {

  // Find the sds_id corresponding to this variable

  int32 sds_index = SDnametoindex (sd_id, sds_name);
  if( sds_index == FAIL ) {
    throw Exception("cannot find HDF4 sds_name","HDF42Blitz::get",sds_name);
  }

  int32 sds_id = SDselect (sd_id, sds_index);

  // Make sure it is the right rank for the Blitz array, and get the size

  int32 rank, data_type, n_attrs;
  char  name[H4_MAX_NC_NAME];
  int32 dim_sizes[H4_MAX_VAR_DIMS];
  intn status = SDgetinfo(sds_id, name, &rank, dim_sizes, &data_type, &n_attrs);

  if( rank != 1 ) {
    throw Exception("data dimension not equal to 1 (array dimension)","HDF42Blitz::get");
  }

  // set the starting index

  int32 start[1];
  start[0] = start_index;
  
  // If the number of desired points is 0, set to the maximum allowed

  if( n == 0 ) {
    n = dim_sizes[0] - start_index;
  }

  // Resize the array, if required
  
  if( var.size() != n) {
    var.resize(n);
  }

  // Set the edge size

  int32 edge[1];
  edge[0] = n;

  // Read the data

  status = SDreaddata(sds_id, start, NULL, edge, VOIDP(var.data()) );

  if( status == FAIL ) {
    status = SDendaccess (sds_id);
    throw Exception("cannot read data","HDF42Blitz::get");
  }

  // Terminate access to this data set

  status = SDendaccess(sds_id);

}

template <class Type>
void 
HDF42Blitz::get(const char* sds_name, blitz::Array<Type, 2>& var, 
		size_t nrow, size_t ncol, int row_index, int col_index) {

  // Find the sds_id corresponding to this variable

  int32 sds_index = SDnametoindex (sd_id, sds_name);
  if( sds_index == FAIL ) {
    throw Exception("cannot find HDF4 sds_name","HDF42Blitz::get",sds_name);
  }

  int32 sds_id = SDselect (sd_id, sds_index);

  // Make sure it is the right rank for the Blitz array, and get the size

  int32 rank, data_type, n_attrs;
  char  name[H4_MAX_NC_NAME];
  int32 dim_sizes[H4_MAX_VAR_DIMS];
  intn status = SDgetinfo(sds_id, name, &rank, dim_sizes, &data_type, &n_attrs);
  // std::cout<<"name: "<<rank<<std::endl;
  // std::cout<<"rank: "<<rank<<std::endl;
  // std::cout<<"data_type: "<<data_type<<std::endl;
  // std::cout<<"n_attrs: "<<n_attrs<<std::endl;

  if( rank != 2 ) {
    throw Exception("data dimension not equal to 2 (array dimension)","HDF42Blitz::get");
  }

  // set the starting index

  int32 start[2];
  start[0] = row_index;
  start[1] = col_index;
  
  // If the number of desired points is 0, set to the maximum allowed

  if( nrow == 0 ) {
    nrow = dim_sizes[0] - row_index;
  }
  if( ncol == 0 ) {
    ncol = dim_sizes[1] - col_index;
  }

  // Resize the array, if required
  
  var.resize(nrow, ncol);

  // Set the edge size

  int32 edge[2];
  edge[0] = nrow;
  edge[1] = ncol;

  // Read the data

  status = SDreaddata(sds_id, start, NULL, edge, VOIDP(var.data()) );

  if( status == FAIL ) {
    status = SDendaccess (sds_id);
    throw Exception("cannot read data","HDF42Blitz::get");
  }

  // Terminate access to this data set

  status = SDendaccess(sds_id);

}
  

// template <class Type>
// void 
// HDF42Blitz::put(const char* var_name, blitz::Array<Type, 1>& var, size_t n, int start_index) {
  
//   NcVar* ncvar = ncFile->get_var(var_name);

//   // set the starting index

//   if(!ncvar->set_cur(start_index)) {
//     throw Exception("cannot move start_index to desired location","HDF42Blitz::put");
//   }

//   // If the number of desired points is 0, set to the maximum allowed

//   if( n == 0 ) {
//     n = ncvar->num_vals() - start_index;
//   }

//   // write the data

//   if( !ncvar->put(var.data(), n) ) {
//     throw Exception("cannot write data","HDF42Blitz::put");
//   }
//   ncFile->sync();
// }

// template <class Type>
// void 
// HDF42Blitz::put(const char* var_name, blitz::Array<Type, 2>& var, size_t nrow, size_t ncol, int row_index, int col_index) {
  
//   NcVar* ncvar = ncFile->get_var(var_name);

//   if(ncvar->num_dims() != 2) {
//     throw Exception("data dimension not equal to 2 (array dimension)","HDF42Blitz::put");
//   }
//   // set the starting index

//   if(!ncvar->set_cur(row_index, col_index)) {
//     throw Exception("cannot move start_index to desired location","HDF42Blitz::put");
//   }

//   // If the number of desired points is 0, set to the maximum allowed

//   long* edges = ncvar->edges();
//   if( nrow == 0 ) {
//     nrow = edges[0] - row_index;
//   }
//   if( ncol == 0 ) {
//     ncol = edges[1] - col_index;
//   }
//   delete [] edges;

//   // Read the data

//   if( !ncvar->put(var.data(), nrow, ncol) ) {
//     throw Exception("cannot write data","HDF42Blitz::put");
//   }
//   ncFile->sync();
// }

// template <class Type, int N>
// void 
// HDF42Blitz::put_rec(const char* var_name, blitz::Array<Type, N>& var, int rec) {
//   NcVar* ncvar = ncFile->get_var(var_name);

//   if(var.size() < ncvar->rec_size()) {
//     throw Exception("array size < record size","HDF42Blitz::put_rec");
//   }

//   if( !ncvar->put_rec(var.data(), rec) ) {
//     throw Exception("cannot write data","HDF42Blitz::put_rec");
//   }
//   ncFile->sync();
// }
#endif
