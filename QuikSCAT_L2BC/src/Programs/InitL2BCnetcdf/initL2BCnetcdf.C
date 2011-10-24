/*!
  \file initL2BCnetcdf.C
  \author E. Rodriguez

  Intialize an empty QuikSCAT L2BC data file (no header info).
*/

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <netcdfcpp.h>

#include "Exception.h"
#include "Options.h"

using namespace std;

const char* USAGE_MESSAGE[] = {
  "l2bc file                                       = ",
  NULL
};

Options init(const char* commandFile,
	  string& l2bcFile
	  ){

  Options opt;
  opt.parseFile(commandFile);
 
  l2bcFile = opt["l2bc file"];
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

  string l2bcFile;

  if(argc != 2){
    usage(argv[0]);
    exit(1);
  }
  const char* commandFile = argv[1];

  // Get RDF file inputs

  Options opt = init(commandFile, l2bcFile);


  return 0;
}
