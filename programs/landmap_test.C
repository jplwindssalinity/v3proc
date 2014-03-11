//=========================================================//
// Copyright (C) 1999, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    make_simple_land_map
//
// SYNOPSIS
//    make_simple_land_map <original_land_map> <simple_land_map>
//
// DESCRIPTION
//    Reads the land map and produces a simple land map.
//    0 = ocean
//    1 = land
//    2 = coast or near coast
//
// OPERANDS
//    The following operands are supported:
//      <original_land_map>  Input original land map.
//      <output_land_map>    Output simple land map.
//
// EXAMPLES
//    An example of a command line is:
//      % make_simple_land_map landmap.dat outmap.dat
//
// ENVIRONMENT
//    Not environment dependent.
//
// EXIT STATUS
//    The following exit values are returned:
//       0  Program executed successfully
//      >0  Program had an error
//
// NOTES
//    None.
//
// AUTHOR
//    James N. Huddleston (hudd@casket.jpl.nasa.gov)
//----------------------------------------------------------------------

//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
    "@(#) $Id$";

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "Misc.h"
#include "Ephemeris.h"
#include "ConfigList.h"
#include "L1A.h"
#include "ConfigSim.h"
#include "L1BHdf.h"
#include "L1AToL1B.h"
#include "Tracking.h"
#include "QscatConfig.h"
#include "List.h"
#include "BufferedList.h"


using std::list;
using std::map;

//-----------//
// TEMPLATES //
//-----------//

class AngleInterval;

template class List<AngleInterval>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class List<WindVectorPlus>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;


//-----------//
// CONSTANTS //
//-----------//

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

//--------------//
// MAIN PROGRAM //
//--------------//

int
main()
{
  ConfigList oldcfg;
  ConfigList simplecfg;
  ConfigList usgscfg;

  oldcfg.Read("testoldlandmap.rdf");
  simplecfg.Read("testsimplelandmap.rdf");
  usgscfg.Read("testusgslandmap.rdf");


  LandMap old,simple,usgs;
  if(!ConfigLandMap(&old,&oldcfg)){
    fprintf(stderr,"Cannot configure old landmap\n");
    exit(1);
  }
  if(!ConfigLandMap(&simple,&simplecfg)){
    fprintf(stderr,"Cannot configure simple landmap\n");
    exit(1);
  }
  if(!ConfigLandMap(&usgs,&usgscfg)){
    fprintf(stderr,"Cannot configure usgs landmap\n");
    exit(1);
  }

  float lon=5.0*dtr;
  for(float lat=-90;lat<90;lat+=0.01){
    printf("%g %d %d %d\n",
	   lat,old.IsLand(lon,lat*dtr),simple.IsLand(lon,lat*dtr),
	   usgs.IsLand(lon,lat*dtr));
  }
  return(0);
}

