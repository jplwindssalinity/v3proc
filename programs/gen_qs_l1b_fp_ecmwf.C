
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

#define RAD_TO_DEG(x) ((x)*180.0f/M_PI)
#define DEG_TO_RAD(x) ((x)*M_PI/180.0f)

int return_num_l1b_frames( int32 sd_id )
{
    int32 rank, dimsizes[32], start[32], stride[32], edge[32];
    int32 data_type, num_attrs;
    char sds_name1[64];

	int32 sds_index = SDnametoindex( sd_id, "orbit_time" );
	int32 sds_id    = SDselect( sd_id, sds_index);

    if( SDgetinfo( sds_id, sds_name1, &rank, &dimsizes[0], &data_type, &num_attrs ) )
      return(-1);
    
    SDendaccess( sds_id );
    
    return(dimsizes[0]);
}


int init_string( char* string, int length )
{
	for( int ii = 0; ii < length; ++ii )
	  string[ii] = NULL;
	return(1);
}

int read_SDS( int32 sd_id, char* sds_name, void* data_buffer )
{
	int32 sds_index = SDnametoindex( sd_id, sds_name );
	int32 sds_id    = SDselect( sd_id, sds_index);
    
    int32 rank, dimsizes[32], start[32], stride[32], edge[32];
    int32 data_type, num_attrs;
    char sds_name1[64];

    if( SDgetinfo( sds_id, sds_name1, &rank, &dimsizes[0], &data_type, &num_attrs ) )
      return(0);
    
    //printf("in * sds_index, sds_id: %d %d\n",sds_index, sds_id);
    //printf("in * sds_name: %s\n",sds_name1);
    //printf("rank:     %d\n",rank);
    //printf("dimsizes: %d %d %d\n",dimsizes[0],dimsizes[1],dimsizes[2]);
    //printf("data_type: %d\n",data_type);
    //printf("num_attrs: %d\n",num_attrs);
        
    for( int ii = 0; ii < 32; ++ii )
    {
      start[ii]  = 0;
      stride[ii] = 1;
      edge[ii]   = 0;
      if( ii <= rank ) edge[ii] = dimsizes[ii];
    }

    if( SDreaddata( sds_id, start, stride, edge, data_buffer ) )
      return(0);
    
    SDendaccess( sds_id );
    
	return(1);
}


int read_frame_time( int32 h_id, int curr_frame, char* frame_time )
{
	int32 frame_time_ref_id = VSfind(h_id, "frame_time");
	if( frame_time_ref_id == 0 )
	  return(0);
	
	int32 frame_time_id = VSattach(h_id, frame_time_ref_id, "r");
    if( frame_time_id == -1 )
      return(0);
    
	if( VSseek( frame_time_id, curr_frame ) < 0 || 
	    VSread( frame_time_id, (uint8 *)frame_time, 1, FULL_INTERLACE ) < 0 ||
	    VSdetach( frame_time_id ) )
	  return(0);

	return(1);
}

int read_attr( int32 obj_id, char* attr_name, void* data_buffer )
{
	int32 data_type, count;
	char attr_name_[80];
	int32 attr_index = SDfindattr( obj_id, attr_name );

	if( SDattrinfo( obj_id, attr_index, attr_name_, &data_type, &count ) ||
	    SDreadattr( obj_id, attr_index, data_buffer ) )
	  return(0);
	  
	return(1);
}

int lin_interp( double x1, double  x2,   // independant variable
                double f1, double  f2,   // dependant variable at x1, x2
                double x,  double* f ) { // interpolate to x, return value into f

// Sanity check for NANs and x in range [x1,x2]
  if( x1 != x1 || x2 != x2 || f1 != f1 || f2 != f2 || x != x || x < x1 || x > x2 ) {
    fprintf( stderr, "lin_interp: Inputs not in range!\n" );
    fprintf( stderr, "x1,x2,f1,f2,x: %f %f %f %f %f\n",x1,x2,f1,f2,x);
    return(0);
  }
  
  if( x1 != x2 )
    *f = f1 + (f2-f1)*(x-x1)/(x2-x1);
  else if( x1 == x2 && f1 == f2 )
    *f = f1;
  else {
    fprintf( stderr, "lin_interp: Expected f1 == f2 if x1 == x2!\n");
    return(0);
  }
  return(1);
}                  

