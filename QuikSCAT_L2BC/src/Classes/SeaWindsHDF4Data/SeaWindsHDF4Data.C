#include "SeaWindsHDF4Data.h"

using namespace std;
using namespace blitz;

SeaWindsHDF4Data::SeaWindsHDF4Data(const char* fileName, int32 mode): sd_id(SDstart(fileName, mode)) {
  if( sd_id == FAIL ) {
    throw Exception(string("HDF4 IO error"),
 		    string("SeaWindsHDF4Data::SeaWindsHDF4Data"),
		    string("Cannot open file: ")+string(fileName));
  }

    
}

float
SeaWindsHDF4Data::get_scale_factor(const char* sds_name) {

  intn status;

  // Find the sds_id corresponding to this variable

  int32 sds_index = SDnametoindex (sd_id, sds_name);
  if( sds_index == FAIL ) {
    throw Exception("cannot find HDF4 sds_name","HDF42Blitz::get",sds_name);
  }

  int32 sds_id = SDselect (sd_id, sds_index);

  // Read the scale factor, if it exists

  double scale = 1.;
  int32 attr_index = SDfindattr(sds_id, "scale_factor");
  if( attr_index == FAIL ) {
    status = SDendaccess(sds_id);
    return scale;
  }
  else {

    // The routine reads the contents of the attribute. 

    status = SDreadattr(sds_id, attr_index,&scale); 

    return scale;
  }

  status = SDendaccess(sds_id);

  return scale;
  
}

void
SeaWindsHDF4Data::get(const char* sds_name, Array<float, 2>& data) {

  HDF42Blitz h2b(sd_id);

  Array<short, 2> idata;
  h2b.get(sds_name, idata);

  data.resize(idata.rows(), idata.cols());

  data = idata*this->get_scale_factor(sds_name);

}

