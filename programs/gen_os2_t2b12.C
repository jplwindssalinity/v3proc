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


int
main(
    int        argc,
    char*    argv[])
{
  const char*  command   = no_path(argv[0]);
  char*        e2b12file = NULL;
  char*        hdf_file  = NULL;
  char*        out_file  = NULL;
  
  int optind = 1;
  while ( (optind < argc) && (argv[optind][0]=='-') ) {
    std::string sw = argv[optind];
    
    if( sw == "-e" ) {
      ++optind;
      e2b12file = argv[optind];
    } else if ( sw == "-o" ) {
      ++optind;
      out_file = argv[optind];
    } else if ( sw == "-i" ) {
      ++optind;
      hdf_file = argv[optind];
    } else {
      fprintf(stderr,"%s: Unknown option\n", command);
      exit(1);
    }
    ++optind;
  }
  
  if( e2b12file==NULL || hdf_file==NULL || out_file==NULL ) {
    fprintf(stderr,"%s: Must specify: -i <l1bfile> -o <outfile> -e <e2b12file>\n",command);
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

  char n_scans_attr[5];
  if( !read_attr_h5( g_id, "L1B Actual Scans", &n_scans_attr ) ) {
    fprintf(stderr,"Error obtaining # of L1B Frames from HDF file!\n");
    exit(1);
  }    
  int num_l1b_frames = atoi( n_scans_attr );
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
  
  
  int e2bsize[2];
  FILE* ifp = fopen( e2b12file, "r" );  
  fseek( ifp, 0, SEEK_END );
  int file_size = ftell( ifp );

  
  if ( file_size == 7899136 ) { // has [3248][152] size arrays
    e2bsize[0] = 3248;
    e2bsize[1] = 152;
  } else if( file_size == 1974784 ) {// has [1624][76] size arrays
    e2bsize[0] = 1624;
    e2bsize[1] = 76;  
  } else {
    fprintf(stderr,"Incorrect E2B file size, %d.\n",file_size);
    exit(1);
  }
  
  printf("E2B size: %d %d\n",e2bsize[0],e2bsize[1]);
  
  vector<float> e2b_lon(e2bsize[0]*e2bsize[1]);
  vector<float> e2b_lat(e2bsize[0]*e2bsize[1]);
  vector<float> tb_v(e2bsize[0]*e2bsize[1]);
  vector<float> tb_h(e2bsize[0]*e2bsize[1]);

  fseek( ifp, 0, SEEK_SET );  
  fread( &e2b_lat[0], sizeof(float), e2bsize[0]*e2bsize[1], ifp );
  fread( &e2b_lon[0], sizeof(float), e2bsize[0]*e2bsize[1], ifp );
  fclose(ifp);
  
  double half_beam_size[2];
  
  half_beam_size[0] = sqrt( 27*45 ) / 111 / 2;
  half_beam_size[1] = sqrt( 29*68 ) / 111 / 2;
  
  int    scan_ind;
  double dlat, dlon, tmp_lat, tmp_lon;
  
  for( int ati = 0; ati < e2bsize[0]; ++ati ) {
    if ( ati % 128 == 0 ) printf("ati: %d\n",ati);
    for( int cti = 0; cti < e2bsize[1]; ++cti ) {
      int e2b_ind = cti + ati*e2bsize[1];
      
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
  fwrite( &e2b_lat[0], sizeof(float), e2bsize[0]*e2bsize[1], ofp );
  fwrite( &e2b_lon[0], sizeof(float), e2bsize[0]*e2bsize[1], ofp );
  fwrite( &tb_v[0],    sizeof(float), e2bsize[0]*e2bsize[1], ofp );
  fwrite( &tb_h[0],    sizeof(float), e2bsize[0]*e2bsize[1], ofp );
  fclose(ofp);
  return(0);
}
