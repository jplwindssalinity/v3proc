/*!
  \file FilterL2BWinds.h
  \author Ernesto Rodriguez
  \brief Filter L2B winds using a separate median filter for speed and direction.
*/

#ifndef _ER_FILTERL2BWINDS_H_
#define _ER_FILTERL2BWINDS_H_

#include <string>
#include <vector>
#include <bitset>
#include <complex>
#include <netcdfcpp.h>
#include <blitz/array.h>
#include "NC2Blitz.h"
#include "Exception.h"
#include "L2BCconstants.h"
#include "median.h"

/*!

  \brief Filter L2B winds using a separate median filter for speed and direction.

  This class provides the interface to read L2B winds from a NetCDF file, filter the
  winds and update the file. In addition to the winds, flags and the model are also read.

*/

class FilterL2BWinds {
public:

  //! Open the netcdf file and read the data

  FilterL2BWinds(const char* unfiltered_l2b_file_name,
		 const char* filtered_l2b_file_name,
		NcError::Behavior b = NcError::verbose_fatal);

  //! Create the mask of good input variables based on the desired list of flags

  void create_input_mask(std::vector<int>& bit_position, //! position of the bit to test
			 std::vector<bool>& bit_value //! value the bit should have
			 );

  //! Filter the directions and return smoothed field and mask

  void filter_directions(int nsmooth, //!< size of smoothing window
			 int min_good, //!< minimum number of good points in window
			 blitz::Array<float, 2>& wind_dir_smooth, //!< returns the smoothed direction
			 blitz::Array<bool, 2>& output_mask //!< returns the output mask
			 );

  //! Filter the speeds and return smoothed field and mask

  void filter_speeds(int nsmooth, //!< size of smoothing window
		       int min_good, //!< minimum number of good points in window
		       blitz::Array<float, 2>& wind_speed_smooth, //!< returns the smoothed direction
		       blitz::Array<bool, 2>& output_mask //!< returns the output mask
		       );

  //! Update the flag values to take into account the smoothing filter mask

  void update_flags(blitz::Array<bool, 2>& output_mask,  //!< Smoothing filter mask
		    int output_bit_position       //!< bit to set if data was not smoothed
		    );

  //! AHChau 6/3/12.  Copy the eflags from the input file to the output file
  void copy_eflags();
  //! AHChau 6/5/12.  When speed is -9999, change direction to be -9999 as well
  void make_dir_consistent_with_spd();

  // Data members

  NcError ncError; //!< netcdf error behavior
  NcFile unfiltered_ncFile; //!< pointer to the open netcdf input file
  NcFile filtered_ncFile; //!< pointer to the open netcdf output file

  blitz::Array<bool, 2> input_mask; 

};

#endif
