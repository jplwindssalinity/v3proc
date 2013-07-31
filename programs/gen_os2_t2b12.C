
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

int determine_n_scans( hid_t obj_id ) {
  hid_t sds_id = H5Dopen1(obj_id,"Scan_number");
  if( sds_id < 0 ) return(0);
  hid_t space_id = H5Dget_space(sds_id);
  hsize_t dims[2], maxdims[2];
  hsize_t n_dims = H5Sget_simple_extent_dims( space_id, &dims[0], &maxdims[0] );
  if( n_dims == 0 ) return(0);
  return(dims[0]);
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
int
main(
    int        argc,
    char*    argv[])
{
  const char*  command     = no_path(argv[0]);
  char*        hdf_file    = NULL;
  char*        out_file    = NULL;
  char*        orbele_file = NULL;
  
  int optind = 1;
  while ( (optind < argc) && (argv[optind][0]=='-') ) {
    std::string sw = argv[optind];
    
    if( sw == "-i" ) {
      hdf_file = argv[++optind];
    } else if( sw == "-o" ) {
      out_file = argv[++optind];
    } else if( sw == "-t" ) {
      orbele_file = argv[++optind];
    } else {
      fprintf(stderr,"%s: Unknow option\n", command);
      exit(1);
    }
    ++optind;
  }
  
  if( hdf_file == NULL || out_file == NULL || orbele_file == NULL ) {
    fprintf( stderr, "%s: Must specify -i <hdfile>, -o <outfile> -t orb_ele files\n", command );
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
  
  
  char n_scans_attr[5];
  if( !read_attr_h5( g_id, "L1B Actual Scans", &n_scans_attr ) ) {
    fprintf(stderr,"Error obtaining # of L1B Frames from HDF file!\n");
    exit(1);
  }    
  int     num_l1b_frames = atoi( n_scans_attr );
  hsize_t num_l1b_frames2 = determine_n_scans( g_id );
  
  if( num_l1b_frames2!=num_l1b_frames) { 
    printf("Attribute n_scans==%d; array size==%d\n",num_l1b_frames,num_l1b_frames2);
    num_l1b_frames = num_l1b_frames2;
  }  
  
  printf("n scans: %d\n",num_l1b_frames);
  

  vector< vector<uint16> > fp_lat(2);
  vector< vector<uint16> > fp_lon(2);
  vector< vector<uint16> > fp_tb(2);
  
  
  
  
  for( int i_pol = 0; i_pol < 2; ++i_pol ) {
    int n_scans  = ( i_pol == 0 ) ? 281 : 282;
    fp_lat[i_pol].resize( num_l1b_frames * n_scans );
    fp_lon[i_pol].resize( num_l1b_frames * n_scans );
    fp_tb[i_pol].resize(  num_l1b_frames * n_scans );
  }

  if( !read_SDS_h5( g_id, "Inner_beam_footprint_latitude",               &fp_lat[0][0] ) ||
      !read_SDS_h5( g_id, "Inner_beam_footprint_longitude",              &fp_lon[0][0] ) ||
      !read_SDS_h5( g_id, "Inner_beam_footprint_brightness_temperature", &fp_tb[0][0] ) ||
      !read_SDS_h5( g_id, "Outer_beam_footprint_latitude",               &fp_lat[1][0] ) ||
      !read_SDS_h5( g_id, "Outer_beam_footprint_longitude",              &fp_lon[1][0] ) ||
      !read_SDS_h5( g_id, "Outer_beam_footprint_brightness_temperature", &fp_tb[1][0] ) ) {
    fprintf(stderr,"Error loading footprint datasets from %s\n", hdf_file );
    exit(1);
  }
  H5Gclose(g_id);
  H5Fclose(h_id);

  // Compute lat, lon bounding box for each L1B frame, this greatly speeds up processing
  vector< float > min_lat(num_l1b_frames);
  vector< float > max_lat(num_l1b_frames);
  vector< float > min_lon(num_l1b_frames);
  vector< float > max_lon(num_l1b_frames);
  
  for( int i_frame = 0; i_frame < num_l1b_frames; ++i_frame ) {
    min_lat[i_frame] =  999;
    max_lat[i_frame] = -999;
    min_lon[i_frame] =  999;
    max_lon[i_frame] = -999;
    
    for( int i_scan = 0; i_scan < 282; ++i_scan ) {
      int    scan_ind = i_frame * 282 + i_scan;
      double tmp_lat  = -90+0.002757*double(fp_lat[1][scan_ind]);
      double tmp_lon  = 0+0.005515*double(fp_lon[1][scan_ind]);      
      min_lat[i_frame] = (tmp_lat<min_lat[i_frame]) ? tmp_lat : min_lat[i_frame];
      max_lat[i_frame] = (tmp_lat>max_lat[i_frame]) ? tmp_lat : max_lat[i_frame];
      min_lon[i_frame] = (tmp_lon<min_lon[i_frame]) ? tmp_lon : min_lon[i_frame];
      max_lon[i_frame] = (tmp_lon>max_lon[i_frame]) ? tmp_lon : max_lon[i_frame];
    }
  }

  vector<float> e2b_lon(3248*152);
  vector<float> e2b_lat(3248*152);
  vector<float> tb_v(3248*152);
  vector<float> tb_h(3248*152);

  // compute L2B grid locations...
  for ( int ati = 0; ati < 3248; ++ati ) {
    for (int cti = 0; cti < 152; ++cti ) {
      int e2b_ind = cti + ati*152;
      float wvc_lat, wvc_lon;
      bin_to_latlon( ati, cti, &orbit_config, &wvc_lat, &wvc_lon );
      
      e2b_lon[e2b_ind] = RAD_TO_DEG(wvc_lon);
      e2b_lat[e2b_ind] = RAD_TO_DEG(wvc_lat);
    }
  }
  
  // Average Noise brightness temperatures onto L2B grid
  double half_beam_size[2];
  
  half_beam_size[0] = sqrt( 27*45 ) / 111 / 2;
  half_beam_size[1] = sqrt( 29*68 ) / 111 / 2;
  
  int    scan_ind;
  double dlat, dlon, tmp_lat, tmp_lon;
  
  for( int ati = 0; ati < 3248; ++ati ) {
    if ( ati % 128 == 0 ) printf("ati: %d\n",ati);
    for( int cti = 0; cti < 152; ++cti ) {
      int e2b_ind = cti + ati*152;
      
      double sum_tb[2] = {0,0};
      int   cnts_tb[2] = {0,0};
      
      double wvc_lon = e2b_lon[e2b_ind];
      double wvc_lat = e2b_lat[e2b_ind];
      
      if( wvc_lon < 0 ) wvc_lon += 360;
      
      for( int i_frame = 0; i_frame < num_l1b_frames; ++i_frame ) {
        
        // Skip L1B frames where the bounding box doesn't contain the 
        // E2B grid cell (extend box by the VV half_beam_size).
        if( wvc_lat < min_lat[i_frame]-half_beam_size[1]                  ||
            wvc_lat > max_lat[i_frame]+half_beam_size[1]                  ||
            wvc_lon < min_lon[i_frame]-half_beam_size[1]/cos(wvc_lat*dtr) ||
            wvc_lon > max_lon[i_frame]+half_beam_size[1]/cos(wvc_lat*dtr) ) continue;
            
        for( int i_pol = 0; i_pol < 2; ++i_pol ) {
          int n_scans  = ( i_pol == 0 ) ? 281 : 282;
          for( int i_scan = 0; i_scan < n_scans; ++i_scan ) {
    
            scan_ind = i_frame * n_scans + i_scan;          
            tmp_lat  = -90+0.002757*double(fp_lat[i_pol][scan_ind]);
            dlat     = tmp_lat - wvc_lat;
            
            if( fabs(dlat) >= half_beam_size[i_pol] ) continue;

            tmp_lon  = 0+0.005515*double(fp_lon[i_pol][scan_ind]);
            
            dlon = tmp_lon - wvc_lon;
            
            if( fabs(dlon) >= half_beam_size[i_pol]/cos(wvc_lat*dtr) ) continue;
            
            sum_tb[i_pol]  += 0.01*double(fp_tb[i_pol][scan_ind]);
            cnts_tb[i_pol] += 1;
          } 
        }  
      }
      if( cnts_tb[0] > 0 )
        tb_h[e2b_ind] = sum_tb[0] / double(cnts_tb[0]);
      else
        tb_h[e2b_ind] = -1;

      if( cnts_tb[1] > 0 )
        tb_v[e2b_ind] = sum_tb[1] / double(cnts_tb[1]);
      else
        tb_v[e2b_ind] = -1;
    }
  }  
  
  FILE* ofp = fopen(out_file,"w");
  fwrite( &e2b_lat[0], sizeof(float), 3248*152, ofp );
  fwrite( &e2b_lon[0], sizeof(float), 3248*152, ofp );
  fwrite( &tb_v[0],    sizeof(float), 3248*152, ofp );
  fwrite( &tb_h[0],    sizeof(float), 3248*152, ofp );
  fclose(ofp);
  return(0);
}


