/*!
  \file CurlDivergence.h
  \author Ernesto Rodriguez
  \brief Make the curl and divergence of a vector field sampled on a lat/lon grid.
*/

#ifndef _ER_CURLDIVERGENCE_H_
#define _ER_CURLDIVERGENCE_H_

#include <iostream>
#include <vector>
#include <bitset>
#include <netcdfcpp.h>
#include <blitz/array.h>
#include <gsl/gsl_errno.h>
#include "Blitz2GSL.h"
#include "NC2Blitz.h"
#include "Exception.h"
#include "L2BCconstants.h"

#define DEG2RAD 0.017453292519943295

typedef enum {WIND_VECTOR_FIELD, STRESS_VECTOR_FIELD} VectorFieldType;

/*!
  \brief Make the curl and divergence of a vector field sampled on a lat/lon grid.
*/

class CurlDivergence {
public:

  //! Open the netcdf file and read the data

  CurlDivergence(const char* l2b_file_name,
		 VectorFieldType vectorFieldType = WIND_VECTOR_FIELD,
		 NcError::Behavior b = NcError::verbose_fatal);

  //! Create the mask of good input variables based on the desired list of flags

  void create_input_mask(std::vector<int>& bit_position, //! position of the bit to test
			 std::vector<bool>& bit_value //! value the bit should have
			 );

  //! Update the flag values to take into account the smoothing filter mask

  void update_flags(blitz::Array<bool, 2>& output_mask,  //!< Smoothing filter mask
		    int output_bit_position       //!< bit to set if data was not smoothed
		    );

  //! Compute the curl and divergence

  void get_curl_divergence(blitz::Array<float, 2>& curl, //!< output curl,
			   blitz::Array<float, 2>& div, //!< output divergence
			   blitz::Array<bool, 2>& output_mask, //!< 0 if datum is good
			   float missing_data_value, //!< value to use for fill when estimate is not possible
			   int row_radius, //!< number of pixels to left or right of point to use in estimates
			   int col_radius, //!< number of pixels up or down from point to use in estimates
			   int min_good //!< minimum number of good observations per fit
			   );	

  // Data members

  double earth_radius;

  NcError ncError; //!< netcdf error behavior
  NcFile ncFile; //!< pointer to the open netcdf file
  
  VectorFieldType vectorFieldType;

  blitz::Array<bool, 2> input_mask; 

  blitz::Array<float, 2> A_phi; //!< meridional component (e.g., U)
  blitz::Array<float, 2> A_theta; //!< zonal component (e.g., V)
  blitz::Array<float, 2> lat; //!< latitude (in degrees)
  blitz::Array<float, 2> lon; //!< latitude (in degrees)

  // blitz::Array<float, 2> curl; //!< curl of the vector field
  // blitz::Array<float, 2> div; //!< divergence of the vector field

  ////////////////////////////////////////////////////////////////////////
  //
  // Some auxiliary functions
  //
  ////////////////////////////////////////////////////////////////////////

  // Get the derivatives of a vector field. Return true if successful, false if not.

  bool get_lat_lon_derivative(blitz::Array<float, 2>& A,
				       int i,
				       int j,
				       int row_radius,
				       int col_radius,
				       int min_good,
				       double& dA_dlon,
				       double& dA_dlat
				       );
  // Load the sensitivity matrix

 int load_sensitivity_matrix( blitz::Array<float, 2>& A,
			      int i,
			      int j,
			      int row_radius,
			      int col_radius,
			      blitz::Array<double, 2>& X, // This is the matrix to solve XC = y
			      blitz::Array<double, 1>& Y);

 // Get the curl and divergence from the field components and derivatives

 void get_curl_div_from_derivs(double lat, //!< latitude in radians
			       double A_phi, double A_theta, 
			       double dA_phi_dlat, double dA_phi_dlon,
			       double dA_theta_dlat, double dA_theta_dlon,
			       double& curl, double& div);

};


#endif
