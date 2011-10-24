/*!
  \file makeCurlDivergence.C
  \author E. Rodriguez

  Make the curl and divergence for the filtered wind fields.
*/

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <netcdfcpp.h>

#include "Exception.h"
#include "L2BCconstants.h"
#include "Options.h"
#include "CurlDivergence.h"
#include "numtochar.h"

using namespace std;
using namespace blitz;

const char* USAGE_MESSAGE[] = {
  "filtered l2bc file                              = ! input/output netcdf file",
  "estimation window size                          = 3 ! size of the estimation window for curl divergence (odd > 1)",
  "minimum number of good points                   = 6 ! minimum number of good points needed for fitting",
  "vector field type                               = wind ! wind or stress",
  "output flag bit position                        = 14 ! position of the bad filtering flag bit",
  "number of flag fields                           = 1 ! number of flags to be checked for good data",
  "flag 1 bit position                             = 15 ! flag 1 bit index to check starting from zero",
  "flag 1 bit value                                = 0 ! flag 1 bit value for good data",
  NULL
};

Options init(const char* commandFile,
	     string& filtered_l2bcFile,
	     int& nsmooth,
	     int& min_good,
	     int& output_bit_position,
	     vector<int>& bit_position,
	     vector<bool>& bit_value,
	     VectorFieldType& vector_field_type
	     ){

  Options opt;
  opt.parseFile(commandFile);
 
  filtered_l2bcFile = opt["filtered l2bc file"];
  nsmooth = opt.toInt("estimation window size");
  min_good = opt.toInt("minimum number of good points");
  if( opt["vector field type"] == string("stress") ){
    vector_field_type = STRESS_VECTOR_FIELD;
  }
  else {
    vector_field_type = WIND_VECTOR_FIELD;
  }
  output_bit_position = opt.toInt("output flag bit position");
  int nflags = opt.toInt("number of flag fields");
  bit_position.resize(nflags);
  bit_value.resize(nflags);
  for( int i = 0; i < nflags; i++) {
    string p_kw = string("flag ")+string(itoa(i+1))+string(" bit position");
    string v_kw = string("flag ")+string(itoa(i+1))+string(" bit value");
    bit_position[i] = opt.toInt(p_kw);
    int val = opt.toInt(v_kw);
    if( val == 0 ) {
      bit_value[i] = false;
    }
    else {
      bit_value[i] = true;
    }
  }

  return opt;
}

void usage(const char* program){
  cout<<"Usage: "<<program<<" commandFile"<<endl;
  int i = 0;
  while(USAGE_MESSAGE[i] != NULL) {
    cout<<USAGE_MESSAGE[i++]<<endl;
  }
}

int main(int argc, char* argv[]){

  string filtered_l2bcFile;
  int nsmooth;
  int min_good;
  int output_bit_position;
  vector<int> bit_position;
  vector<bool> bit_value;
  VectorFieldType vector_field_type;

  if(argc != 2){
    usage(argv[0]);
    exit(1);
  }
  const char* commandFile = argv[1];

  // Get RDF file inputs

  Options opt = init(commandFile, filtered_l2bcFile, 
		     nsmooth, min_good, output_bit_position,
		     bit_position, bit_value,
		     vector_field_type);

  // Open the input and output files

  CurlDivergence curl_div(filtered_l2bcFile.c_str(), vector_field_type);
  NC2Blitz nc2b(&curl_div.ncFile);

  // Create the bad data mask

  curl_div.create_input_mask(bit_position, bit_value);

  // Calculate the curl and divergence

  Array<float, 2> curl;
  Array<float, 2> div;
  Array<bool, 2> output_mask;
  float missing_data_value = L2BC_FLOAT_FILL_VALUE;
  int row_radius = (nsmooth - 1)/2;
  int col_radius = (nsmooth - 1)/2;

  curl_div.get_curl_divergence(curl, //!< output curl,
			       div, //!< output divergence
			       output_mask, //!< 0 if datum is good
			       missing_data_value, //!< value to use for fill when estimate is not possible
			       row_radius, //!< number of pixels to left or right of point to use in estimates
			       col_radius, //!< number of pixels up or down from point to use in estimates
			       min_good //!< minimum number of good observations per fit
			       );		    

  if( vector_field_type  == WIND_VECTOR_FIELD ) {
    nc2b.put(L2BC_FILTERED_WIND_CURL_KW,curl);
    nc2b.put(L2BC_FILTERED_WIND_DIVERGENCE_KW,div);
  }
  else {
    nc2b.put(L2BC_FILTERED_WIND_CURL_KW,curl);
    nc2b.put(L2BC_FILTERED_WIND_DIVERGENCE_KW,div);
  }

  // Update flag field given the mask field

  curl_div.update_flags(output_mask, output_bit_position );

  return 0;
}
