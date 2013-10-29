//==============================================================//
// Copyright (C) 2010, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//
//----------------------------------------------------------------------
// NAME
//        l2a_fix_ISRO_composites.C
//
// SYNOPSIS
//        l2a_fix_ISRO_composites -c ISRO.rdf -o l2a_flagged.dat [-kp]
//
//       -kp will transform the kp A, B, C to the alpha beta gamma convention
//       using the kpr attitude table from config file, the kpri value from
//       config file, and the attenuation map given in config file.
//
// DESCRIPTION
//        Converts l2a composites, adds land flags to composite centroid locations
//        rather than worst-case upgrading of slice flags to composite flags as done
//        in Meas::Composite().  Optionally it will transform the KP from the A, B, C
//        to alpha beta gamma convention using keys in the config file.
//
//        For the ISRO (re)processing task.
//
// OPTIONS
//
// OPERANDS
//        None.
//
// EXAMPLES
//        An example of a command line is:
//            % l2a_fix_ISRO_composites -c QS.rdf -o l2a_flagged.dat -kp
//
// ENVIRONMENT
//        Not environment dependent.
//
// EXIT STATUS
//        The following exit values are returned:
//        0    Program executed successfully
//        >0    Program had an error
//
// NOTES
//        None.
//
// AUTHOR
//        Alex Fore (alexander.fore@jpl.nasa.gov)
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
#include <string>
#include "Misc.h"
#include "Ephemeris.h"
#include "ConfigList.h"
#include "L1A.h"
#include "ConfigSim.h"
#include "L1AToL1B.h"
#include "Tracking.h"
#include "Tracking.C"
#include "QscatConfig.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
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
#define QS_ICEMAP_FILE_KEYWORD	 		"QS_ICEMAP_FILE"

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
  QSIceMap     qs_icemap;
   
  int transform_kp = 0;
  int do_footprint = 0;
  int optind       = 1;
  while ( (optind < argc) && (argv[optind][0]=='-') ) {
    std::string sw = argv[optind];
    
    if( sw == "-c" ) {
      config_file = argv[++optind];
    } else if ( sw == "-o" ) {
      l2a_flagged_file = argv[++optind];
    } else if ( sw == "-kp" ) {
      transform_kp = 1;
    } else if ( sw == "-fp" ) {
      do_footprint = 1;
    } else {
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
    fprintf(stderr, "%s: error reading sim config file %s\n",
           command, config_file);
    exit(1);
  }
  
  l2a_file = config_list.Get(L2A_FILE_KEYWORD);
  if( l2a_file == NULL ) {
    fprintf(stderr,"%s: config file must specify l2a filename!\n",command);
    exit(1);
  }
      
  // Check for QS_LANDMAP and QS_ICEMAP keywords
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
  
  while( l2a.ReadDataRec() ) {
    // Each WVC is a l2a data record
    
    // if all sigma0 obs are land or ice; skip output of WVC
    int num_ocean_sigma0 = 0;
  
    MeasList* ml = &(l2a.frame.measList);
    
    Meas* meas = ml->GetHead();
    
    while ( meas ) {    
      double alt, lon, lat;
      meas->centroid.GetAltLonGDLat( &alt, &lon, &lat );
      
      // Transform from the A, B, C kp convention to the alpha beta gamma
      // set numSlices == -1 to indicate.
      if( transform_kp && meas->numSlices > 0 ) {
        double sos = meas->EnSlice / meas->XK; // sos = sigma0 / SNR
        
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
        // Attenuation 
        double atten_lin = 1.0;
        if( kp.useAttenMap ) {
          double atten_dB = kp.attenmap.GetNadirAtten( lon, lat )
                          / cos( meas->incidenceAngle );
          atten_lin = pow( 10.0, atten_dB / 10.0 );
        }
        atten_lin = 1.0;
        
        // root-sum-square instrument and attitude KPR
        double kpr_eu = sqrt( kprs2 + kpri2 );
        
        // Uncomment to replicate bug in offical processor.
        // To 1st order the effect is to mutiply kpr_eu by log(10)/10 factor.
        // kpr_eu = pow( 10, kpr_eu / 10 ) - 1;
        
        double kp_alpha = (1+kpr_eu*kpr_eu) * ( 1 + meas->A );
        double kp_beta  = (1+kpr_eu*kpr_eu) * meas->B * sos;
        double kp_gamma = (1+kpr_eu*kpr_eu) * meas->C * sos * sos;
        
        kp_alpha  = 1 + ( kp_alpha - 1 ) * atten_lin * atten_lin;
        kp_beta  *= atten_lin * atten_lin;
        kp_gamma *= atten_lin * atten_lin;        
        
        meas->A = kp_alpha;
        meas->B = kp_beta;
        meas->C = kp_gamma;
        
        // Apply attenuation correction to composited sigma-0
        meas->value *= atten_lin;
        
        // Set numSlices == -1 to indicate to GMF::GetVariance()
        // that the A, B, C are the alpha beta gamma KPs.
        meas->numSlices = -1;
      }
      
      // Set landFlag on centroids of composites;
      int old_flag = meas->landFlag;
      
      // using flagging_mode == 0 for footprints, 1 for slice composites
      int flagging_mode = (!do_footprint) ? 1 : 0;
      
      meas->landFlag = 0;
      if( qs_landmap.IsLand( lon, lat, flagging_mode ) )
        meas->landFlag += 1;
      
      if( old_flag == 2 || old_flag == 3 ) 
        meas->landFlag += 2;
      
      // composite SNR
      double snr = meas->value * meas->XK / meas->EnSlice;
      
      // Skip low snr composites.
      int skip_composite = 0;
      //if( 10*log10( fabs(snr) ) < -20 )
      //  skip_composite = 1;
      
      // If any ocean sigma0 in this WVC then write out all obs
      // in WVC to L2A output file. 2/3/2011 AGF
      if( meas->landFlag == 0 )
        num_ocean_sigma0 += 1;
      
      // Remove composite if skip_composite != 0
      if( skip_composite ) {
        meas = ml->RemoveCurrent();
        delete meas;
        meas = ml->GetCurrent();
      }
      else
        meas = ml->GetNext();
    }
    
    // Write out this WVC if it contains any ocean sigma0
    if( num_ocean_sigma0 > 0 )
      l2a.WriteDataRec();
  }
  // Close and quit
  l2a.CloseInputFile();
  l2a.CloseOutputFile();
  return(0);
}


