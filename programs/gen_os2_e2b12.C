
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
#include "QscatConfig.h"
#include "List.h"
#include "BufferedList.h"
#include "SeaPac.h"
#include "AttenMap.h"
#include "hdf5.h"

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


typedef struct {
    float lambda_0;
    float inclination;
    float rev_period;
    int xt_steps;
    double at_res;
    double xt_res;
} latlon_config;

int init_string( char* string, int length )
{
	for( int ii = 0; ii < length; ++ii )
	  string[ii] = NULL;
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


void bin_to_latlon(int at_ind, int ct_ind,
        const latlon_config *config, float *lat, float *lon) {

    /* Utilizes e2, r1_earth from Constants.h */
    const static double P1 = 60*1440.0f;

    const static double P2 = config->rev_period;
    const double inc = DEG_TO_RAD(config->inclination);
    const int    r_n_xt_bins = config->xt_steps;
    const double at_res = config->at_res;
    const double xt_res = config->xt_res;
    
    // Modified for OS2 starting at north pole
    //lambda_0 = DEG_TO_RAD(config->lambda_0);
    double lambda_0 = config->lambda_0 + 180 + P2/(2*P1) * 360;
    if( lambda_0 > 360 ) lambda_0 -= 360;
    lambda_0 = DEG_TO_RAD(lambda_0);
    
    
    const double r_n_at_bins = 1624.0 * 25.0 / at_res;
    const double atrack_bin_const = two_pi/r_n_at_bins;
    const double xtrack_bin_const = xt_res/r1_earth;

    double lambda, lambda_t, lambda_pp;
    double phi, phi_pp;
    double Q, U, V, V1, V2;

    double sin_phi_pp, sin_lambda_pp;
    double sin_phi_pp2, sin_lambda_pp2;
    double sini, cosi;

    sini = sinf(inc);
    cosi = cosf(inc);
    // Modified for OS2 starting at north pole
    //lambda_pp = (at_ind + 0.5)*atrack_bin_const - pi_over_two;
    lambda_pp = (at_ind + 0.5)*atrack_bin_const - pi_over_two + pi;
    phi_pp = -(ct_ind - (r_n_xt_bins/2 - 0.5))*xtrack_bin_const;

    sin_phi_pp = sinf(phi_pp);
    sin_phi_pp2 = sin_phi_pp*sin_phi_pp;
    sin_lambda_pp = sinf(lambda_pp);
    sin_lambda_pp2 = sin_lambda_pp*sin_lambda_pp;
    
    Q = e2*sini*sini/(1 - e2);
    U = e2*cosi*cosi/(1 - e2);

    V1 = (1 - sin_phi_pp2/(1 - e2))*cosi*sin_lambda_pp;
    V2 = (sini*sin_phi_pp*sqrtf((1 + Q*sin_lambda_pp2)*(1 - 
                    sin_phi_pp2) - U*sin_phi_pp2));

    V = (V1 - V2)/(1 - sin_phi_pp2*(1 + U));

    lambda_t = atan2f(V, cosf(lambda_pp));
    lambda = lambda_t - (P2/P1)*lambda_pp + lambda_0;

    
    lambda += (lambda < 0)       ?  two_pi : 
              (lambda >= two_pi) ? -two_pi :
                                    0.0f;
    
                                    
    phi = atanf((tanf(lambda_pp)*cosf(lambda_t) - 
                cosf(inc)*sinf(lambda_t))/((1 - e2)*
                sinf(inc)));

    *lon = lambda;
    *lat = phi;
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
  const char*  command     = no_path(argv[0]);
  char*        ecmwf_dir   = NULL;
  char*        hdf_file    = NULL;
  char*        out_file    = NULL;
  char*        orbele_file = NULL;
  int          use_bigE    = 0;
  
  int optind = 1;
  while ( (optind < argc) && (argv[optind][0]=='-') ) {
    std::string sw = argv[optind];
    
    if( sw == "-e" ) {
      ecmwf_dir = argv[++optind];
    }
    else if( sw == "-i" ) {
      hdf_file = argv[++optind];
    }
    else if( sw == "-o" ) {
      out_file = argv[++optind];
    }
    else if( sw == "-t" ) {
      orbele_file = argv[++optind];
    }
    else if( sw == "-bigE" ) {
      use_bigE = 1;
    }    
    else {
      fprintf(stderr,"%s: Unknow option\n", command);
      exit(1);
    }
    ++optind;
  }
  
  if( ecmwf_dir == NULL || hdf_file == NULL || out_file == NULL || orbele_file == NULL ) {
    fprintf( stderr, "%s: Must specify -e <ecmwf-dir>, -i <hdfile>, -o <outfile> -t orb_ele files\n", command );
    exit(1);
  }
  
  hid_t h_id = H5Fopen( hdf_file, H5F_ACC_RDONLY, H5P_DEFAULT );
  if( h_id < 0 ) {
    fprintf(stderr,"ERROR opening hdf file %s.\n",hdf_file);
    exit(1);
  }
  
  hid_t g_id = H5Gopen( h_id, "science_data", H5P_DEFAULT );
  if( g_id < 0 ) {
    fprintf(stderr,"ERROR opening /science_data group in file %s.\n",hdf_file);
    exit(1);
  }

  double attr_lon_asc_node;
  double attr_orbit_period;
  double attr_orbit_inc;
  double attr_orbit_semi_major;
  double attr_orbit_ecc;
  double t_asc_node;

  if( !read_orbit_elements_from_attr_os2( g_id, attr_lon_asc_node,
      attr_orbit_period, attr_orbit_inc, attr_orbit_semi_major,
      attr_orbit_ecc, t_asc_node) ) {
    fprintf(stderr,"Error reading orbit elements from HDF file\n");
    exit(1);
  }
  attr_orbit_period = 99.46;
  if( attr_orbit_period < 70 ) {
    fprintf(stderr,"Error: unexpected value for orbit period: %f in %s\n",
      attr_orbit_period, hdf_file); 
    exit(1);
  }
  
  latlon_config orbit_config;
  
  orbit_config.lambda_0    = attr_lon_asc_node;
  orbit_config.inclination = attr_orbit_inc;
  orbit_config.rev_period  = attr_orbit_period * 60;
  orbit_config.xt_steps    = 152;
  orbit_config.at_res      = 12.5;
  orbit_config.xt_res      = 12.5;
  
  char str_t_start[CODE_B_TIME_LENGTH];
  char str_t_end[CODE_B_TIME_LENGTH];
  char str_t_dec[CODE_B_TIME_LENGTH];
  char str_revtag[12];
  char this_revtag[12];
  
  
  if( !init_string(this_revtag,12) || 
      !read_attr_h5( g_id, "Revolution Number", this_revtag ) ) {
    fprintf( stderr, "Error reading Revolution Number\n");
    exit(1);
  }   
  
  //printf("This rev number: %s\n",this_revtag);
  
  FILE* ifp = fopen(orbele_file,"r");
  char line[1024];
  while(1) {
    fgets(line,100,ifp);
    //printf("line: %s\n",line);
    sscanf(line,"%s %s %s %s %f %f",
      str_revtag, str_t_start, str_t_end, str_t_dec,
      &orbit_config.lambda_0, &orbit_config.rev_period );
    orbit_config.rev_period *= 60;
    if( feof(ifp) || strcmp(this_revtag,str_revtag)==0 ) break;
  }  
  fclose(ifp);
  
  printf("time_start: %s\n", str_t_start );
  
  ETime  etime_start, etime_curr;
  
  etime_start.FromCodeB( str_t_start );
  
  double t_start = etime_start.GetTime();
  
  
  char ecmwf_file_1[1024],      ecmwf_file_2[1024];
  char ecmwf_file_1_last[1024], ecmwf_file_2_last[1024];
  
  WindField nwp_1, nwp_2;
  
  float ** output_lat = (float**)make_array(sizeof(float),2,3248,152);
  float ** output_lon = (float**)make_array(sizeof(float),2,3248,152);
  float ** output_spd = (float**)make_array(sizeof(float),2,3248,152);
  float ** output_dir = (float**)make_array(sizeof(float),2,3248,152);
  
  for ( int ati = 0; ati < 3248; ++ati ) {
    
    double t_curr = t_start + attr_orbit_period*double(ati)/double(3248);
    
    etime_curr.SetTime( t_curr );
    
    char code_B_str_curr[CODE_B_TIME_LENGTH];
    
    etime_curr.ToCodeB( code_B_str_curr );
    
    //printf("ati: %d, current time: %g %s\n", ati, t_curr, code_B_str_curr );
    
    int year_curr  = atoi( strtok( code_B_str_curr, "-" ) );
    int  doy_curr  = atoi( strtok( NULL,            "T" ) );
    int hour_curr  = atoi( strtok( NULL,            ":" ) );
    int  min_curr  = atoi( strtok( NULL,            ":" ) );
    float sec_curr = atof( strtok( NULL,            ":" ) );
    
    //printf( "%4.4d-%3.3dT%2.2d:%2.2d:%6.3f\n", year_curr, doy_curr, hour_curr, min_curr, sec_curr );
    
    // generate filenames
    
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

//     if( use_bigE ) { 
//       // use big endian filenames
//       sprintf( ecmwf_file_1, "%s/SNWP1%4.4d%3.3d%2.2d", ecmwf_dir, year_1, doy_1, hour_1 );
//       sprintf( ecmwf_file_2, "%s/SNWP1%4.4d%3.3d%2.2d", ecmwf_dir, year_2, doy_2, hour_2 );    
//     }
//     else {
//       // use little endian filnames
//       sprintf( ecmwf_file_1, "%s/SNWP1%4.4d%3.3d%2.2d.swap", ecmwf_dir, year_1, doy_1, hour_1 );
//       sprintf( ecmwf_file_2, "%s/SNWP1%4.4d%3.3d%2.2d.swap", ecmwf_dir, year_2, doy_2, hour_2 );
//     }

    
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
    //printf( "ati: %d ecmwf_file_1,2: %s %s\n", ati, ecmwf_file_1, ecmwf_file_2 );
     
    // Copy to _last string variables so we don't reload every time when files are the same.
    strcpy( ecmwf_file_1_last, ecmwf_file_1 );
    strcpy( ecmwf_file_2_last, ecmwf_file_2 );
    
    for (int cti = 0; cti < 152; ++cti ) {
      
      // Get lat lon for this wvc
      float wvc_lat, wvc_lon;
      bin_to_latlon( ati, cti, &orbit_config, &wvc_lat, &wvc_lon );
            
      double     wvc_lon_deg = double( RAD_TO_DEG(wvc_lon) );
      double     wvc_lat_deg = double( RAD_TO_DEG(wvc_lat) );      

      if( wvc_lon_deg < 0    ) wvc_lon_deg += 360;
      if( wvc_lon_deg >= 360 ) wvc_lon_deg -= 360;


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
      output_lat[ati][cti] = wvc_lat_deg;
      output_lon[ati][cti] = wvc_lon_deg;
      output_spd[ati][cti] = sqrt( uuu_ * uuu_ + vvv_ * vvv_ );
      output_dir[ati][cti] = RAD_TO_DEG( atan2( uuu_, vvv_ ) );
      
      //printf("ati, cti, lat, lon, spd, dir: %6d %6d %12.6f %12.6f %12.6f %12.6f\n", 
      //        ati, cti, RAD_TO_DEG(wvc_lat), RAD_TO_DEG(wvc_lon),
      //        output_spd[ati][cti], output_dir[ati][cti] );
    }
  }
  
  // write out E2B file.
  FILE* ofp = fopen( out_file, "w" );
  write_array( ofp, &output_lat[0], sizeof(float), 2, 3248, 152 );
  write_array( ofp, &output_lon[0], sizeof(float), 2, 3248, 152 );
  write_array( ofp, &output_spd[0], sizeof(float), 2, 3248, 152 );
  write_array( ofp, &output_dir[0], sizeof(float), 2, 3248, 152 ); 
  fclose(ofp);
  
  // Free the arrays.
  free_array( output_lat, 2, 3248, 152 );
  free_array( output_lon, 2, 3248, 152 );
  free_array( output_spd, 2, 3248, 152 );
  free_array( output_dir, 2, 3248, 152 );
  
  return(0);
}











