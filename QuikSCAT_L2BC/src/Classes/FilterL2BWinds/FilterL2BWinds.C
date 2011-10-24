#include "FilterL2BWinds.h"

using namespace std;
using namespace blitz;

FilterL2BWinds::FilterL2BWinds(const char* unfiltered_l2b_file_name, 
			       const char* filtered_l2b_file_name, 
			       NcError::Behavior b):
  ncError(b),unfiltered_ncFile(unfiltered_l2b_file_name,NcFile::ReadOnly),
  filtered_ncFile(filtered_l2b_file_name,NcFile::Write) {

  if(!unfiltered_ncFile.is_valid()) {
    throw Exception(string("netcdf IO error"),
 		    string("FilterL2BWinds::FilterL2BWinds"),
		    string("Cannot open file: ")+string(unfiltered_l2b_file_name));
  }
  if(!filtered_ncFile.is_valid()) {
    throw Exception(string("netcdf IO error"),
 		    string("FilterL2BWinds::FilterL2BWinds"),
		    string("Cannot open file: ")+string(filtered_l2b_file_name));
  }

}

void 
FilterL2BWinds::create_input_mask(std::vector<int>& bit_position, //! position of the bit to test
				  std::vector<bool>& bit_value //! value the bit should have
				  ) {

  //! Read the L2B flags

  NC2Blitz input_nc2b(&unfiltered_ncFile);
  Array<short, 2> wvc_quality_flag;
  input_nc2b.get(L2BC_WVC_QUALITY_FLAG_KW,wvc_quality_flag);

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
FilterL2BWinds::filter_directions(int nsmooth, int min_good,
				  Array<float, 2>& wind_dir_smooth,
				  Array<bool, 2>& output_mask) {

  const complex<float> deg2arg = complex<float>(0.,M_PI/180.);
  const complex<float> I = complex<float>(0.,1.);
  const float rad2deg = 180./M_PI;
  
  // Open blitz connection to the file

  NC2Blitz input_nc2b(&unfiltered_ncFile);

  // Read the measured wind direction (prior to filter) and compute zeta
  
  Array<float, 2> buffer;
  input_nc2b.get(L2BC_WIND_DIR_KW,buffer);

  Array< complex<float>, 2> zeta(buffer.shape());
  zeta = exp(deg2arg*buffer);
  
  // Read the model wind direction (prior to filter) and compute zeta_model
  
  input_nc2b.get(L2BC_MODEL_WIND_DIR_KW,buffer);

  Array< complex<float>, 2> zeta_model(buffer.shape());
  zeta_model = exp(deg2arg*buffer);

  // Compute the delta direction

  zeta /= zeta_model;

  buffer = arg(zeta);

  // Recover some memory deleting unused arrays

  zeta.free();

  // Make sure the input_mask has been created and declare the output mask

  if( ( input_mask.rows() != buffer.rows() ) ||
      ( input_mask.cols() != buffer.cols() ) ) {
    throw Exception("Direction filter failed","FilterL2BWinds::filter_directions",
		    "input_mask not initialized prior to filtering");
  }

  output_mask.resize(input_mask.shape());

  // Filter the delta directions

  wind_dir_smooth.resize(buffer.shape());

  median(buffer, //!< Matrix to smooth (not changed)
  	 input_mask, //!< when false, do not use these points
  	 min_good,   //!< Minimum number of good points in the window
  	 nsmooth,   //!< smoothing size for rows
  	 nsmooth,  //!< smoothing size for columns
  	 wind_dir_smooth, //!< Median filtered result (resized to match input array)
  	 output_mask //!< These are the filtered points mask
  	 );


  // Recover some memory deleting unused arrays

  buffer.free();

  // Put the model directions back in, and convert to degrees

  wind_dir_smooth = arg(exp(I*wind_dir_smooth)*zeta_model)*rad2deg;
  zeta_model.free();

  // The direction is defined [0,360), not [-180,180), so fix that

  wind_dir_smooth = where(wind_dir_smooth < 0., wind_dir_smooth + 360., wind_dir_smooth);

  // Compute the conservative mask: data not filtered & data flagged

  output_mask = ( input_mask && output_mask );

}

void 
FilterL2BWinds::filter_speeds(int nsmooth, int min_good,
			      Array<float, 2>& wind_speed_smooth,
			      Array<bool, 2>& output_mask) {

  // Open blitz connection to the file

  NC2Blitz input_nc2b(&unfiltered_ncFile);

  // Read the measured wind direction (prior to filter) and compute zeta
  
  Array<float, 2> wind_speed;
  input_nc2b.get(L2BC_WIND_SPEED_KW,wind_speed);

  // Read the model wind speed (prior to filter) 
  
  Array<float, 2> wind_speed_model;
  input_nc2b.get(L2BC_MODEL_WIND_SPEED_KW,wind_speed_model);

  // Compute the delta speed

  wind_speed -= wind_speed_model;

  // Make sure the input_mask has been created and declare the output mask

  if( ( input_mask.rows() != wind_speed.rows() ) ||
      ( input_mask.cols() != wind_speed.cols() ) ) {
    throw Exception("Speed filter failed","FilterL2BWinds::filter_speeds",
		    "input_mask not initialized prior to filtering");
  }

  output_mask.resize(input_mask.shape());

  // Filter the delta directions

  wind_speed_smooth.resize(wind_speed.shape());

  median(wind_speed, //!< Matrix to smooth (not changed)
  	 input_mask, //!< when false, do not use these points
  	 min_good,   //!< Minimum number of good points in the window
  	 nsmooth,   //!< smoothing size for rows
  	 nsmooth,  //!< smoothing size for columns
  	 wind_speed_smooth, //!< Median filtered result (resized to match input array)
  	 output_mask //!< These are the filtered points mask
  	 );

  // Put the model directions back in

  wind_speed_smooth += wind_speed_model;

  // Free excess memory

  wind_speed_model.free();
  wind_speed.free();

  // Speeds must be > 0.

  wind_speed_smooth = where(wind_speed_smooth < 0., L2BC_FLOAT_FILL_VALUE, wind_speed_smooth);
  output_mask = where(wind_speed_smooth < 0., false, output_mask);

  // Compute the conservative mask: data not filtered & data flagged

  output_mask = ( input_mask && output_mask );

}

void
FilterL2BWinds::update_flags(Array<bool, 2>& output_mask, int output_bit_position) {
  //! Read the L2B flags

  NC2Blitz output_nc2b(&filtered_ncFile);
  Array<short, 2> wvc_quality_flag;
  output_nc2b.get(L2BC_FILTERED_WVC_QUALITY_FLAG_KW,wvc_quality_flag);

  // Check that sizes are consistent

  if( ( wvc_quality_flag.rows() != output_mask.rows() ) ||
      ( wvc_quality_flag.cols()) != output_mask.cols() ) {
    throw Exception("Cannot update wind flags",
		    "FilterL2BWinds::update_flags",
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
