/*!
  \file makeFilteredWinds.C
  \author E. Rodriguez

  Update the wind speed and direction from an l2bc file by median filtering.
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
#include "FilterL2BWinds.h"
#include "numtochar.h"

using namespace std;
using namespace blitz;

const char* USAGE_MESSAGE[] = {
  "unfiltered l2bc file                            = ! input netcdf file",
  "filtered l2bc file                              = ! output netcdf file",
  "smoothing window size                           = 3 ! size of the smoothing window",
  "minimum number of good points                   = 6 ! minimum number of good points needed for smoothing",
  "output flag bit position                        = 15 ! position of the bad filtering flag bit",
  "number of flag fields                           = 2 ! number of flags to be checked for good data",
  "flag 1 bit position                             = 9 ! flag 1 bit index to check starting from zero",
  "flag 1 bit value                                = 0 ! flag 1 bit value for good data",
  "flag 2 bit position                             = 13 ! flag 2 bit index to check starting from zero",
  "flag 2 bit value                                = 0 ! flag 2 bit value for good data",
  NULL
};

Options init(const char* commandFile,
	     string& unfiltered_l2bcFile,
	     string& filtered_l2bcFile,
	     int& nsmooth,
	     int& min_good,
	     int& output_bit_position,
	     vector<int>& bit_position,
	     vector<bool>& bit_value
	     ){

  Options opt;
  opt.parseFile(commandFile);
 
  unfiltered_l2bcFile = opt["unfiltered l2bc file"];
  filtered_l2bcFile = opt["filtered l2bc file"];
  nsmooth = opt.toInt("smoothing window size");
  min_good = opt.toInt("minimum number of good points");
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
  cout<<"Usage: "<<program<<" commnadFile"<<endl;
  int i = 0;
  while(USAGE_MESSAGE[i] != NULL) {
    cout<<USAGE_MESSAGE[i++]<<endl;
  }
}

int main(int argc, char* argv[]){

  string unfiltered_l2bcFile;
  string filtered_l2bcFile;
  int nsmooth;
  int min_good;
  int output_bit_position;
  vector<int> bit_position;
  vector<bool> bit_value;

  if(argc != 2){
    usage(argv[0]);
    exit(1);
  }
  const char* commandFile = argv[1];

  // Get RDF file inputs

  Options opt = init(commandFile, unfiltered_l2bcFile, filtered_l2bcFile, 
		     nsmooth, min_good, output_bit_position,
		     bit_position, bit_value);


  // Open the input and output files

  FilterL2BWinds l2bc(unfiltered_l2bcFile.c_str(),filtered_l2bcFile.c_str());
  NC2Blitz filtered_nc2b(&l2bc.filtered_ncFile);

  // Create the bad data mask

  l2bc.create_input_mask(bit_position, bit_value);

  // Filter the directions and update the file

  Array<float, 2> buffer;
  Array<bool, 2> wind_dir_mask;
  l2bc.filter_directions(nsmooth, min_good, buffer, wind_dir_mask);

  filtered_nc2b.put(L2BC_FILTERED_WIND_DIR_KW,buffer);

  // Filter the speeds

  Array<bool, 2> wind_speed_mask;
  l2bc.filter_speeds(nsmooth, min_good, buffer, wind_speed_mask);

  filtered_nc2b.put(L2BC_FILTERED_WIND_SPEED_KW,buffer);

  // Get the final mask

  wind_dir_mask = ( wind_dir_mask && wind_speed_mask );

  // Update flag field given the mask field

  l2bc.update_flags(wind_dir_mask, output_bit_position );

  return 0;
}
