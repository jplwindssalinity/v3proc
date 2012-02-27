//==============================================================//
// Copyright (C) 1997-2012, California Institute of Technology.    //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//        l1b_isro_to_l1b_v1.3.C
//
// SYNOPSIS
//        l1b_isro_to_l1b config_file
//
// DESCRIPTION
//        Generates l1b.dat file given a L1B HDF file from ISRO.
//      Use this program for V1.3 L1B OceanSat-II data files.
//
// OPTIONS
//        The following keywords in the config file are used:
//      required: L1B_HDF_FILE, L1B_FILE
//      optional: LANDMAP_FILE, EPHEMERIS_FILE
//
// OPERANDS
//        None.
//
// EXAMPLES
//        An example of a command line is:
//            % l1b_isro_to_l1b quikscat.rdf
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
//        Sermsak Jaruwatanadilok (jaruwata@jpl.nasa.gov) and
//        Alex Fore (alexander.fore@jpl.nasa.gov) and
//----------------------------------------------------------------------


//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
    "@(#) $Id$";

#define QS_LANDMAP_FILE_KEYWORD 		"QS_LANDMAP_FILE"
#define QS_ICEMAP_FILE_KEYWORD	 		"QS_ICEMAP_FILE"

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
#include "Tracking.C"
#include "QscatConfig.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "SeaPac.h"
#include "AttenMap.h"
/* hdf5 include */
#include "hdf5.h"

using namespace std;
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

const char* usage_array[] = { "config_file", 0 };


int read_SDS_h5( hid_t obj_id, char* sds_name, void* data_buffer )
{
    hid_t sds_id = H5Dopen1(obj_id,sds_name);
    if( sds_id < 0 ) return(0);
    
	hid_t type_id = H5Dget_type(sds_id);
	if( type_id < 0 ) return(0);
	
	if( H5Dread( sds_id, type_id, H5S_ALL, H5S_ALL,H5P_DEFAULT, data_buffer) < 0 ||
	    H5Dclose( sds_id ) < 0 ) return(0);
	
	return(1);
}

int read_attr_h5( hid_t obj_id, char* attr_name, void* data_buffer )
{
	hid_t attr_id = H5Aopen( obj_id, attr_name, H5P_DEFAULT );
	if( attr_id < 0 ) return(0);
	
	hid_t attr_type = H5Aget_type(attr_id);
	if( attr_type < 0 ) return(0);
	
	if( H5Aread( attr_id, attr_type, data_buffer ) < 0 || 
	    H5Aclose( attr_id ) < 0 ) return(0);
	  
	return(1);
}

int init_string( char* string, int length )
{
	for( int ii = 0; ii < length; ++ii )
	  string[ii] = NULL;
	return(1);
}

int read_orbit_elements_from_attr_os2( hid_t obj_id, double &lon_asc_node,
      double &orbit_period, double &orbit_inc, double &orbit_semi_major,
      double &orbit_ecc, double &t_asc_node )
{
    char attr_string[1024];
  	char attr_asc_node_t_str[CODE_B_TIME_LENGTH];
  	
	read_attr_h5( obj_id, "Equator Crossing Longitude", attr_string );
	lon_asc_node = atof( attr_string );
	
	read_attr_h5( obj_id, "Orbit Period", attr_string );
	orbit_period = atof( attr_string );
	
	read_attr_h5( obj_id, "Orbit Inclination", attr_string );
	orbit_inc = atof( attr_string );
	
	read_attr_h5( obj_id, "Orbit Semi-major Axis", attr_string );
	orbit_semi_major = atof( attr_string );
	
	read_attr_h5( obj_id, "Orbit Eccentricity", attr_string );
	orbit_ecc = atof( attr_string );
	
	read_attr_h5( obj_id, "Equator Crossing Date", attr_string );
    ETime eTime_asc_node;
    eTime_asc_node.FromCodeB( attr_string );
	
	ETime etime;
	etime.FromCodeB("1970-001T00:00:00.000");
	double time_base = (double)etime.GetSec() + (double)etime.GetMs()/1000;	
    
    t_asc_node = (double)eTime_asc_node.GetSec() 
	+ (double)eTime_asc_node.GetMs()/1000 - time_base;
	
	return(1);
} 

struct Sigma0Mapping {
  double s0_min;
  double s0_step;
  int    n_bins;
  int    start_slice[2];
  int    n_slices[2];

  vector< vector< vector< vector<float> > > > map_asc;
  //vector< vector< vector< vector<float> > > > map_dec;
};

int map_sigma0( Sigma0Mapping* sigma0_map, 
                int            i_pol, 
                int            i_scan, 
                int            i_slice,
                int            is_asc,
                double         sigma0_in, 
                double*        sigma0_out ) {

  int sliceidx = i_slice - sigma0_map->start_slice[i_pol];
  int idxmap   = (int)floor((sigma0_in-sigma0_map->s0_min)/sigma0_map->s0_step+0.5 );
  
  
  // Check in range
  if( idxmap   < 0 || idxmap   >= sigma0_map->n_bins ||
      sliceidx < 0 || sliceidx >= sigma0_map->n_slices[i_pol] ) {
    *sigma0_out = sigma0_in;
    return(0);
  }
  
  *sigma0_out = sigma0_map->map_asc[i_pol][i_scan][sliceidx][idxmap];
//  if( is_asc )
//    *sigma0_out = sigma0_map->map_asc[i_pol][i_scan][sliceidx][idxmap];
//  else
//    *sigma0_out = sigma0_map->map_dec[i_pol][i_scan][sliceidx][idxmap];
    
  return(1);
}

//--------------//
// MAIN PROGRAM //
//--------------//
 
