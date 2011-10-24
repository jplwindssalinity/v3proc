/*!

  \file SeaWindsHDF4Data.h
  \author E. Rodriguez
  \brief Read SeaWinds HDF4 data.

 */

#ifndef _ER_SEAWINDSHDF4DATA_H_
#define _ER_SEAWINDSHDF4DATA_H_

#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <mfhdf.h>
#include <blitz/array.h>
#include "Exception.h"
#include "HDF42Blitz.h"


/*!
  \brief Read SeaWinds HDF4 data.

 */

class SeaWindsHDF4Data {

public:

   SeaWindsHDF4Data(const char* fileName, 
		    int32 mode = DFACC_READ
		    );

  void get(const char* sds_name, blitz::Array<float, 2>& data);

  float get_scale_factor(const char* sds_name);

  // Public data members
  // HDF4 file identifier

  int32 sd_id;
};


#endif
