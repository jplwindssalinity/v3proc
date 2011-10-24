/*!

  \file NC2Options.h
  \author E. Rodriguez
  \brief Write Options object to NetCDF metdata or read NectCDF metadata into an Options object. 

 */

#ifndef _ER_NC2OPTIONS_H_
#define _ER_NC2OPTIONS_H_

#include <netcdfcpp.h>
#include <string>
#include "Options.h"
#include "numtochar.h"

//! Given an open NcFile object and an Options object, write all the options as NetCDF global header metdata.

void Options2NC(NcFile& ncFile, Options& opt);

//! Given an open NcFile object, a var name, and an Options object, write all the options as NetCDF metdata 
//! for that variable.

void Options2NC(NcFile& ncFile, Options& opt, const char* var_name);

//! Read the global attributes from an ncFile and return an options object

Options NC2Options(NcFile& ncFile);


#endif
