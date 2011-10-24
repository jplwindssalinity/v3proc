/*!
  Tests the functionality of the class HDF42Blitz and SeaWindsHDF4Data classes.
*/

#include <iostream>
#include <blitz/array.h>
#include "Options.h"
#include "HDF42Blitz.h"
#include "SeaWindsHDF4Data.h"

using namespace std;
using namespace blitz;

const char* USAGE_MESSAGE[] = {
  "l2b file                                        = ../../../data/L2B/QS_S2B44444.20080021548",
  NULL
};

Options init(const char* commandFile,
	  string& seaWindsFile
	  ){

  Options opt;
  opt.parseFile(commandFile);
 
  seaWindsFile = opt["l2b file"];

  return opt;
}

void usage(const char* program){
  cout<<"Usage: "<<program<<" commnadFile"<<endl;
  int i = 0;
  while(USAGE_MESSAGE[i] != NULL) {
    cout<<USAGE_MESSAGE[i++]<<endl;
  }
}

int main(int argc, char* argv[]) {
  
  string seaWindsFile;

  if(argc != 2){
    usage(argv[0]);
    exit(1);
  }
  const char* commandFile = argv[1];

  // Get RDF file inputs

  init(commandFile, seaWindsFile);

  // Open the HDF4 file for reading

  SeaWindsHDF4Data l2b(seaWindsFile.c_str(), DFACC_READ);
  cout<<"sd_id: "<<l2b.sd_id<<endl;

  // Test the scale factor

  float scale = l2b.get_scale_factor("wind_speed_selection");
  cout<<"wind speed scale factor: "<<scale<<endl;

  // Initialize the HDF4 bridge

  HDF42Blitz h2b(l2b.sd_id);

  Array<short, 2> speed;
  h2b.get("wind_speed_selection",speed);

  cout<<"( "<<speed.rows()<<" , "<<speed.cols()<<" )"<<endl;

  cout<<speed(Range::all(),speed.cols()/2)<<endl;

  // Test the reading of the data

  Array<float, 2> fspeed;
  l2b.get("wind_speed_selection",fspeed);

  cout<<fspeed(Range::all(),fspeed.cols()/2)<<endl;

  return 0;
}
