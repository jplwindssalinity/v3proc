
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

int read_orbit_elements_from_attr( int32 sd_id, double &lon_asc_node,
      double &orbit_period, double &orbit_inc, double &orbit_semi_major,
      double &orbit_ecc, double &t_asc_node )
{
    char attr_string[1024];
  	char attr_asc_node_t_str[CODE_B_TIME_LENGTH];
  	
    // Read lon asc node
	if( !init_string(attr_string,1024) || 
	    !read_attr( sd_id, "EquatorCrossingLongitude", attr_string ) )
	  return(0);
	char* attr_string_trimmed = &attr_string[8];
	lon_asc_node = atof( attr_string_trimmed );
	
	// Read orbit period
	if( !init_string(attr_string,1024) || 
	    !read_attr( sd_id, "rev_orbit_period", attr_string ) )
	  return(0);
	attr_string_trimmed = &attr_string[8];
	orbit_period = atof( attr_string_trimmed );

	// Read orbit inclination
	if( !init_string(attr_string,1024) || 
	    !read_attr( sd_id, "orbit_inclination", attr_string ) )
	  return(0);
	attr_string_trimmed = &attr_string[8];
	orbit_inc = atof( attr_string_trimmed );
	
	// Read orbit semi-major-axis
	if( !init_string(attr_string,1024) || 
	    !read_attr( sd_id, "orbit_semi_major_axis", attr_string ) )
	  return(0);
	attr_string_trimmed = &attr_string[8];
	orbit_semi_major = atof( attr_string_trimmed );
	
	// Read orbit eccentricity
	if( !init_string(attr_string,1024) || 
	    !read_attr( sd_id, "orbit_eccentricity", attr_string ) )
	  return(0);
	attr_string_trimmed = &attr_string[8];
	orbit_ecc = atof( attr_string_trimmed );
	
	// Read date of ascending node
	if( !init_string(attr_string,1024) || 
	    !read_attr( sd_id, "EquatorCrossingDate", attr_string ) )
	  return(0);
	  
    // copy year+day-of-year chars to attr_asc_node_t_str
	for ( int ii = 0; ii < 8; ++ii )
	  attr_asc_node_t_str[ii] = attr_string[ii+7];

	char try__[] = "T";  
	attr_asc_node_t_str[8] = try__[0]; // Stick in the "T"
    
    // Read time of asc node.
	if( !init_string(attr_string,1024) || 
	    !read_attr( sd_id, "EquatorCrossingTime", attr_string ) )
	  return(0);
	  
	// copy time chars to string.
	for ( int ii = 0; ii < 12; ++ii )
	  attr_asc_node_t_str[ii+9] = attr_string[ii+7];

	attr_asc_node_t_str[21] = NULL; // null terminate string
    
    ETime eTime_asc_node;
    eTime_asc_node.FromCodeB( attr_asc_node_t_str );

	ETime etime;
	etime.FromCodeB("1970-001T00:00:00.000");
	double time_base = (double)etime.GetSec() + (double)etime.GetMs()/1000;
    
    t_asc_node = (double)eTime_asc_node.GetSec() 
               + (double)eTime_asc_node.GetMs()/1000 - time_base;
	
	printf("t_asc_node: %s\n",attr_asc_node_t_str);
	printf("t_asc_node: %12.6f\n",eTime_asc_node.GetTime());	
	printf("lon_asc_node:      %12.6f\n",lon_asc_node);
	printf("orbit_period:      %12.6f\n",orbit_period);
	printf("orbit_inc:         %12.6f\n",orbit_inc);
	printf("orbit_semi_major:  %12.6f\n",orbit_semi_major);
	printf("orbit_ecc:         %12.6f\n",orbit_ecc);
	
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

    const double lambda_0 = DEG_TO_RAD(config->lambda_0);

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

    lambda_pp = (at_ind + 0.5)*atrack_bin_const - pi_over_two;
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
  const char*  command   = no_path(argv[0]);
  char*        ecmwf_dir = NULL;
  char*        hdf_file  = NULL;
  char*        out_file  = NULL;
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
    else if( sw == "-bigE" ) {
      use_bigE = 1;
    }    
    else {
      fprintf(stderr,"%s: Unknow option\n", command);
      exit(1);
    }
    ++optind;
  }
  
  if( ecmwf_dir == NULL || hdf_file == NULL || out_file == NULL ) {
    fprintf( stderr, "%s: Must specify -e <ecmwf-dir>, -i <hdfile>, and -o <outfile>\n", command );
    exit(1);
  }


  int32 sd_id = SDstart(hdf_file,DFACC_READ);    
    
  if( sd_id < 0 ) {
    fprintf(stderr,"ERROR opening hdf file %s.\n",hdf_file);
    exit(1);
  }


  double attr_lon_asc_node;
  double attr_orbit_period;
  double attr_orbit_inc;
  double attr_orbit_semi_major;
  double attr_orbit_ecc;
  double t_asc_node;

  if( !read_orbit_elements_from_attr( sd_id, attr_lon_asc_node,
      attr_orbit_period, attr_orbit_inc, attr_orbit_semi_major,
      attr_orbit_ecc, t_asc_node) ) {
    fprintf(stderr,"Error reading orbit elements from HDF file\n");
    exit(1);
  }
  
  latlon_config orbit_config;
  
  orbit_config.lambda_0    = attr_lon_asc_node;
  orbit_config.inclination = attr_orbit_inc;
  orbit_config.rev_period  = attr_orbit_period;
  orbit_config.xt_steps    = 152;
  orbit_config.at_res      = 12.5;
  orbit_config.xt_res      = 12.5;
  
  char attr_string[1024];  
  char RangeBeginningDate[9];
  char RangeBeginningTime[13];

  if( !init_string(attr_string,1024) || 
	  !read_attr( sd_id, "RangeBeginningDate", attr_string ) ) {
    fprintf( stderr, "Error reading RangeBeginningDate\n");
    exit(1);
  }	  
  // copy time chars to string.
  for ( int ii = 0; ii < 9; ++ii ) 
    RangeBeginningDate[ii] = attr_string[ii+7];  
  RangeBeginningDate[8] = NULL; // null terminate string  

  
  if( !init_string(attr_string,1024) || 
	  !read_attr( sd_id, "RangeBeginningTime", attr_string ) ) {
    fprintf( stderr, "Error reading RangeBeginningTime\n");
    exit(1);
  }	  
  // copy time chars to string.
  for ( int ii = 0; ii < 13; ++ii ) 
    RangeBeginningTime[ii] = attr_string[ii+7];  
  RangeBeginningTime[12] = NULL; // null terminate string  
  
  printf( "RangeBeginningDate: %s\n", RangeBeginningDate );
  printf( "RangeBeginningTime: %s\n", RangeBeginningTime );

  if( SDend( sd_id ) ) {
    fprintf(stderr,"ERROR closing hdf file %s.\n",hdf_file);
    exit(1);
  }
  
  int  year_start = atoi( strtok( RangeBeginningDate, "-" ) );
  int   doy_start = atoi( strtok( NULL,               "-" ) );
  int  hour_start = atoi( strtok( RangeBeginningTime, ":" ) );
  int   min_start = atoi( strtok( NULL,               ":" ) );
  float sec_start = atof( strtok( NULL,               ":" ) );
   
  char code_B_range_beginning[CODE_B_TIME_LENGTH];
  sprintf( code_B_range_beginning, "%4.4d-%3.3dT%2.2d:%2.2d:%06.3f",
    year_start, doy_start, hour_start, min_start, sec_start );
  
  printf("time_start: %s\n", code_B_range_beginning );
  
  ETime  etime_start, etime_curr;
  
  etime_start.FromCodeB( code_B_range_beginning );
  
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
      
      if( doy_curr == 365 ||
          doy_curr == 366 &&
          ( year_curr % 4 == 0 &&
          ( year_curr % 100 != 0 || year_curr % 400 == 0 ) ) ) {
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











