#include "CurlDivergence.h"

using namespace std;
using namespace blitz;

CurlDivergence::CurlDivergence(const char* l2b_file_name, 
			       VectorFieldType _vectorFieldType,
			       NcError::Behavior b):
  ncError(b),ncFile(l2b_file_name,NcFile::Write),vectorFieldType(_vectorFieldType) {

  earth_radius = 6378.e3;

  // Make sure the file opened correctly

  if(!ncFile.is_valid()) {
    throw Exception(string("netcdf IO error"),
 		    string("CurlDivergence::CurlDivergence"),
		    string("Cannot open file: ")+string(l2b_file_name));
  }

  // Build the blitz to netcdf bridge and read variables

  NC2Blitz nc2b(&ncFile);

  // Read the lat and lon components in radians

  nc2b.get( L2BC_FILTERED_LAT_KW, lat);
  lat *= DEG2RAD;
  nc2b.get( L2BC_FILTERED_LON_KW, lon);
  lon *= DEG2RAD;

  // Read the magnitude and direction 

  Array<float, 2> dir;
  nc2b.get(L2BC_FILTERED_WIND_DIR_KW,dir);
  dir *= DEG2RAD;

  Array<float, 2> mag;
  if ( vectorFieldType == WIND_VECTOR_FIELD ) {
    nc2b.get(L2BC_FILTERED_WIND_SPEED_KW,mag);
  }
  else {
    nc2b.get(L2BC_FILTERED_STRESS_KW,mag);
  }

  // Resize and calculate meridional and zonal components

  A_phi.resize(mag.rows(),mag.cols()); //!< meridional component (e.g., U)
  A_theta.resize(mag.rows(),mag.cols()); //!< zonal component (e.g., V)

  A_theta = mag*cos(dir);
  A_phi = mag*sin(dir);
}

void 
CurlDivergence::create_input_mask(vector<int>& bit_position, //! position of the bit to test
				  vector<bool>& bit_value //! value the bit should have
				  ) {

  //! Read the L2B flags

  NC2Blitz nc2b(&ncFile);
  Array<short, 2> wvc_quality_flag;
  nc2b.get(L2BC_FILTERED_WVC_QUALITY_FLAG_KW,wvc_quality_flag);

  // Resize the flag array and fill it with the desired values

  input_mask.resize(wvc_quality_flag.rows(), wvc_quality_flag.cols());
  input_mask = true;

  for( int i = 0; i < wvc_quality_flag.rows(); i++ ) {
    for( int j = 0; j < wvc_quality_flag.cols(); j++ ) {
      bitset<16> flag = size_t(wvc_quality_flag(i,j));
      for( size_t k = 0; k < bit_position.size(); k++ ) {
	if ( flag[bit_position[k]] != bit_value[k] ) {
	  input_mask(i,j) = false;
	  break;
	}
      }
    }
  }

}

void 
CurlDivergence::get_curl_divergence(Array<float, 2>& curl, //!< output curl,
				    Array<float, 2>& div, //!< output divergence
				    Array<bool, 2>& output_mask, //!< true if datum is good
				    float missing_data_value,
				    int row_radius, int col_radius,
				    int min_good
				    ) {

  // These are the derivative values at each point

  double dA_phi_dlon;
  double dA_phi_dlat;
  double dA_theta_dlon;
  double dA_theta_dlat;
  double curlij = missing_data_value;
  double divij = missing_data_value;

  bool flag_phi;
  bool flag_theta;

  // Resize output arrays

  int nrows = A_phi.rows();
  int ncols = A_phi.cols();

  curl.resize(nrows, ncols);
  div.resize(nrows, ncols);
  output_mask.resize(nrows, ncols);

  // Fill the edges

  Range all = Range::all();
  Range row_range_low = Range(0,row_radius-1);
  Range row_range_high = Range(A_phi.rows()-row_radius,A_phi.rows()-1);
  Range col_range_low = Range(0,col_radius-1);
  Range col_range_high = Range(A_phi.cols()-col_radius,A_phi.cols()-1);

  output_mask(row_range_low,all) = false;
  output_mask(row_range_high,all) = false;
  output_mask(all,col_range_low) = false;
  output_mask(all,col_range_high) = false;

  curl(row_range_low,all) = missing_data_value;
  curl(row_range_high,all) = missing_data_value;
  curl(all,col_range_low) = missing_data_value;
  curl(all,col_range_high) = missing_data_value;

  // AHChau 6/3/12.  Add next 4 lines to initialize edges for div
  div(row_range_low,all) = missing_data_value;
  div(row_range_high,all) = missing_data_value;
  div(all,col_range_low) = missing_data_value;
  div(all,col_range_high) = missing_data_value;

  // Loop over all points

  for (int i = row_radius; i < nrows-row_radius; i++) {
    for (int j = col_radius; j < ncols-col_radius; j++) {

      // Get the derivatives of A_phi and A_theta

      flag_phi = get_lat_lon_derivative(A_phi,i,j,row_radius,col_radius,min_good,dA_phi_dlon,dA_phi_dlat);
      flag_theta = get_lat_lon_derivative(A_theta,i,j,row_radius,col_radius,min_good,dA_theta_dlon,dA_theta_dlat);

      if( ( flag_phi == false ) || ( flag_theta == false) ) {
	output_mask(i,j) = false;
	curl(i,j) = missing_data_value;
	div(i,j) = missing_data_value;
      }
      else {
	output_mask(i,j) = true;

	get_curl_div_from_derivs(double(lat(i,j)), //!< latitude in radians
				 double(A_phi(i,j)), double(A_theta(i,j)), 
				 dA_phi_dlat, dA_phi_dlon,
				 dA_theta_dlat, dA_theta_dlon,
				 curlij, divij);
	curl(i,j) = curlij;
	div(i,j) = divij;
      }
    }
  }

}		    