int
main(
    int        argc,
    char*    argv[])
{
    //-----------//
    // variables //
    //-----------//
    const char*  command         = NULL;
    char*        l1b_hdf_file    = NULL;
    char*        output_file     = NULL;
    char*        ephemeris_file  = NULL;
    char*        config_file     = NULL;
    char*        attenmap_file   = NULL;
    char*        qslandmap_file  = NULL;
    char*        sigma0_map_file = NULL;
    int          do_footprint    = 0;
    
    ConfigList   config_list;
    AttenMap     attenmap;
    QSLandMap    qs_landmap;
    Kp           kp;

    command = no_path(argv[0]);

    //------------------------//
    // parse the command line //
    //------------------------//
    while ( (optind < argc) && (argv[optind][0]=='-') ) {
      std::string sw = argv[optind];
    
      if( sw == "-c" ) {
        ++optind;
        config_file = argv[optind];
      }
      else if ( sw == "-m" ) {
        ++optind;
        sigma0_map_file = argv[optind];      
      }
      else if ( sw == "-fp" )
        do_footprint = 1;
      else {
        fprintf(stderr,"%s: Unknow option\n", command);
        exit(1);
      }
      ++optind;
    }
    
    
    
    Sigma0Mapping sigma0_map;
    
    if( sigma0_map_file ) {
      printf("Using sigma0 mapping file: %s\n", sigma0_map_file );
      FILE* ifp = fopen( sigma0_map_file, "r" );
      fread( &(sigma0_map.s0_min),         sizeof(double), 1, ifp );
      fread( &(sigma0_map.s0_step),        sizeof(double), 1, ifp );
      fread( &(sigma0_map.n_bins),         sizeof(int),    1, ifp );
      fread( &(sigma0_map.start_slice[0]), sizeof(int),    2, ifp );
      fread( &(sigma0_map.n_slices[0]),    sizeof(int),    2, ifp );
      
      sigma0_map.map_asc.resize(2);
      //sigma0_map.map_dec.resize(2);

      for( int i_pol = 0; i_pol < 2; ++i_pol ) {
      
        int n_scans = ( i_pol == 0 ) ? 281 : 282;
        sigma0_map.map_asc[i_pol].resize(n_scans);
        //sigma0_map.map_dec[i_pol].resize(n_scans);

        for( int i_scan = 0; i_scan < n_scans; ++i_scan ) {
          sigma0_map.map_asc[i_pol][i_scan].resize(sigma0_map.n_slices[i_pol]);
          for( int i_slice = 0; i_slice < sigma0_map.n_slices[i_pol]; ++i_slice ) {
            sigma0_map.map_asc[i_pol][i_scan][i_slice].resize(sigma0_map.n_bins);
            fread( &(sigma0_map.map_asc[i_pol][i_scan][i_slice][0]), sizeof(float), sigma0_map.n_bins, ifp );
          }
        }

        //for( int i_scan = 0; i_scan < n_scans; ++i_scan ) {
        //  sigma0_map.map_dec[i_pol][i_scan].resize(sigma0_map.n_slices[i_pol]);
        //  for( int i_slice = 0; i_slice < sigma0_map.n_slices[i_pol]; ++i_slice ) {
        //    sigma0_map.map_dec[i_pol][i_scan][i_slice].resize(sigma0_map.n_bins);
        //    fread( &(sigma0_map.map_dec[i_pol][i_scan][i_slice][0]), sizeof(float), sigma0_map.n_bins, ifp );
        //  }
        //}
        
      }
      fclose(ifp);
    }
    
    if( config_file == NULL ) {
      fprintf(stderr,"%s: Must specify -c config_file\n", command );
      exit(1);
    }
    
    if (! config_list.Read(config_file)) {
      fprintf(stderr, "%s: error reading config file %s\n", command, config_file);
      exit(1);
    }
    
    //----------------------------------//
    // check for config file parameters //
    //----------------------------------//
    config_list.DoNothingForMissingKeywords();
        
    if (! ConfigKp(&kp, &config_list)) {
      fprintf(stderr, "%s: error configuring Kp\n", command);
      exit(1);
    }
    
    // Check for QS_LANDMAP keywords
    qslandmap_file = config_list.Get(QS_LANDMAP_FILE_KEYWORD);
    
    if( qslandmap_file == NULL ) {
      fprintf(stderr,"%s: Must specify QS_LANDMAP_FILE in config!\n",command);
      exit(1);
    }

    fprintf(stdout,"%s: Using QS LANDMAP %s\n",command,qslandmap_file);
    qs_landmap.Read( qslandmap_file );
    
    // Check for ( ATTEN_MAP_FILE, USE_ATTEN_MAP ) keywords
    int use_atten_map = 0;
    attenmap_file = config_list.Get(ATTEN_MAP_FILE_KEYWORD);
    
    if( !config_list.GetInt(USE_ATTEN_MAP_KEYWORD,&use_atten_map) ) {
      fprintf(stderr,"%s: Error reading from config file\n",command);
      exit(1);
    }

    if( use_atten_map ) {
      if( attenmap_file == NULL ) {
        fprintf(stderr,"%s: Specify attenuation filename!\n",command);
        exit(1);
      }
      fprintf(stdout,"%s: Using attenuation map: %s\n",command,attenmap_file);
      attenmap.ReadWentzAttenMap( attenmap_file );
    }
 
    // Check for L1B_HDF_FILE keyword
    l1b_hdf_file = config_list.Get(L1B_HDF_FILE_KEYWORD);
    if (l1b_hdf_file == NULL) {
        fprintf(stderr, "%s: config file must specify L1B_HDF_FILE\n", command);
        exit(1);
    }
    printf("%s: Using l1b HDF file: %s\n", command, l1b_hdf_file);
    
    // Check for L1B_FILE keyword
    output_file = config_list.Get(L1B_FILE_KEYWORD);
    if (output_file == NULL) {
        fprintf(stderr, "%s: config file must specify L1B_FILE\n", command);
        exit(1);
    }
    printf("%s: Using l1b file: %s\n", command, output_file);
    
    // Check for EPHEMERIS_FILE keyword
    ephemeris_file = config_list.Get(EPHEMERIS_FILE_KEYWORD);
    if( ephemeris_file == NULL ) {
        fprintf(stderr, "%s: config file must specify EPHEMERIS_FILE\n", command);
        exit(1);
    }    
    printf("%s: Using ephemeris file: %s\n", command, ephemeris_file);
    

    int use_kprs     = 0;
    int use_kpri     = 0;
    int do_composite = 0;
    if( !config_list.GetInt(RETRIEVE_USING_KPRS_FLAG_KEYWORD, &use_kprs) ||
        !config_list.GetInt(RETRIEVE_USING_KPRI_FLAG_KEYWORD, &use_kpri) ||
        !config_list.GetInt("USE_COMPOSITING",&do_composite) ) {
      fprintf(stderr,"%s: Error reading from config file!\n",command);
      exit(1);
    }
    
    printf("%s: Use compositing flag: %d\n", command, do_composite);

    //
    //------Done reading from config file.-------------------------------------
    //

    L1B l1b;    
    // Prepare to write
    if (l1b.OpenForWriting(output_file) == 0) {
        fprintf(stderr, "%s: cannot open l1b file %s for output\n", command, output_file);
        exit(1);
    }
	    
    hid_t h_id = H5Fopen( l1b_hdf_file, H5F_ACC_RDONLY, H5P_DEFAULT );
    if( h_id < 0 ) {
      fprintf(stderr,"ERROR opening hdf file %s.\n",l1b_hdf_file);
      exit(1);
    }
  
  	// write ephemeris contained in ISRO-OS2 L1B data	
  	
	FILE* eph_fp = fopen(ephemeris_file, "w");
	if (eph_fp == NULL) {
	  fprintf(stderr, "%s: ERROR opening ephemeris file %s\n", command, ephemeris_file);
	  exit(1);
	}    	
  	
    hid_t g_id = H5Gopen( h_id, "OAT_data", H5P_DEFAULT );
    if( g_id < 0 ) {
      fprintf(stderr,"ERROR opening /OAT_data group in file %s.\n",l1b_hdf_file);
      exit(1);
    }	

	hsize_t dimsizes[32], maxsizes[32];
	hid_t sds_id   = H5Dopen1(g_id,"Oat_record_time");
	hid_t space_id = H5Dget_space(sds_id);
	int   ndims    = H5Sget_simple_extent_dims( space_id, &dimsizes[0], &maxsizes[0] );
	H5Dclose(sds_id);
	
	int n_ephem_samples = dimsizes[1];
	//printf("ndims, dimsizes %d %d %d\n",ndims,dimsizes[0],dimsizes[1]);
    
    char ephemtime_[n_ephem_samples][22];
    read_SDS_h5(g_id,"Oat_record_time",&ephemtime_[0]);
    
    vector< float > Oat_pitch(n_ephem_samples);
    vector< float > Oat_roll(n_ephem_samples);
    vector< float > Oat_yaw(n_ephem_samples);
    vector< float > Oat_x(n_ephem_samples);
    vector< float > Oat_y(n_ephem_samples);
    vector< float > Oat_z(n_ephem_samples);
    vector< float > Oat_vx(n_ephem_samples);
    vector< float > Oat_vy(n_ephem_samples);
    vector< float > Oat_vz(n_ephem_samples);
    
    read_SDS_h5(g_id,"Oat_pitch",                         &Oat_pitch[0]);
    read_SDS_h5(g_id,"Oat_roll",                          &Oat_roll[0] );
    read_SDS_h5(g_id,"Oat_yaw",                           &Oat_yaw[0]  );
    read_SDS_h5(g_id,"Oat_satellite_position_X_component",&Oat_x[0]    );
    read_SDS_h5(g_id,"Oat_satellite_position_Y_component",&Oat_y[0]    );
    read_SDS_h5(g_id,"Oat_satellite_position_Z_component",&Oat_z[0]    );
    read_SDS_h5(g_id,"Oat_satellite_velocity_X_component",&Oat_vx[0]   );
    read_SDS_h5(g_id,"Oat_satellite_velocity_Y_component",&Oat_vy[0]   );
    read_SDS_h5(g_id,"Oat_satellite_velocity_Z_component",&Oat_vz[0]   );
	H5Gclose(g_id);
	
	for( int ii=0; ii<n_ephem_samples; ++ii ) {
	  char ephem_time_str[64];	
	  init_string( ephem_time_str, 64 );
	  strcpy(ephem_time_str,ephemtime_[ii]);
	  ETime etime;
      etime.FromCodeB("1970-001T00:00:00.000");
      double time_base = (double)etime.GetSec() + (double)etime.GetMs()/1000;
      if(!etime.FromCodeB(ephem_time_str)) {
        fprintf(stderr, "l1b_hdf_to_l1b: Error: could not parse time string: %s\n",
          ephem_time_str);
        exit(1);
      }
	  
	  double time = (double)etime.GetSec()+(double)etime.GetMs()/1000 - time_base;
	  double sc_pos[3], sc_vel[3];

	  sc_pos[0] = Oat_x[ii]*1000; sc_vel[0] = Oat_vx[ii]*1000;
	  sc_pos[1] = Oat_y[ii]*1000; sc_vel[1] = Oat_vy[ii]*1000;
	  sc_pos[2] = Oat_z[ii]*1000; sc_vel[2] = Oat_vz[ii]*1000;
      
      
      // 2-27-2012 AGF--- OAT velocity is the partial derivative of the 
      // position w.r.t. time, not the total derivative.  Need to add the 
      // term [omega z-hat] cross [sc_position] to the OAT velocity to
      // construct the total time derivative of the OAT position.
	  double omega = w_earth;  // rotation rate of earth
	  
	  sc_vel[0] -= omega * sc_pos[1];
	  sc_vel[1] += omega * sc_pos[0];
	  
	  fwrite(&time,  sizeof(double),1,eph_fp);
	  fwrite(&sc_pos,sizeof(double),3,eph_fp);
	  fwrite(&sc_vel,sizeof(double),3,eph_fp);
	  
	  //printf("ii: %d; %f\n",ii,time);
	}
	fclose(eph_fp);
	
    g_id = H5Gopen( h_id, "science_data", H5P_DEFAULT );
    if( g_id < 0 ) {
      fprintf(stderr,"ERROR opening /science_data group in file %s.\n",l1b_hdf_file);
      exit(1);
    }
    
    char n_scans_attr[5];
    if( !read_attr_h5( g_id, "L1B Actual Scans", &n_scans_attr ) ) {
      fprintf(stderr,"Error obtaining # of L1B Frames from HDF file!\n");
      exit(1);
    }
    
    int num_l1b_frames = atoi( n_scans_attr );
    printf("n scans: %d\n",num_l1b_frames);
    
    // Footprint datasets
    vector< vector<uint16> > fp_Kpa(2);
    vector< vector<uint16> > fp_Kpb(2);
    vector< vector<uint16> > fp_Kpc(2);
    vector< vector<uint16> > fp_lat(2);
    vector< vector<uint16> > fp_lon(2);
    vector< vector<uint16> > fp_snr(2);
    vector< vector<uint16> > fp_xf(2);
    vector< vector<uint16> > fp_azi(2);
    vector< vector<uint16> > fp_inc(2);
    vector< vector<uint16> > fp_s0(2);
    vector< vector<uint16> > fp_flg(2);
    
    // Slice datasets
    vector< vector<uint16> > slice_Kpa(2);
    vector< vector<uint16> > slice_Kpb(2);
    vector< vector<uint16> > slice_Kpc(2);
    vector< vector<uint16> > slice_lat(2); 
    vector< vector<uint16> > slice_lon(2);
    vector< vector<uint16> > slice_snr(2);
    vector< vector<uint16> > slice_xf(2);
    vector< vector<uint16> > slice_azi(2);
    vector< vector<uint16> > slice_inc(2);
    vector< vector<uint16> > slice_s0(2);
    vector< vector<uint16> > slice_flg(2);
    
    if( do_footprint ) { 
      // resize and load fp quantities
      for( int i_pol = 0; i_pol < 2; ++i_pol ) {
        int n_scans = ( i_pol == 0 ) ? 281 : 282;
        
        fp_Kpa[i_pol].resize( num_l1b_frames * n_scans );
        fp_Kpb[i_pol].resize( num_l1b_frames * n_scans );
        fp_Kpc[i_pol].resize( num_l1b_frames * n_scans );
        fp_lat[i_pol].resize( num_l1b_frames * n_scans );
        fp_lon[i_pol].resize( num_l1b_frames * n_scans );
        fp_snr[i_pol].resize( num_l1b_frames * n_scans );
        fp_xf[i_pol].resize(  num_l1b_frames * n_scans );
        fp_azi[i_pol].resize( num_l1b_frames * n_scans );
        fp_inc[i_pol].resize( num_l1b_frames * n_scans );
        fp_s0[i_pol].resize(  num_l1b_frames * n_scans );
        fp_flg[i_pol].resize( num_l1b_frames * n_scans );
      }
      
      if( !read_SDS_h5( g_id, "Inner_beam_footprint_Kpa",             &fp_Kpa[0][0] ) ||
          !read_SDS_h5( g_id, "Inner_beam_footprint_Kpb",             &fp_Kpb[0][0] ) ||
          !read_SDS_h5( g_id, "Inner_beam_footprint_Kpc",             &fp_Kpc[0][0] ) ||
          !read_SDS_h5( g_id, "Inner_beam_footprint_latitude",        &fp_lat[0][0] ) ||
          !read_SDS_h5( g_id, "Inner_beam_footprint_longitude",       &fp_lon[0][0] ) ||
          !read_SDS_h5( g_id, "Inner_beam_footprint_SNR",             &fp_snr[0][0] ) ||
          !read_SDS_h5( g_id, "Inner_beam_footprint_Xfactor",         &fp_xf[0][0]  ) ||
          !read_SDS_h5( g_id, "Inner_beam_footprint_azimuth_angle",   &fp_azi[0][0] ) ||
          !read_SDS_h5( g_id, "Inner_beam_footprint_incidence_angle", &fp_inc[0][0] ) ||
          !read_SDS_h5( g_id, "Inner_beam_footprint_sigma0",          &fp_s0[0][0]  ) ||
          !read_SDS_h5( g_id, "Inner_beam_footprint_sigma0_flag",     &fp_flg[0][0] ) ||
          !read_SDS_h5( g_id, "Outer_beam_footprint_Kpa",             &fp_Kpa[1][0] ) ||
          !read_SDS_h5( g_id, "Outer_beam_footprint_Kpb",             &fp_Kpb[1][0] ) ||
          !read_SDS_h5( g_id, "Outer_beam_footprint_Kpc",             &fp_Kpc[1][0] ) ||
          !read_SDS_h5( g_id, "Outer_beam_footprint_latitude",        &fp_lat[1][0] ) ||
          !read_SDS_h5( g_id, "Outer_beam_footprint_longitude",       &fp_lon[1][0] ) ||
          !read_SDS_h5( g_id, "Outer_beam_footprint_SNR",             &fp_snr[1][0] ) || 
          !read_SDS_h5( g_id, "Outer_beam_footprint_Xfactor",         &fp_xf[1][0]  ) ||
          !read_SDS_h5( g_id, "Outer_beam_footprint_azimuth_angle",   &fp_azi[1][0] ) ||
          !read_SDS_h5( g_id, "Outer_beam_footprint_incidence_angle", &fp_inc[1][0] ) || 
          !read_SDS_h5( g_id, "Outer_beam_footprint_sigma0",          &fp_s0[1][0]  ) ||
          !read_SDS_h5( g_id, "Outer_beam_footprint_sigma0_flag",     &fp_flg[1][0] ) ) {
         fprintf(stderr,"Error loading footprint datasets from %s\n", l1b_hdf_file );
         exit(1);
      }
    }
    else {  // resize and load slice datasets
      for( int i_pol = 0; i_pol < 2; ++i_pol ) {
        int n_scans  = ( i_pol == 0 ) ? 281 : 282;
        int n_slices = ( i_pol == 0 ) ?   7 :  12;
        slice_Kpa[i_pol].resize( num_l1b_frames * n_scans * n_slices );
        slice_Kpb[i_pol].resize( num_l1b_frames * n_scans * n_slices );
        slice_Kpc[i_pol].resize( num_l1b_frames * n_scans * n_slices );
        slice_lat[i_pol].resize( num_l1b_frames * n_scans * n_slices );
        slice_lon[i_pol].resize( num_l1b_frames * n_scans * n_slices );
        slice_snr[i_pol].resize( num_l1b_frames * n_scans * n_slices );
        slice_xf[i_pol].resize(  num_l1b_frames * n_scans * n_slices );
        slice_azi[i_pol].resize( num_l1b_frames * n_scans * n_slices );
        slice_inc[i_pol].resize( num_l1b_frames * n_scans * n_slices );
        slice_s0[i_pol].resize(  num_l1b_frames * n_scans * n_slices );
        slice_flg[i_pol].resize( num_l1b_frames * n_scans * n_slices );
      }
      
      if( !read_SDS_h5( g_id, "Inner_beam_slice_Kpa",             &slice_Kpa[0][0] ) ||
          !read_SDS_h5( g_id, "Inner_beam_slice_Kpb",             &slice_Kpb[0][0] ) ||
          !read_SDS_h5( g_id, "Inner_beam_slice_Kpc",             &slice_Kpc[0][0] ) ||
          !read_SDS_h5( g_id, "Inner_beam_slice_latitude",        &slice_lat[0][0] ) ||
          !read_SDS_h5( g_id, "Inner_beam_slice_longitude",       &slice_lon[0][0] ) ||
          !read_SDS_h5( g_id, "Inner_beam_slice_SNR",             &slice_snr[0][0] ) ||
          !read_SDS_h5( g_id, "Inner_beam_slice_Xfactor",         &slice_xf[0][0]  ) ||
          !read_SDS_h5( g_id, "Inner_beam_slice_azimuth_angle",   &slice_azi[0][0] ) ||
          !read_SDS_h5( g_id, "Inner_beam_slice_incidence_angle", &slice_inc[0][0] ) ||
          !read_SDS_h5( g_id, "Inner_beam_slice_sigma0",          &slice_s0[0][0]  ) ||
          !read_SDS_h5( g_id, "Inner_beam_slice_sigma0_flag",     &slice_flg[0][0] ) ||
          !read_SDS_h5( g_id, "Outer_beam_slice_Kpa",             &slice_Kpa[1][0] ) ||
          !read_SDS_h5( g_id, "Outer_beam_slice_Kpb",             &slice_Kpb[1][0] ) ||
          !read_SDS_h5( g_id, "Outer_beam_slice_Kpc",             &slice_Kpc[1][0] ) ||
          !read_SDS_h5( g_id, "Outer_beam_slice_latitude",        &slice_lat[1][0] ) ||
          !read_SDS_h5( g_id, "Outer_beam_slice_longitude",       &slice_lon[1][0] ) ||
          !read_SDS_h5( g_id, "Outer_beam_slice_SNR",             &slice_snr[1][0] ) ||
          !read_SDS_h5( g_id, "Outer_beam_slice_Xfactor",         &slice_xf[1][0]  ) ||
          !read_SDS_h5( g_id, "Outer_beam_slice_azimuth_angle",   &slice_azi[1][0] ) ||
          !read_SDS_h5( g_id, "Outer_beam_slice_incidence_angle", &slice_inc[1][0] ) ||
          !read_SDS_h5( g_id, "Outer_beam_slice_sigma0",          &slice_s0[1][0]  ) ||
          !read_SDS_h5( g_id, "Outer_beam_slice_sigma0_flag",     &slice_flg[1][0] ) ) {
        fprintf(stderr,"Error loading slice datasets from %s\n", l1b_hdf_file );
        exit(1);
      }
    }
    
	char frame_time_str[64];	
	char sst[num_l1b_frames][22];
	read_SDS_h5(g_id,"Scan_start_time",&sst[0]);
    
    Ephemeris ephem( ephemeris_file, 10000 );
    
    for( int i_frame = 0; i_frame < num_l1b_frames; ++i_frame ) {
      // Flush whatever is in the spotList object.
      l1b.frame.spotList.FreeContents();
        
      init_string( frame_time_str, 64 );
      strcpy(frame_time_str,sst[i_frame]);

      // Unix time is the epoch time for QSCATsim software.
      ETime etime;
      etime.FromCodeB("1970-001T00:00:00.000");
      double time_base = (double)etime.GetSec() + (double)etime.GetMs()/1000;
      if(!etime.FromCodeB(frame_time_str)) {
        fprintf(stderr, "l1b_hdf_to_l1b: Error: could not parse time string: %s\n",
          frame_time_str);
        exit(1);
      }
        
      double frame_time = (double)etime.GetSec() + (double)etime.GetMs()/1000 
                        - time_base;
        
      if( i_frame % 100 == 0 ) 
        printf("i_frame: %6d; frame_time: %s \n",i_frame,frame_time_str);

      // Get seconds of year for attenuation map.  Note that strtok replaces the 
      // tokens with NULL; frame_time_str is basically unusable after this point.
      int year        = atoi( strtok( &frame_time_str[0], "-"  ) );
      int doy         = atoi( strtok( NULL,               "T"  ) );
      int hour        = atoi( strtok( NULL,               ":"  ) );
      int minute      = atoi( strtok( NULL,               ":"  ) );
      float seconds   = atof( strtok( NULL,               "\0" ) );
      double sec_year = seconds+60.0*(float(minute)+60.0*(float(hour)+24.0*float(doy-1)));  

      uint16 FLAG_MASK   = uint16( 0xF0   ); // bits 4,5,6,7
      uint16 ICE_MASK    = uint16( 0xE000 ); // bits 13,14,15
      uint16 NEG_S0_MASK = uint16( 0x200  ); // bit 9
      uint16 ASC_MASK    = uint16( 0x1    ); // bit 0
      uint16 FORE_MASK   = uint16( 0x4    ); // bit 2
      
      for( int i_pol = 0; i_pol < 2; ++i_pol ) {
        int n_scans  = ( i_pol == 0 ) ? 281 : 282;
        int n_slices = ( i_pol == 0 ) ?   7 :  12;
        
        for( int i_scan = 0; i_scan < n_scans; ++i_scan ) {
          MeasSpot* new_meas_spot = new MeasSpot();
          // Set time in meas_spot and in orbit state
          new_meas_spot->time              = frame_time;
          new_meas_spot->scOrbitState.time = frame_time;        
          new_meas_spot->scAttitude.SetRPY(0.0,0.0,0.0);
          
          ephem.GetOrbitState( frame_time, EPHEMERIS_INTERP_ORDER, &new_meas_spot->scOrbitState );
        
          int scan_ind = i_frame * n_scans + i_scan;
          
          if( do_footprint ) {
            if( fp_flg[i_pol][scan_ind]&FLAG_MASK ) 
              continue;
            
            Meas* new_meas = new Meas();
            
            double xf  = -120 + 0.000613 * double(fp_xf[i_pol][scan_ind]);
            double s0  = -96  + 0.00147  * double(fp_s0[i_pol][scan_ind]);
            double snr = -65  + 0.001547 * double(fp_snr[i_pol][scan_ind]);
            
            new_meas->XK      = pow(10.0,0.1*xf);
            new_meas->EnSlice = pow(10.0,0.1*(s0+xf-snr));
            
            double tmp_lon = dtr*(  0+0.005515*double(fp_lon[i_pol][scan_ind]));
            double tmp_lat = dtr*(-90+0.002757*double(fp_lat[i_pol][scan_ind]));
            
            // make longitude, latitude to be in range.
            if( tmp_lon < 0       ) tmp_lon += two_pi;
            if( tmp_lon >= two_pi ) tmp_lon -= two_pi;
            if( tmp_lat < -pi/2   ) tmp_lat  = -pi/2;
            if( tmp_lat >  pi/2   ) tmp_lat  =  pi/2;
            
            new_meas->centroid.SetAltLonGDLat( 0.0, tmp_lon, tmp_lat );

            // Set inc angle
            new_meas->incidenceAngle = dtr*(46+0.0002451*double(fp_inc[i_pol][scan_ind]));
            
            // Get attenuation map value
            float atten_dB = 0;
            if( use_atten_map )
              atten_dB = attenmap.GetNadirAtten( tmp_lon, tmp_lat, sec_year )
                       / cos( new_meas->incidenceAngle );
            float atten_lin = pow(10.0,0.1*atten_dB);
            
            float northAzimuth       = dtr*0.005515*double(fp_azi[i_pol][scan_ind]);
            new_meas->eastAzimuth    = (450.0*dtr - northAzimuth);
            if (new_meas->eastAzimuth >= two_pi) new_meas->eastAzimuth -= two_pi;         
            
            double value = pow(10.0,0.1*s0);
            if( fp_flg[i_pol][scan_ind]&NEG_S0_MASK ) value *= -1;
            
            
            double value_mapped;
            
            int is_asc = 0;
            if( fp_flg[i_pol][scan_ind]&ASC_MASK ) is_asc = 1;
            
            
            
            // try to use sigma0 mapping, if commanded and it doesn't fail.
            if(!sigma0_map_file||!map_sigma0(&sigma0_map,i_pol,i_scan,0,is_asc,value,&value_mapped))
              value_mapped = value;
            
            int is_fore = 0;
            if( fp_flg[i_pol][scan_ind]&FORE_MASK ) is_fore = 1;            
            if( is_fore )
              new_meas->scanAngle     = 0.0;
            else
              new_meas->scanAngle     = 3.1459;
              
            new_meas->measType      = (i_pol==0) ? Meas::HH_MEAS_TYPE : Meas::VV_MEAS_TYPE;
            new_meas->beamIdx       = i_pol;
            new_meas->value         = value_mapped;
            new_meas->numSlices     = -1;
            new_meas->startSliceIdx = -1;
            if( i_pol == 0 ) {
              new_meas->azimuth_width = 27;
              new_meas->range_width   = 45;
            } else {
              new_meas->azimuth_width = 30;
              new_meas->range_width   = 68;
            }
            
            // Get Land map value
            new_meas->landFlag = 0;
            if( qs_landmap.IsLand(tmp_lon,tmp_lat,0) )  
              new_meas->landFlag += 1; // bit 0 for land
		  
		    // Test for ice flag
		    if( fp_flg[i_pol][scan_ind]&ICE_MASK )
		      new_meas->landFlag += 2; // bit 1 for ice
		      
		    double sos = pow( 10.0, 0.1*(s0-snr) );
		    
		    double kprs2 = 0.0;
		    if ( use_kprs && !kp.GetKprs2(new_meas, &kprs2)) {
		      fprintf(stderr, "%s: Error computing Kprs2\n",command);
		      exit(1);
		    }
		    
		    double kpri2 = 0.0;
		    if( use_kpri && !kp.GetKpri2( &kpri2) ) {
		      fprintf(stderr,"%s: Error computing Kpri2\n",command);
		      exit(1);
		    }

		    double kpr2     = 1 + kprs2 + kpri2;
		    
		    double kpA = 0.0000154*double(fp_Kpa[i_pol][scan_ind]);
		    double kpB = 0.0000154*double(fp_Kpb[i_pol][scan_ind]);
		    double kpC = 0.0000154*double(fp_Kpc[i_pol][scan_ind]);
		    
		    double kp_alpha = (1+kpA) * kpr2;
		    double kp_beta  =     kpB * kpr2 * sos;
		    double kp_gamma =     kpC * kpr2 * sos * sos;
		    
		    // Set kp alpha, beta, gamma and correct for attenuation
		    new_meas->A = 1 + ( kp_alpha - 1 ) * atten_lin * atten_lin;
		    new_meas->B = kp_beta              * atten_lin * atten_lin;
		    new_meas->C = kp_gamma             * atten_lin * atten_lin;           
		    
		    // Stick this meas in the measSpot
		    new_meas_spot->Append(new_meas);
          } else {
            
            for( int i_slice = 0; i_slice < n_slices; ++i_slice ) {
              
              if( i_slice >= 10 ) continue;
              
              int slice_ind = i_frame * n_scans * n_slices + i_scan * n_slices + i_slice;
              
              if( slice_flg[i_pol][slice_ind]&FLAG_MASK ) 
                continue;
              
              Meas* new_meas = new Meas();

              double xf  = -120 + 0.000613 * double(slice_xf[i_pol][slice_ind]);
              double s0  = -96  + 0.00147  * double(slice_s0[i_pol][slice_ind]);
              double snr = -65  + 0.001547 * double(slice_snr[i_pol][slice_ind]);
            
              new_meas->XK      = pow(10.0,0.1*xf);
              new_meas->EnSlice = pow(10.0,0.1*(s0+xf-snr));
            
              double tmp_lon = dtr*(  0+0.005515*double(slice_lon[i_pol][slice_ind]));
              double tmp_lat = dtr*(-90+0.002757*double(slice_lat[i_pol][slice_ind]));
            
              // make longitude, latitude to be in range.
              if( tmp_lon < 0       ) tmp_lon += two_pi;
              if( tmp_lon >= two_pi ) tmp_lon -= two_pi;
              if( tmp_lat < -pi/2   ) tmp_lat  = -pi/2;
              if( tmp_lat >  pi/2   ) tmp_lat  =  pi/2;
            
              new_meas->centroid.SetAltLonGDLat( 0.0, tmp_lon, tmp_lat );

              // Set inc angle
              new_meas->incidenceAngle = dtr*(46+0.0002451*double(slice_inc[i_pol][slice_ind]));    
              
              // Get attenuation map value
              float atten_dB = 0;
              if( use_atten_map )
                atten_dB = attenmap.GetNadirAtten( tmp_lon, tmp_lat, sec_year )
                         / cos( new_meas->incidenceAngle );
              float atten_lin = pow(10.0,0.1*atten_dB);
            
              float northAzimuth       = dtr*0.005515*double(slice_azi[i_pol][slice_ind]);
              new_meas->eastAzimuth    = (450.0*dtr - northAzimuth);
              if (new_meas->eastAzimuth >= two_pi) new_meas->eastAzimuth -= two_pi;         

              int is_fore = 0;
              if( slice_flg[i_pol][slice_ind]&FORE_MASK ) is_fore = 1;            
              if( is_fore )
                new_meas->scanAngle     = 0.0;
              else
                new_meas->scanAngle     = 3.1459;

              new_meas->measType      = (i_pol==0) ? Meas::HH_MEAS_TYPE : Meas::VV_MEAS_TYPE;
              new_meas->beamIdx       = i_pol;
              
              if( i_pol == 0 ) {
                new_meas->azimuth_width = 27.0;
                new_meas->range_width   = 45.0/7.0;
              } else {
                new_meas->azimuth_width = 30.0;
                new_meas->range_width   = 68.0/12.0;
              }
              new_meas->range_width = 7.0;
              
              // Get Land map value
              new_meas->landFlag = 0;
              if( qs_landmap.IsLand(tmp_lon,tmp_lat,1) )  
                new_meas->landFlag += 1; // bit 0 for land

              // Test for ice flag
              if( slice_flg[i_pol][slice_ind]&ICE_MASK )
		        new_meas->landFlag += 2; // bit 1 for ice
		      
              double slice_kpA = 0.0000154*double(slice_Kpa[i_pol][slice_ind]);
		      double slice_kpB = 0.0000154*double(slice_Kpb[i_pol][slice_ind]);
		      double slice_kpC = 0.0000154*double(slice_Kpc[i_pol][slice_ind]);
		      
		      double value_mapped;
		      double value  = pow(10.0,0.1*s0);
		      if( slice_flg[i_pol][slice_ind]&NEG_S0_MASK ) 
		        value *= -1;
              
              int is_asc = 0;
              if( slice_flg[i_pol][slice_ind]&ASC_MASK ) is_asc = 1;
              
              // try to use sigma0 mapping, if commanded and it doesn't fail.
              if(!sigma0_map_file||!map_sigma0(&sigma0_map,i_pol,i_scan,i_slice,is_asc,value,&value_mapped))
                value_mapped = value;
              
              //assigned mapped or unmapped value
              new_meas->value = value_mapped;
		      
		      if( do_composite ) {
		        // Set numslices == 1 to indicate software to treat A,B,C as kp_a, kp_b, kp_c.
                new_meas->numSlices     = 1;
                new_meas->startSliceIdx = i_slice;
                
                // Set A, B, C for later compositing.
                new_meas->A = slice_kpA;
                new_meas->B = slice_kpB;
                new_meas->C = slice_kpC;                
		        
		      } else {
   		        // Set numslices == -1 to indicate software to 
                // treat A,B,C as kp_alpha, kp_beta, kp_gamma.
                new_meas->numSlices     = -1;
                new_meas->startSliceIdx = i_slice;
		        new_meas->value        *= atten_lin;
		        
                double kprs2 = 0.0;
                if (! kp.GetKprs2(new_meas, &kprs2)) {
                  fprintf(stderr, "%s: Error computing Kprs2\n",command);
                  exit(1);
                }
             
                double kpri2 = 0.0;
                if( !kp.GetKpri2( &kpri2) ) {
                  fprintf(stderr,"%s: Error computing Kpri2\n",command);
                  exit(1);
                }

                // compute sigma-0 over SNR ratio.
                double sos = pow( 10.0, 0.1*(s0-snr) );
                double kpr2      = 1 + kprs2 + kpri2;
		        double kp_alpha  = (1+slice_kpA) * kpr2;
		        double kp_beta   = slice_kpB     * kpr2 * sos;
		        double kp_gamma  = slice_kpC     * kpr2 * sos * sos;

                // Set kp alpha, beta, gamma and correct for attenuation
                new_meas->A = 1 + ( kp_alpha - 1 ) * atten_lin * atten_lin;
                new_meas->B = kp_beta              * atten_lin * atten_lin;
                new_meas->C = kp_gamma             * atten_lin * atten_lin;           
		      }
              new_meas_spot->Append(new_meas);
              
            }
          }
          // Stick this measSpot in the spotList.
		  l1b.frame.spotList.Append(new_meas_spot);
        }
      }

      l1b.frame.frame_i              = i_frame;
      l1b.frame.num_l1b_frames       = num_l1b_frames;
      l1b.frame.num_pulses_per_frame = 563;
      l1b.frame.num_slices_per_pulse = -1;
      
      // Write this L1BFrame
      if( ! l1b.WriteDataRec() ) {
        fprintf(stderr, "%s: writing to %s failed.\n", command, ephemeris_file );
        return (1);
      }      
      //if( i_frame % 32 == 0 ) printf("Wrote %d frames of %d\n", i_frame, num_l1b_frames);
    }
    // Close /science_data/ group.
	
	
	//--Overwrite ephem.dat using orbital elements since the OAT data
	//--does not seem to be usable. 2012-02-14 AGF
//     Spacecraft    spacecraft;
//     SpacecraftSim spacecraft_sim;    
// 	double attr_lon_dec_node;
// 	double attr_orbit_period;
// 	double attr_orbit_inc;
// 	double attr_orbit_semi_major;
// 	double attr_orbit_ecc;
// 	double t_dec_node;
// 
// 	if( !read_orbit_elements_from_attr_os2( g_id, attr_lon_dec_node,
//       attr_orbit_period, attr_orbit_inc, attr_orbit_semi_major,
//       attr_orbit_ecc, t_dec_node) ) {
//     	fprintf(stderr,"Error reading orbit elements from HDF file\n");
//     	exit(1);
//     }
//     
//     spacecraft_sim.DefineOrbit( t_dec_node, 
//                                 attr_orbit_semi_major, 
//                                 attr_orbit_ecc, 
//                                 attr_orbit_inc,
//                                 attr_lon_dec_node, 
//                                 90.0, 
//                                 0.0 );
//                                 
//     spacecraft_sim.LocationToOrbit( attr_lon_dec_node, 0.0, -1 );
//     spacecraft_sim.Initialize( t_dec_node );
// 
// 	// Create Ephemeris file from orbit elements
//     ETime  etime_base, etime_start, etime_end;
//     double time_base,  time_start,  time_end;
//     
//     etime_base.FromCodeB("1970-001T00:00:00.000");
//     time_base = (double)etime_base.GetSec() + (double)etime_base.GetMs()/1000;
//     
//     init_string( frame_time_str, 64 );
//     strcpy(frame_time_str,sst[0]);
//     etime_start.FromCodeB(frame_time_str);
//     time_start = (double)etime_start.GetSec() 
//                + (double)etime_start.GetMs()/1000 - time_base;
//     
//     init_string( frame_time_str, 64 );
//     strcpy(frame_time_str,sst[num_l1b_frames-1]);
//     etime_end.FromCodeB(frame_time_str);
//     time_end = (double)etime_end.GetSec() 
//              + (double)etime_end.GetMs()/1000 - time_base;
// 
// 	double t_ephem = time_start - 20;
// 	
// 	FILE *ephem_fp = fopen("ephem.dat","w");
// 	while( t_ephem < time_end + 20 ) {
// 	  spacecraft_sim.UpdateOrbit( t_ephem, &spacecraft );
// 	  //ephem.GetOrbitState( t_ephem, EPHEMERIS_INTERP_ORDER, &spacecraft.orbitState );
// 	  spacecraft.orbitState.Write(ephem_fp); 	  
// 	  t_ephem += 1;
// 	}
// 	fclose(ephem_fp);	
	
	H5Gclose(g_id);
	H5Fclose(h_id);
	
    return(0);   
}
