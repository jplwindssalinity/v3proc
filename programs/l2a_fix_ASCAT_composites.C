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
#include <string>
#include <vector>
#include "Misc.h"
#include "Ephemeris.h"
#include "ConfigList.h"
#include "L1A.h"
#include "ConfigSim.h"
#include "L1AToL1B.h"
#include "Tracking.h"
#include "QscatConfig.h"
#include "List.h"
#include "BufferedList.h"
#include "SeaPac.h"
#include "AttenMap.h"

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

// config file keys we search for
#define QS_LANDMAP_FILE_KEYWORD 		"QS_LANDMAP_FILE"

int
main(
    int        argc,
    char*    argv[])
{
  //-----------//
  // variables //
  //-----------//
  const char*  command          = no_path(argv[0]);
  char*        qslandmap_file   = NULL;
  char*        config_file      = NULL;
  char*        l2a_flagged_file = NULL;
  char*        l2a_file         = NULL;
  
  L2A          l2a;
  Kp           kp;
  ConfigList   config_list;
  QSLandMap    qs_landmap;

  int optind       = 1;
  while ( (optind < argc) && (argv[optind][0]=='-') ) {
    std::string sw = argv[optind];
    
    if( sw == "-c" )
      config_file = argv[++optind];
    else if ( sw == "-o" )
      l2a_flagged_file = argv[++optind];
    else {
      fprintf(stderr,"%s: Unknow option\n", command);
      exit(1);
    }
    ++optind;
  }
  
  if( config_file == NULL || l2a_flagged_file == NULL ) {
    fprintf(stderr,"%s: Must specify config file and output l2a flagged file!\n",
            command );
    exit(1);
  }
  
  if (! config_list.Read(config_file)) {
    fprintf(stderr, "%s: error reading sim config file %s\n", command, config_file);
    exit(1);
  }
  
  l2a_file = config_list.Get(L2A_FILE_KEYWORD);
  if( l2a_file == NULL ) {
    fprintf(stderr,"%s: config file must specify l2a filename!\n",command);
    exit(1);
  }
      
  // Check for QS_LANDMAP keyword
  qslandmap_file = config_list.Get(QS_LANDMAP_FILE_KEYWORD);
    
  if( qslandmap_file == NULL  ) {
    fprintf(stderr,"%s: Must specify QS_LANDMAP_FILE in config!\n",command);
    exit(1);
  }

  fprintf(stdout,"%s: Using QS LANDMAP %s\n",command,qslandmap_file);
  qs_landmap.Read( qslandmap_file );
  
  
  if (! ConfigKp(&kp, &config_list)) {
    fprintf(stderr, "%s: error configuring Kp\n", command);
    exit(1);
  }
  
  int use_kprs     = 0;
  int use_kpri     = 0;
  int do_composite = 0;
  if( !config_list.GetInt(RETRIEVE_USING_KPRS_FLAG_KEYWORD, &use_kprs) ||
      !config_list.GetInt(RETRIEVE_USING_KPRI_FLAG_KEYWORD, &use_kpri) ||
      !config_list.GetInt("USE_COMPOSITING",&do_composite) ) {
    fprintf(stderr,"%s: Error reading from config file!\n",command);
    exit(1);
  }

  if( !do_composite ) {
    fprintf(stderr,"%s: This program only for use with composited L2A files!\n",command);
    exit(1);
  }
  l2a.OpenForReading( l2a_file );
  l2a.OpenForWriting( l2a_flagged_file );
  l2a.ReadHeader();
  l2a.WriteHeader();
  
  int recs_read = 0;
  
  // Each WVC is a l2a data record  
  while( l2a.ReadDataRec() ) {    
    recs_read++;
    
    if( recs_read % 5000 == 0 ) 
      printf("ati: %d, %d recs read.\n",l2a.frame.ati,recs_read);

    MeasList* ml               = &(l2a.frame.measList);
    Meas*     meas             = ml->GetHead();        
    int       num_ocean_sigma0 = 0;
    
    while ( meas ) {
      double alt, lon, lat;
      meas->centroid.GetAltLonGDLat( &alt, &lon, &lat );
       
      // Add in KPR
      // Attitude KPR
      double kprs2 = 0;
      if( use_kprs && !kp.GetKprs2( meas, &kprs2 ) ) {
        fprintf(stderr,"%s: Error computing kprs!\n",command);
        exit(1);
      }
      // Instrument KPR
      double kpri2 = 0;
      if( use_kpri && !kp.GetKpri2( &kpri2 ) ) {
        fprintf(stderr,"%s: Error computing kpri!\n",command);
        exit(1);
      }
       
      // root-sum-square instrument and attitude KPR
      double kpr_eu = sqrt( kprs2 + kpri2 );
       
      meas->A         = (1+kpr_eu*kpr_eu) * ( 1 + meas->A );
      meas->B         = 0;
      meas->C         = 0;
      meas->numSlices = -1;       
      
      int old_flag = meas->landFlag;
      meas->landFlag = 0; 
      if( qs_landmap.IsLand( lon, lat, 1 ) ) meas->landFlag   += 1;
      if( old_flag == 2 || old_flag == 3   ) meas->landFlag   += 2;
      if( meas->landFlag == 0              ) num_ocean_sigma0++;
      
      meas = ml->GetNext();
    }
    if( num_ocean_sigma0 > 0 ) 
      l2a.WriteDataRec();    
  }
  // Close and quit
  l2a.CloseInputFile();
  l2a.CloseOutputFile();
  return(0);
}
