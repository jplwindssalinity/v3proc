//==============================================================//
// Copyright (C) 2012, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    l2a_to_l2b_spdret
//
// SYNOPSIS
//    l2a_to_l2b_spdret <sim_config_file>
//
// DESCRIPTION
//    Retrieves wind speed using the nudge direction as the wind direction.
//
// OPTIONS
//
// OPERANDS
//    The following operand is supported:
//      <sim_config_file>  The sim_config_file needed listing
//                         all input parameters, input files, and
//                         output files.
//
// EXAMPLES
//    An example of a command line is:
//      % l2a_to_l2b_spdret sws1b.cfg
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
// AUTHORS
//    James N. Huddleston (James.N.Huddleston@jpl.nasa.gov)
//    Alex Fore
//----------------------------------------------------------------------

//-----------------------//
// Configuration Control //
//-----------------------//

static const char l2a_to_l2b_spdret_c[] =
    "@(#) $Id$";


//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "ConfigList.h"
#include "L2A.h"
#include "ConfigSim.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Tracking.h"
#include "Tracking.C"
#include "Array.h"
#include "Meas.h"
#include "GSparameters.h"

using std::list;
using std::map; 

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<AngleInterval>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;

//-----------//
// CONSTANTS //
//-----------//

#define MAX_ALONG_TRACK_BINS  1624
#define OPTSTRING "iRt:a:n:w:N"

//-------//
// HACKS //
//-------//

//#define LATLON_LIMIT_HACK

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


int
main( int    argc,
      char*  argv[])  {
  
  const char* command     = no_path(argv[0]);
  const char* config_file = argv[1];

  ConfigList config_list;
  if (! config_list.Read(config_file)) {
    fprintf(stderr, "%s: error reading sim config file %s\n", command, config_file);
    exit(1);
  }
  
  L2A l2a;
  if (! ConfigL2A(&l2a, &config_list)) {
    fprintf(stderr, "%s: error configuring Level 2A Product\n", command);
    exit(1);
  }

  L2B l2b;
  if (! ConfigL2B(&l2b, &config_list)) {
    fprintf(stderr, "%s: error configuring Level 2B Product\n", command);
    exit(1);
  }
  
  GMF gmf;
  if (! ConfigGMF(&gmf, &config_list)) {
    fprintf(stderr, "%s: error configuring GMF\n", command);
    exit(1);
  }
  
  Kp kp;
  if (! ConfigKp(&kp, &config_list)) {
    fprintf(stderr, "%s: error configuring Kp\n", command);
    exit(1);
  }

  L2AToL2B l2a_to_l2b;
  if (! ConfigL2AToL2B(&l2a_to_l2b, &config_list)) {
    fprintf(stderr, "%s: error configuring L2AToL2B\n", command);
    exit(1);
  }
    
  l2a.OpenForReading();
  l2b.OpenForWriting();
  
  if (! l2a.ReadHeader()) {
    fprintf(stderr, "%s: error reading Level 2A header\n", command);
    exit(1);
  }
  
  int along_track_bins =  (int)(two_pi * r1_earth / l2a.header.alongTrackResolution + 0.5);
 
  if (! l2b.frame.swath.Allocate(l2a.header.crossTrackBins, along_track_bins)) {
    fprintf(stderr, "%s: error allocating wind swath\n", command);
    exit(1);
  } 

  l2b.header.crossTrackResolution = l2a.header.crossTrackResolution;
  l2b.header.alongTrackResolution = l2a.header.alongTrackResolution;
  l2b.header.zeroIndex            = l2a.header.zeroIndex;
  
  int l2a_record_number = 0;
  while( l2a.ReadDataRec() ) {
    if( l2a_record_number % 1000 == 0 ) printf("L2A Record: %d\n",l2a_record_number);

    int       cti       = (int)l2a.frame.cti;
    int       ati       = (int)l2a.frame.ati;
    MeasList* meas_list = &(l2a.frame.measList);
    WVC*      wvc       = new WVC();
    Meas*     meas      = meas_list->GetHead();
    
    while( meas ) { 
      if( meas->landFlag == 0 )
        meas = meas_list->GetNext();
      else {
        meas = meas_list->RemoveCurrent();
        delete meas;
        meas = meas_list->GetCurrent();
      }
    }
        
    if( meas_list->NodeCount() == 0 ) {
      delete wvc;
      continue;
    }
    
    // Get nudge vector
    wvc->lonLat  = meas_list->AverageLonLat();    
    wvc->nudgeWV = new WindVectorPlus();
    
    l2a_to_l2b.nudgeField.InterpolatedWindVector( wvc->lonLat, wvc->nudgeWV );
    
    float spdout;
    float objout;
    float delta_spd = 0.1;
    float spd_start = wvc->nudgeWV->spd;
    float angle     = wvc->nudgeWV->dir * rtd;
    float prior_dir = 0;
    int   do_interp = 1;
    
    // Find best speed for nudge direction
    gmf.LineMaximize( meas_list, spd_start, angle, &kp, delta_spd, 
      do_interp, &spdout, &objout, prior_dir );

    WindVectorPlus* wvp = new WindVectorPlus();
    
    wvp->spd = spdout;
    wvp->dir = wvc->nudgeWV->dir;
    wvp->obj = objout;    
    
    if (! wvc->ambiguities.Append(wvp)) {
      delete wvp;
      fprintf( stderr, "Error adding retrieval!\n" );
      exit(1);
    }
    
    wvc->selected = wvc->ambiguities.GetHead();
        
    l2b.frame.swath.Add(cti, ati, wvc);
    l2a_record_number++;
  }
  
  if ( !l2b.WriteHeader() || !l2b.WriteDataRec() ) {
    fprintf( stderr, "Error writing to L2B file\n" );
    exit(1);
  }
  
  l2a.Close();  
  l2b.Close();  
  return 0;
}