bool 
CurlDivergence::get_lat_lon_derivative(blitz::Array<float, 2>& A,
				       int i,
				       int j,
				       int row_radius,
				       int col_radius,
				       int min_good,
				       double& dA_dlon,
				       double& dA_dlat
				       ) {
  
  Array<double, 2> X; // This is the matrix to solve XC = y
  Array<double, 1> Y;
  Array<double, 1> C(2);
  Array<double, 2> Cov(2,2);
  double chisq;
  int status;

  int ngood = load_sensitivity_matrix( A, i, j, row_radius, col_radius, X, Y );

  if ( ngood < min_good ) {
    return false;
  }

  status = multifit_linear(X, Y, C, Cov, chisq);

  if (status) {
    cout<<"CurlDivergence::get_lat_lon_derivative: multifit_linear exited with status: "<<gsl_strerror (status)<<endl;
    return false;
  }

  dA_dlon = C(0);
  dA_dlat = C(1);

  return true;
}

int 
CurlDivergence::load_sensitivity_matrix( Array<float, 2>& A,
				       int icenter,
				       int jcenter,
				       int row_radius,
				       int col_radius,
				       Array<double, 2>& X, // This is the matrix to solve XC = y
				       Array<double, 1>& Y
				       ) {
  int ngood = 0;
  int max_good = (2*row_radius + 1)*(2*col_radius + 1); // maximum number of good observations

  // Resize to the largest possible size

  X.resize(max_good,2);
  Y.resize(max_good);

  // AHChau 6/3/12. Add the if statement to not report good values if the center value is invalid
  if( input_mask(icenter,jcenter) ){
    for( int i = icenter - row_radius; i <= icenter + row_radius; i++ ){
      for( int j = jcenter - col_radius; j <= jcenter + col_radius; j++ ){
	if( input_mask(i,j) == true ) { // Valid data
	  double dlat = lat(i,j) - lat(icenter,jcenter);
	  double dlon = lon(i,j) - lon(icenter,jcenter);
	  X(ngood,0) = dlon;
	  X(ngood,1) = dlat;
	  Y(ngood) = A(i,j) - A(icenter,jcenter);
	  ngood++;
	}
      }
    }
  } 

  X.resizeAndPreserve(ngood,2);
  Y.resizeAndPreserve(ngood);

  return ngood;
}

void
CurlDivergence::get_curl_div_from_derivs(double lat, double A_phi, double A_theta, 
					 double dA_phi_dlat, double dA_phi_dlon,
					 double dA_theta_dlat, double dA_theta_dlon,
					 double& curl, double& div) {

  double sin_lat = sin(lat);
  double cos_lat = cos(lat);
  double rsin_lat_inv = 1./(earth_radius*cos_lat);

  // AHChau 6/3/12.  Change sign of curl (request from Svetla)
  //curl = rsin_lat_inv*(-sin_lat*A_phi + cos_lat*dA_phi_dlat - dA_theta_dlon);
  curl = -1.*rsin_lat_inv*(-sin_lat*A_phi + cos_lat*dA_phi_dlat - dA_theta_dlon);
  div = rsin_lat_inv*(-sin_lat*A_theta + cos_lat*dA_theta_dlat + dA_phi_dlon);
}

void
CurlDivergence::update_flags(Array<bool, 2>& output_mask, int output_bit_position) {
  
  //! Read the L2B flags

  NC2Blitz output_nc2b(&ncFile);
  Array<short, 2> wvc_quality_flag;
  output_nc2b.get(L2BC_FILTERED_WVC_QUALITY_FLAG_KW,wvc_quality_flag);

  // Check that sizes are consistent

  if( ( wvc_quality_flag.rows() != output_mask.rows() ) ||
      ( wvc_quality_flag.cols()) != output_mask.cols() ) {
    throw Exception("Cannot update wind flags",
		    "CurlDivergence::update_flags",
		    "Incompatible mask and flag sizes");
  }

  for( int i = 0; i < wvc_quality_flag.rows(); i++ ) {
    for( int j = 0; j < wvc_quality_flag.cols(); j++ ) {
      bitset<16> flag = size_t(wvc_quality_flag(i,j));
      if( output_mask(i,j) == false ) {
	flag.set(output_bit_position,1);
	wvc_quality_flag(i,j) = short(flag.to_ulong());
      }
      else {
	flag.set(output_bit_position,0);
	wvc_quality_flag(i,j) = short(flag.to_ulong());
      }
    }
  }

  // Write out the updated fields

  output_nc2b.put(L2BC_FILTERED_WVC_QUALITY_FLAG_KW,wvc_quality_flag);

}