int bilinear_interp( double x1,  double x2,
                     double y1,  double y2,
                     double f11, double f12, // dependant variable at x=x1, y=y1; x=x1, y=y2
                     double f21, double f22, // dependant variable at x=x2, y=y1; x=x2, y=y2
                     double x,   double y, double *f ) {
  
  double fx1; // dependant variable at x=x, y=y1
  double fx2; // dependant variable at x=x, y=y2
  
  if( !lin_interp( x1, x2, f11, f21, x, &fx1 ) ||
      !lin_interp( x1, x2, f12, f22, x, &fx2 ) ||
      !lin_interp( y1, y2, fx1, fx2, y, f    ) )
    return(0);
  else
    return(1);
}

int
main(
    int        argc,
    char*    argv[])
{
  const char*  command   = no_path(argv[0]);
  char*        ecmwf_dir = NULL;
  char*        hdf_file  = NULL;
  char*        out_file  = NULL;
  char*        landmap_file = NULL;
  int          use_bigE  = 0;
  
  int optind = 1;
  while ( (optind < argc) && (argv[optind][0]=='-') ) {
    std::string sw = argv[optind];
    
    if( sw == "-e" ) {
      ++optind;
      ecmwf_dir = argv[optind];
    }
    else if( sw == "-i" ) {
      ++optind;
      hdf_file = argv[optind];
    }
    else if( sw == "-o" ) {
      ++optind;
      out_file = argv[optind];
    }
    else if( sw == "-lmap" ) {
      ++optind;
      landmap_file = argv[optind];
    }
    else if( sw == "-bigE" ) {
      use_bigE = 1;
    }    
    else {
      fprintf(stderr,"%s: Unknown option\n", command);
      exit(1);
    }
    ++optind;
  }
  
  if( ecmwf_dir == NULL || hdf_file == NULL || out_file == NULL ) {
    fprintf( stderr, "%s: Must specify -e <ecmwf-dir>, -i <hdfile>, and -o <outfile>\n", command );
    exit(1);
  }
  
  QSLandMap qs_landmap;

  if( landmap_file ) 
    qs_landmap.Read( landmap_file );
  
  
  int32 sd_id;
  int32 h_id;
	
  h_id = Hopen(hdf_file, DFACC_READ, 0);
  Vstart(h_id);
    
  sd_id = SDstart(hdf_file,DFACC_READ);    
    
  if( sd_id < 0 ) {
    fprintf(stderr,"ERROR opening hdf file %s.\n",hdf_file);
    exit(1);
  }
    
  int num_l1b_frames = return_num_l1b_frames( sd_id );
  printf("num_l1b_frames: %d\n",num_l1b_frames);
    
  if( num_l1b_frames <= 0 ) {
    fprintf(stderr,"Error obtaining # of L1B Frames from HDF file!\n");
    exit(1);
  }

  
  vector<float> fp_lat;
  vector<float> fp_lon;
  vector<float> espd;
  vector<float> edir;
  vector<char>  island;
  
  int n_scans = 100;
  fp_lat.resize( num_l1b_frames * n_scans );
  fp_lon.resize( num_l1b_frames * n_scans );
  espd.resize(   num_l1b_frames * n_scans );
  edir.resize(   num_l1b_frames * n_scans );
  island.resize( num_l1b_frames * n_scans );
  
  read_SDS( sd_id, "cell_lat",   &fp_lat[0] );
  read_SDS( sd_id, "cell_lon",   &fp_lon[0] );

  char ecmwf_file_1[1024],      ecmwf_file_2[1024];
  char ecmwf_file_1_last[1024], ecmwf_file_2_last[1024];
  
  WindField nwp_1, nwp_2;  
  
  for( int i_frame = 0; i_frame < num_l1b_frames; ++i_frame ) {

    char frame_time_str[64];
    for ( int ii=0;ii<64;++ii)
        frame_time_str[ii] = NULL;
            
    if(!read_frame_time( h_id, i_frame, &frame_time_str[0] )) {
      fprintf(stderr,"error reading frame time at frame %d\n",i_frame);
      exit(1);
    }
    
    //printf("Frame time: %s\n", frame_time_str );

    int year_curr  = atoi( strtok( frame_time_str, "-" ) );
    int  doy_curr  = atoi( strtok( NULL,           "T" ) );
    int hour_curr  = atoi( strtok( NULL,           ":" ) );
    int  min_curr  = atoi( strtok( NULL,           ":" ) );
    float sec_curr = atof( strtok( NULL,           ":" ) );
    
    //printf( "%4.4d-%3.3dT%2.2d:%2.2d:%6.3f\n", year_curr, doy_curr, hour_curr, min_curr, sec_curr );

    int year_1, year_2, doy_1, doy_2, hour_1, hour_2;
    
    year_1 = year_curr;
    doy_1  = doy_curr;
    hour_1 = hour_curr;
    year_2 = year_1;
    doy_2  = doy_1;
    hour_2 = hour_1;
    
    double t_1   = -1;
    double t_2   = -1;
    double t_now = -1;
    
    // t_now is hours of day
    t_now = double( hour_curr ) + ( double( min_curr ) + double( sec_curr ) / 60.0 ) / 60.0;
    
    if( hour_curr < 6 ) {
      hour_1 = 0;
      hour_2 = 6;
      t_1    = 0.0;
      t_2    = 6.0;
    }
    else if( hour_curr < 12 ) {
      hour_1 = 6;
      hour_2 = 12;
      t_1    = 6.0;
      t_2    = 12.0;
    }
    else if( hour_curr < 18 ) {
      hour_1 = 12;
      hour_2 = 18;
      t_1    = 12.0;
      t_2    = 18.0;      
    }
    else if( hour_curr < 24 ) {
      hour_1 = 18;
      hour_2 = 0;
      t_1    = 18.0;
      t_2    = 24.0;
      
      int is_leap_year = 0;
      if( year_curr % 400 == 0 )
        is_leap_year = 1;
      else if( year_curr % 100 == 0 )
        is_leap_year = 0;
      else if( year_curr % 4 == 0 )
        is_leap_year = 1;
      else
        is_leap_year = 0;

      if( ( doy_curr == 365 && is_leap_year == 0 ) || doy_curr == 366 ) {
        doy_2  = 1;
        year_2 = year_1 + 1;
      }
      else {
        doy_2  = doy_1 + 1;
        year_2 = year_1;
      }
    }
    
    if( use_bigE ) { 
      // use big endian filenames
      sprintf( ecmwf_file_1, "%s/SNWP3%4.4d%3.3d%2.2d", ecmwf_dir, year_1, doy_1, hour_1 );
      sprintf( ecmwf_file_2, "%s/SNWP3%4.4d%3.3d%2.2d", ecmwf_dir, year_2, doy_2, hour_2 );    
    }
    else {
      // use little endian filnames
      sprintf( ecmwf_file_1, "%s/SNWP3%4.4d%3.3d%2.2d.swap", ecmwf_dir, year_1, doy_1, hour_1 );
      sprintf( ecmwf_file_2, "%s/SNWP3%4.4d%3.3d%2.2d.swap", ecmwf_dir, year_2, doy_2, hour_2 );
    }

    // Test if need to load new files.
    if( strcmp( ecmwf_file_1, ecmwf_file_1_last ) != 0 ||
        strcmp( ecmwf_file_2, ecmwf_file_2_last ) != 0 ) {
      printf("need to load new files!\n");
      
      // load files
      if( !nwp_1.ReadNCEP1( ecmwf_file_1, use_bigE ) ||
          !nwp_2.ReadNCEP1( ecmwf_file_2, use_bigE ) ) {
        fprintf( stderr, "Error loading nwp files!\n");
        exit(1);
      }
    }    

    strcpy( ecmwf_file_1_last, ecmwf_file_1 );
    strcpy( ecmwf_file_2_last, ecmwf_file_2 );
    
    for( int i_scan = 0; i_scan < n_scans; ++i_scan ) {

      int scan_ind = i_frame * n_scans + i_scan;

      double wvc_lat_deg = fp_lat[scan_ind];
      double wvc_lon_deg = fp_lon[scan_ind];
        
      if( wvc_lon_deg < 0    ) wvc_lon_deg += 360;
      if( wvc_lon_deg >= 360 ) wvc_lon_deg -= 360;
      
      double lat_rad = wvc_lat_deg * M_PI / 180;
      double lon_rad = wvc_lon_deg * M_PI / 180;
      
      island[scan_ind] = 0;
      if( landmap_file && qs_landmap.IsLand(lon_rad,lat_rad,1) )
        island[scan_ind] = 1;
      
      double     lat[2];     // time index
      double     lon[2];     // time index
      float      u[2][2][2]; // time, lon, lat index order
      float      v[2][2][2]; // time, lon, lat index order
      double     s[2][2][2]; // time, lon, lat index order
      double     uu[2];      // bilinearly interpolated u at t1,t2
      double     vv[2];      // bilinearly interpolated v at t1,t2
      double     ss[2];      // bilinearly interpolated s at t1,t2
      double     uuu;        // trilinearly interpolated u
      double     vvv;        // trilinearly interpolated v
      double     sss;        // trilinearly interpolated speed
      
      
      // Corner of the "box" for bilinear interpolation
      // Assuming nwp is posted at integer lat lon (in degrees!)
      lat[0] = floor( wvc_lat_deg );
      lat[1] =  ceil( wvc_lat_deg );

      lon[0] = floor( wvc_lon_deg );
      lon[1] =  ceil( wvc_lon_deg );

      // Get windvector objects at the 4 corners
      for( int i_lon = 0; i_lon < 2; ++i_lon ) {
        for( int i_lat = 0; i_lat < 2; ++i_lat ) {
          
          LonLat     tmp_ll;
          WindVector tmp_wv[2];
          
          tmp_ll.Set( DEG_TO_RAD( lon[i_lon] ), DEG_TO_RAD( lat[i_lat] ) );
          
          if( !nwp_1.NearestWindVector( tmp_ll, &tmp_wv[0] ) ||
              !nwp_2.NearestWindVector( tmp_ll, &tmp_wv[1] ) ) {
            fprintf( stderr, "Error getting nearest wind vectors @ corners!\n");
            exit(1);
          }
            
          if( !tmp_wv[0].GetUV( &u[0][i_lon][i_lat], &v[0][i_lon][i_lat] ) ||
              !tmp_wv[1].GetUV( &u[1][i_lon][i_lat], &v[1][i_lon][i_lat] ) ) {
            fprintf( stderr, "Error getting u,v from WindVector!\n");
            exit(1);
          }
            
          s[0][i_lon][i_lat] = tmp_wv[0].spd;
          s[1][i_lon][i_lat] = tmp_wv[1].spd;
        }
      }        
        
      // Interpolate!
      for ( int tt = 0; tt < 2; ++tt ) {
        // interpolate for u, v, and speed.
        if( !bilinear_interp( lon[0],      lon[1],      lat[0],      lat[1], 
                              u[tt][0][0], u[tt][0][1], u[tt][1][0], u[tt][1][1], 
                              wvc_lon_deg, wvc_lat_deg, &uu[tt] ) ||
            !bilinear_interp( lon[0],      lon[1],      lat[0],      lat[1], 
                              v[tt][0][0], v[tt][0][1], v[tt][1][0], v[tt][1][1], 
                              wvc_lon_deg, wvc_lat_deg, &vv[tt] ) ||
            !bilinear_interp( lon[0],      lon[1],      lat[0],      lat[1], 
                              s[tt][0][0], s[tt][0][1], s[tt][1][0], s[tt][1][1], 
                              wvc_lon_deg, wvc_lat_deg, &ss[tt] ) ) {
          fprintf( stderr, "Error in spatial (bilinear) interpolation\n");
          exit(1);
        }
      }
      // interpolate in time
      if( !lin_interp( t_1, t_2, uu[0], uu[1], t_now, &uuu ) ||
          !lin_interp( t_1, t_2, vv[0], vv[1], t_now, &vvv ) ||
          !lin_interp( t_1, t_2, ss[0], ss[1], t_now, &sss ) ) {
        fprintf( stderr, "Error in temporal interpolation\n" );
        exit(1);
      }
      
      // Final (u,v) components (as per RSD's code); RSD says this from M. Freilich.
      // BWS says to continue to do it this way...
      double uuu_ = uuu * sss / sqrt( uuu*uuu + vvv*vvv );
      double vvv_ = vvv * sss / sqrt( uuu*uuu + vvv*vvv );

      // Use clockwise from North convention for wind direction (oceanographic)
      espd[scan_ind] = sqrt( uuu_ * uuu_ + vvv_ * vvv_ );
      edir[scan_ind] = RAD_TO_DEG( atan2( uuu_, vvv_ ) );
    }
  }
  
  FILE* ofp = fopen(out_file,"w");
  
  fwrite( &num_l1b_frames, sizeof(int),                      1, ofp );
  fwrite( &n_scans,        sizeof(int),                      1, ofp );
  fwrite( &espd[0],      sizeof(float), n_scans*num_l1b_frames, ofp );
  fwrite( &edir[0],      sizeof(float), n_scans*num_l1b_frames, ofp );  
  
  if( landmap_file )
    fwrite( &island[0], sizeof(char), n_scans*num_l1b_frames, ofp );  
  
  fclose(ofp);

  if( Hclose(h_id) || SDend( sd_id ) ) {
    fprintf(stderr,"ERROR closing hdf file %s.\n",hdf_file);
    exit(1);
  }

  return(0);
}











