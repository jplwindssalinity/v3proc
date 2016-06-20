//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.    //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//        l1b_hdf_to_l1b_fast.C
//
// SYNOPSIS
//        l1b_hdf_to_l1b_fast config_file
//
// DESCRIPTION
//        Generates l1b.dat file given a L1B HDF file.
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
//            % l1b_hdf_to_l1b_fast quikscat.rdf
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

#define EXTEND_EPHEM
#define START_SLICE_INDEX    -4

#define QS_LANDMAP_FILE_KEYWORD         "QS_LANDMAP_FILE"
#define QS_ICEMAP_FILE_KEYWORD          "QS_ICEMAP_FILE"
#define DO_COASTAL_PROCESSING_KEYWORD   "DO_COASTAL_PROCESSING"
#define COASTAL_METHOD_KEYWORD          "COASTAL_PROCESSING_METHOD"
#define COASTAL_DISTANCE_FILE_KEYWORD   "COASTAL_DISTANCE_FILE"
#define LCRES_ACCUM_FILE_KEYWORD        "LCRES_ACCUM_FILE"
#define LCRES_MAP_TILE_DIR_KEYWORD      "LCRES_MAP_TILE_DIR"
#define LCR_THRESHOLD_FLAG_KEYWORD      "LCR_THRESHOLD_FLAG"
#define LCRES_THRESHOLD_FLAG_KEYWORD    "LCRES_THRESHOLD_FLAG"
#define LCRES_THRESHOLD_CORR_KEYWORD    "LCRES_THRESHOLD_CORR"

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <fstream>
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
#include "CoastDistance.h"
#include "LCRESMap.h"

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

int read_SDS( int32 sd_id, const char* sds_name, void* data_buffer )
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

int read_frame_time( int32 h_id, int curr_frame, const char* frame_time )
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

int read_attr( int32 obj_id, const char* attr_name, void* data_buffer )
{
	int32 data_type, count;
	char attr_name_[80];
	int32 attr_index = SDfindattr( obj_id, attr_name );

	if( SDattrinfo( obj_id, attr_index, attr_name_, &data_type, &count ) ||
	    SDreadattr( obj_id, attr_index, data_buffer ) )
	  return(0);
	  
	return(1);
}

int init_string( char* string, int length )
{
	for( int ii = 0; ii < length; ++ii )
	  string[ii] = NULL;
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
	
//	printf("t_asc_node: %s\n",attr_asc_node_t_str);
//	printf("t_asc_node: %12.6f\n",eTime_asc_node.GetTime());	
//	printf("lon_asc_node:      %12.6f\n",lon_asc_node);
//	printf("orbit_period:      %12.6f\n",orbit_period);
//	printf("orbit_inc:         %12.6f\n",orbit_inc);
//	printf("orbit_semi_major:  %12.6f\n",orbit_semi_major);
//	printf("orbit_ecc:         %12.6f\n",orbit_ecc);
	
	return(1);
}

//------------------------------------------------------------//
// Routine for extrapolating the ends of an ephemeris file    //
//------------------------------------------------------------//
#ifdef EXTEND_EPHEM
void ExtendEphemerisEnds(char* infile, char* outfile, float timestep, 
    int nsteps, char *output_file, int32 sd_id, const char *command){
    //----------------------------//
    // open the input ephem file //
    //---------------------------//
	
	double attr_lon_asc_node;
	double attr_orbit_period;
	double attr_orbit_inc;
	double attr_orbit_semi_major;
	double attr_orbit_ecc;
	double t_asc_node;

	if( !read_orbit_elements_from_attr( sd_id, attr_lon_asc_node,
      attr_orbit_period, attr_orbit_inc, attr_orbit_semi_major,
      attr_orbit_ecc, t_asc_node) )
    {
    	fprintf(stderr,"Error reading orbit elements from HDF file\n");
    	exit(1);
    }

    FILE* ephem_fp = fopen(infile,"r");
    if (ephem_fp == NULL)
    {
        fprintf(stderr, "%s: error opening ephem file %s\n", command,
            infile);
        exit(1);
    }

    //------------------//
    // open output file //
    //------------------//

    FILE* output_fp = fopen(outfile, "w");
    if (output_fp == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            output_file);
        exit(1);
    }

    double        sc_pos[3], sc_vel[3];
    double        nodal_period;
    double        arg_lat;
    double        long_asc_node;
    double        orb_inclination;
    double        orb_smaj_axis;
    double        orb_eccen;
    double        arg_per;
    double        mean_anom;
    Spacecraft    spacecraft;
    SpacecraftSim spacecraft_sim;    
    OrbitState    os;
    int           first_time = 1;
    
    while (os.Read(ephem_fp))
    {
        if (first_time) 
        {            
            sc_pos[0] = os.rsat.GetX()*1000;
            sc_pos[1] = os.rsat.GetY()*1000;
            sc_pos[2] = os.rsat.GetZ()*1000;  
           
            sc_vel[0] = os.vsat.GetX()*1000;
            sc_vel[1] = os.vsat.GetY()*1000;
            sc_vel[2] = os.vsat.GetZ()*1000;
            
            if( !compute_orbit_elements( sc_pos[0], sc_pos[1], sc_pos[2],
                                         sc_vel[0], sc_vel[1], sc_vel[2],
                                         &nodal_period,   &arg_lat,        &long_asc_node,
                                         &orb_inclination,&orb_smaj_axis,  &orb_eccen,
                                         &arg_per, &mean_anom ) )
              fprintf( stderr, "ERROR in compute_orbit_elements!\n");
//             printf("At 1st ehem point: %f\n",os.time);
//             printf("SC position [m]:   %12.6f %12.6f %12.6f\n",
//                    sc_pos[0], sc_pos[1], sc_pos[2] );
//             printf("SC velocity [m/s]: %12.6f %12.6f %12.6f\n",
//                    sc_vel[0], sc_vel[1], sc_vel[2] );
//             printf("Orbit Elements:\n");
//             printf("nodal_period:    %12.6f\n",nodal_period);
//             printf("arg_lat:         %12.6f\n",arg_lat);
//             printf("long_asc_node:   %12.6f\n",long_asc_node);
//             printf("orb_inclination: %12.6f\n",orb_inclination);
//             printf("orb_smaj_axis:   %12.6f\n",orb_smaj_axis);
//             printf("orb_eccen:       %12.6f\n",orb_eccen);
//             printf("arg_per:         %12.6f\n",arg_per);
//             printf("mean_anom:       %12.6f\n",mean_anom);
            
            double tmp = sqrt(xmu/pow(attr_orbit_semi_major*1e-3,3));
            double delta_t = t_asc_node-os.time;
            double mean_anom_at_asc_node = mean_anom + tmp * delta_t * rtd;
            
            while( mean_anom_at_asc_node > 360 ) mean_anom_at_asc_node -= 360;
            while( mean_anom_at_asc_node < 0   ) mean_anom_at_asc_node += 360;
            
            //printf("mean anom: %12.6f %12.6f\n",mean_anom,mean_anom_at_asc_node);
            //printf("sqrt(xmu/a^3): %12.6f\n",tmp);
            //printf("delta_t: %12.6f\n",delta_t);
            //printf("t_asc_node, os.time: %12.6f %12.6f\n",t_asc_node, os.time);
            
            spacecraft_sim.DefineOrbit( t_asc_node, attr_orbit_semi_major*1e-3, 
                                        attr_orbit_ecc, attr_orbit_inc,
                                        attr_lon_asc_node, arg_per, 
                                        mean_anom_at_asc_node );
            
            spacecraft_sim.LocationToOrbit( attr_lon_asc_node, 0.0, 1 );            
            spacecraft_sim.Initialize( t_asc_node );
            
            for(int c=-nsteps;c<0;c++){
                double curr_time_sec=os.time+c*timestep;                
                spacecraft_sim.UpdateOrbit( curr_time_sec, &spacecraft );        
                spacecraft.orbitState.Write(output_fp);                
            }
            first_time = 0;
        }
        os.Write(output_fp);
    }
    fclose(ephem_fp);

    for(int c=1;c<=nsteps;c++){
	    double curr_time_sec=os.time+c*timestep;                
        spacecraft_sim.UpdateOrbit( curr_time_sec, &spacecraft );        
        spacecraft.orbitState.Write(output_fp);       
    }
    fclose(output_fp);

}
#endif


// reads KPC B and C from the HDF attributes.
// They are indexed by the sigma0 mode flags that give the 
// gatewidth and by the beam index.
int read_kp_BC_from_attr( int32 sd_id, double* kp_B, double* kp_C ) 
{
  char kpc_B_attr_string[1024];
  char kpc_C_attr_string[1024];
  
  // Read KPC B and KPC C
  if( !init_string( kpc_B_attr_string,1024)                   || 
      !init_string( kpc_C_attr_string,1024)                   ||
      !read_attr(   sd_id, "slice_kpc_b", kpc_B_attr_string ) ||
      !read_attr(   sd_id, "slice_kpc_c", kpc_C_attr_string ) ) {
    fprintf(stderr,"In read_kp_BC_from_attr, Error reading HDF attributes!\n");
    exit(1);
  }
  //printf("kpc_B_attr_string:\n%s\n",kpc_B_attr_string);
  //printf("kpc_C_attr_string:\n%s\n",kpc_C_attr_string);
  
  char* kpc_B_pos_start = strtok( kpc_B_attr_string, "\n" );
  
  int size_1 = atoi( strtok( NULL, ","  ) );
  int size_2 = atoi( strtok( NULL, "\n" ) );
  
  //printf("size_1, size_2: %d %d\n", size_1, size_2 );
  for ( int ii = 0; ii < size_1; ++ii ) {
    for ( int jj = 0; jj < size_2; ++jj ) {
      kp_B[ii*2+jj] = atof( strtok( NULL, "\n" ) );
      //printf( "kpc_B[%d*2+%d] = %g\n", ii, jj, kp_B[ii*2+jj] );
    }
  }
  
  char* kpc_C_pos_start = strtok( kpc_C_attr_string, "\n" );
  
  size_1 = atoi( strtok( NULL, "," ) );
  size_2 = atoi( strtok( NULL, "\n" ) );

  //printf("size_1, size_2: %d %d\n", size_1, size_2 );
  for ( int ii = 0; ii < size_1; ++ii ) {
    for ( int jj = 0; jj < size_2; ++jj ) {
      kp_C[ii*2+jj] = atof( strtok( NULL, "\n" ) );
      //printf( "kpc_C[%d*2+%d] = %g\n", ii, jj, kp_C[ii*2+jj] );
    }
  }
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
    char*        kprtable_file   = NULL;
    char*        attenmap_file   = NULL;
    char*        qslandmap_file  = NULL;
    char*        qsicemap_file   = NULL;
    
    ConfigList   config_list;
    AttenMap     attenmap;
    QSLandMap    qs_landmap;
    QSIceMap     qs_icemap;
    Kp           kp;
    LandMap      lmap;
    Antenna      antenna;

    enum CoastalProcMethod {NONE, LCR, LCRES_FLAG, LCRES_CORR, LCRES_ACCUM};

    //QSKpr        qs_kpr;

    //------------------------//
    // parse the command line //
    //------------------------//

    command = no_path(argv[0]);
    
    while ( (optind < argc) && (argv[optind][0]=='-') ) {
      std::string sw = argv[optind];
      if( sw == "-c" ) {
        ++optind;
        config_file = argv[optind];
      } else {
        fprintf(stderr,"%s: Unknow option\n", command);
        exit(1);
      }
      ++optind;
    }
    
    if( config_file==NULL ) {
      fprintf(stderr,"Usage: %s -c config_file\n",command);
      exit(1);
    }
    
    if (! config_list.Read(config_file)) {
        fprintf(stderr, "%s: error reading config file %s\n",
            command, config_file);
        exit(1);
    }
    
    //----------------------------------//
    // check for config file parameters //
    //----------------------------------//
    config_list.ExitForMissingKeywords();
    ConfigKp(&kp, &config_list);

    // Check for QS_LANDMAP and QS_ICEMAP keywords
    qslandmap_file = config_list.Get(QS_LANDMAP_FILE_KEYWORD);
    qsicemap_file  = config_list.Get(QS_ICEMAP_FILE_KEYWORD);

    int do_coastal_processing = 0;
    CoastalProcMethod coastal_method = NONE;
    float land_frac_threshold = 1;
    float lcres_thresh_flag = 1;
    float lcres_thresh_corr = 1;

    // Declare these as null pointers and new them if commanded to in config
    // file.  They consume large amounts of RAM
    CoastDistance* coast_dist = NULL;
    LCRESMap* lcres_accum = NULL;
    LCRESMapTileList* lcres_map_tiles = NULL;

    config_list.DoNothingForMissingKeywords();
    config_list.GetInt(DO_COASTAL_PROCESSING_KEYWORD,&do_coastal_processing);

    if(do_coastal_processing) {
        config_list.ExitForMissingKeywords();

        // Determine which method to use
        const char* method = config_list.Get(COASTAL_METHOD_KEYWORD);
        fprintf(stdout, "%s\n", method);

        // Land Contamination Ratio method (simple land fraction threshold).
        if (strcmp(method, "LCR") == 0){
            coastal_method = LCR;

            coast_dist = new CoastDistance();
            coast_dist->Read(config_list.Get(COASTAL_DISTANCE_FILE_KEYWORD));

            config_list.GetFloat(
                LCR_THRESHOLD_FLAG_KEYWORD, &land_frac_threshold);

        // Land Contamination Ratio Expected Sigma0 threshold
        } else if (strcmp(method, "LCRES_FLAG") == 0) {
            coastal_method = LCRES_FLAG;

            char* lcres_map_tile_dir = config_list.Get(
                LCRES_MAP_TILE_DIR_KEYWORD);

            lcres_map_tiles = new LCRESMapTileList(lcres_map_tile_dir);

            coast_dist = new CoastDistance();
            coast_dist->Read(config_list.Get(COASTAL_DISTANCE_FILE_KEYWORD));

            config_list.GetFloat(
                LCRES_THRESHOLD_FLAG_KEYWORD, &lcres_thresh_flag);

        // Land Contamination Ratio Expected Sigma0 correction
        } else if (strcmp(method, "LCRES_CORR") == 0) {
            coastal_method = LCRES_CORR;

            char* lcres_map_tile_dir = config_list.Get(
                LCRES_MAP_TILE_DIR_KEYWORD);

            lcres_map_tiles = new LCRESMapTileList(lcres_map_tile_dir);

            coast_dist = new CoastDistance();
            coast_dist->Read(config_list.Get(COASTAL_DISTANCE_FILE_KEYWORD));

            config_list.GetFloat(
                LCRES_THRESHOLD_CORR_KEYWORD, &lcres_thresh_corr);

        // Mode to accumulate the LCRES maps
        } else if (strcmp(method, "LCRES_ACCUM") == 0) {
            coastal_method = LCRES_ACCUM;

            lcres_accum = new LCRESMap();
            if(!lcres_accum->Read(config_list.Get(LCRES_ACCUM_FILE_KEYWORD))) {
                fprintf(stderr, "Error reading lcres accum file\n");
            }

            coast_dist = new CoastDistance();
            coast_dist->Read(config_list.Get(COASTAL_DISTANCE_FILE_KEYWORD));
        }

        config_list.DoNothingForMissingKeywords();
    }

    fprintf(stdout,"%s: Using QS LANDMAP %s\n",command,qslandmap_file);
    fprintf(stdout,"%s: Using QS ICEMAP %s\n",command,qsicemap_file);
    qs_landmap.Read( qslandmap_file );
    qs_icemap.Read( qsicemap_file );      
    
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
    
    
    int do_composite = 0;
    if( !config_list.GetInt("USE_COMPOSITING",&do_composite) ) {
      fprintf(stderr,"%s: Error reading from config file\n",command);
      exit(1);
    }
    printf("%s: Use compositing flag: %d\n", command, do_composite);
    
    // Check for Coastal maps & configure landmap if commanded
    if(coastal_method != NONE) {
      if(!ConfigLandMap(&lmap,&config_list)) {
        fprintf(stderr,"%s: Error configuring LandMap\n",command);
        exit(1);
      }
      if(!ConfigAntenna(&antenna,&config_list)) {
        fprintf(stderr,"%s: Error configuring Antenna\n",command);
        exit(1);
      }
    }

    //
    //------Done reading from config file.-------------------------------------
    //

    L1B l1b;
    // Prepare to write
    if (l1b.OpenForWriting(output_file) == 0) {
        fprintf(stderr, "%s: cannot open l1b file %s for output\n", command, output_file);
        exit(1);
    }
	
	// open ephem file for writing.
	#ifdef EXTEND_EPHEM
	FILE* eph_fp = fopen("l1bhdf_to_l1b_tmpfile", "w");
	#else
	FILE* eph_fp = fopen(ephemeris_file, "w");
	#endif
	if (eph_fp == NULL) {
	  fprintf(stderr, "%s: ERROR opening ephemeris file %s\n", command,
              ephemeris_file);
	  exit(1);
	}  
    
    int32 sd_id;
	int32 h_id;
	
	h_id = Hopen(l1b_hdf_file, DFACC_READ, 0);
	Vstart(h_id);
    
    sd_id = SDstart(l1b_hdf_file,DFACC_READ);    
    
    if( sd_id < 0 ) {
      fprintf(stderr,"ERROR opening hdf file %s.\n",l1b_hdf_file);
      exit(1);
    }
    
    int num_l1b_frames = return_num_l1b_frames( sd_id );
    printf("num_l1b_frames: %d\n",num_l1b_frames);
    
    if( num_l1b_frames <= 0 ) {
      fprintf(stderr,"Error obtaining # of L1B Frames from HDF file!\n");
      exit(1);
    }
    
    // Arrays for holding the L1B datasets from HDF file.
    float* x_pos = (float*)calloc(sizeof(float),num_l1b_frames);
    float* y_pos = (float*)calloc(sizeof(float),num_l1b_frames);
    float* z_pos = (float*)calloc(sizeof(float),num_l1b_frames);
    float* x_vel = (float*)calloc(sizeof(float),num_l1b_frames);
    float* y_vel = (float*)calloc(sizeof(float),num_l1b_frames);
    float* z_vel = (float*)calloc(sizeof(float),num_l1b_frames);
    int16* roll  = (int16*)calloc(sizeof(int16),num_l1b_frames);
    int16* pitch = (int16*)calloc(sizeof(int16),num_l1b_frames);
    int16* yaw   = (int16*)calloc(sizeof(int16),num_l1b_frames);
    
    uint16* antenna_azimuth = (uint16*)calloc(sizeof(uint16),num_l1b_frames*100);
    
    float* cell_lat = (float*)calloc(sizeof(float),num_l1b_frames*100);
    float* cell_lon = (float*)calloc(sizeof(float),num_l1b_frames*100);
    
    int16* frequency_shift = (int16*)calloc(sizeof(int16),num_l1b_frames*100);
    
    int16* slice_lat       = (int16*)calloc(sizeof(int16),num_l1b_frames*100*8);
    int16* slice_lon       = (int16*)calloc(sizeof(int16),num_l1b_frames*100*8);
    int16* slice_sigma0    = (int16*)calloc(sizeof(int16),num_l1b_frames*100*8);
    int16* x_factor        = (int16*)calloc(sizeof(int16),num_l1b_frames*100*8);
    uint16* slice_azimuth  = (uint16*)calloc(sizeof(uint16),num_l1b_frames*100*8);
    int16* slice_incidence = (int16*)calloc(sizeof(int16),num_l1b_frames*100*8);
    int16* slice_snr       = (int16*)calloc(sizeof(int16),num_l1b_frames*100*8);
    int16* slice_kpc_a     = (int16*)calloc(sizeof(int16),num_l1b_frames*100*8);
    
    uint16* sigma0_mode_flag      = (uint16*)calloc(sizeof(uint16),num_l1b_frames*100);
    uint16* sigma0_qual_flag      = (uint16*)calloc(sizeof(uint16),num_l1b_frames*100);
    unsigned int* slice_qual_flag = (unsigned int*)calloc(sizeof(unsigned int),num_l1b_frames*100);
    
    char         frame_time_str[64];

    if( !read_SDS( sd_id, "x_pos", &x_pos[0] ) || 
        !read_SDS( sd_id, "y_pos", &y_pos[0] ) ||
        !read_SDS( sd_id, "z_pos", &z_pos[0] ) ||
        !read_SDS( sd_id, "x_vel", &x_vel[0] ) || 
        !read_SDS( sd_id, "y_vel", &y_vel[0] ) ||
        !read_SDS( sd_id, "z_vel", &z_vel[0] ) ||
        !read_SDS( sd_id, "roll",  &roll[0]  ) || // uint16
        !read_SDS( sd_id, "pitch", &pitch[0] ) || // uint16
        !read_SDS( sd_id, "yaw",   &yaw[0]   ) )  // uint16
    {
      fprintf(stderr,"Error reading ephemeris from HDF file!\n");
      exit(1);
    }
    
    if( !read_SDS( sd_id, "cell_lat",        &cell_lat[0]        ) ||
        !read_SDS( sd_id, "cell_lon",        &cell_lon[0]        ) ||
        !read_SDS( sd_id, "antenna_azimuth", &antenna_azimuth[0] ) ||
        !read_SDS( sd_id, "frequency_shift", &frequency_shift[0] ) )  // uint16
    {
      fprintf(stderr,"Error reading geolocation from HDF file!\n");
      exit(1);
    }     
    
    if( !read_SDS( sd_id, "slice_sigma0",     &slice_sigma0[0]    )  ||
        !read_SDS( sd_id, "x_factor",         &x_factor[0]        )  ||
        !read_SDS( sd_id, "slice_azimuth",    &slice_azimuth[0]   )  ||
        !read_SDS( sd_id, "slice_incidence",  &slice_incidence[0] )  ||
        !read_SDS( sd_id, "slice_snr",        &slice_snr[0]       )  ||
        !read_SDS( sd_id, "slice_kpc_a",      &slice_kpc_a[0]     )  ||
        !read_SDS( sd_id, "slice_lat",        &slice_lat[0]       )  || // uint16
        !read_SDS( sd_id, "slice_lon",        &slice_lon[0]       )  || // uint16        
        !read_SDS( sd_id, "slice_qual_flag",  &slice_qual_flag[0] ) )
    {
      fprintf(stderr,"Error reading slice quantities from HDF file!\n");
      exit(1);
    }  
    
    if( !read_SDS( sd_id, "sigma0_mode_flag", &sigma0_mode_flag[0] ) ||
        !read_SDS( sd_id, "sigma0_qual_flag", &sigma0_qual_flag[0] ) )
    {
      fprintf(stderr,"Error reading egg sigma0 flags from HDF file!\n");
      exit(1);
    }  

    // read kpB kpC from HDF attributes
    double kp_B[8*2], kp_C[8*2];
    if( !read_kp_BC_from_attr( sd_id, &kp_B[0], &kp_C[0] ) ) {
      fprintf(stderr,"Error readng KpB, kpC from HDF file!\n");
      exit(1);
    }
    
    for( int i_frame = 0; i_frame < num_l1b_frames; ++i_frame ) {
      // Read Frame times (put null in all chars since VSread won't null terminate
      // the chars that it reads).
      for ( int ii=0;ii<64;++ii)
        frame_time_str[ii] = NULL;
      
      if(!read_frame_time( h_id, i_frame, &frame_time_str[0] )) {
        fprintf(stderr,"error reading frame time at frame %d\n",i_frame);
        exit(1);
      }

      // Flush whatever is in the spotList object.
      l1b.frame.spotList.FreeContents();
      
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

      //printf("i_frame: %6d; frame_time: %s \n",i_frame,frame_time_str);
      
      // Get seconds of year for attenuation map.  Note that strtok replaces the 
      // tokens with NULL; frame_time_str is basically unusable after this point.
      int year      = atoi( strtok( &frame_time_str[0], "-"  ) );
      int doy       = atoi( strtok( NULL,               "T"  ) );
      int hour      = atoi( strtok( NULL,               ":"  ) );
      int minute    = atoi( strtok( NULL,               ":"  ) );
      float seconds = atof( strtok( NULL,               "\0" ) );
      
      double sec_year = seconds+60.0*(float(minute)+60.0*(float(hour)+24.0*float(doy - 1)));
      
      //printf("Frame time: %4.4d-%3.3d-%2.2d:%2.2d:%2.6f\n", year, doy, hour, minute, seconds );
      
      for( int i_pulse = 0; i_pulse < 100; ++i_pulse ) {
        int pulse_ind = i_frame * 100 + i_pulse;
        
        MeasSpot* new_meas_spot = new MeasSpot();
        
        // Set time in meas_spot and in orbit state
        new_meas_spot->time              = frame_time;
        new_meas_spot->scOrbitState.time = frame_time;
        
        double this_pos[3], this_vel[3], this_yaw, this_pitch, this_roll;
        double factor = (double)i_pulse/(double)100.0;

        // other frame; interpolate if not last frame, else extrapolate.
        int o_frame = i_frame + 1;
        if(i_frame==num_l1b_frames-1) {
            o_frame = i_frame - 1;
            factor *= -1;
        }

        this_pos[0] = x_pos[i_frame] + factor*(x_pos[o_frame]-x_pos[i_frame]);
        this_pos[1] = y_pos[i_frame] + factor*(y_pos[o_frame]-y_pos[i_frame]);
        this_pos[2] = z_pos[i_frame] + factor*(z_pos[o_frame]-z_pos[i_frame]);

        this_vel[0] = x_vel[i_frame] + factor*(x_vel[o_frame]-x_vel[i_frame]);
        this_vel[1] = y_vel[i_frame] + factor*(y_vel[o_frame]-y_vel[i_frame]);
        this_vel[2] = z_vel[i_frame] + factor*(z_vel[o_frame]-z_vel[i_frame]);

        this_yaw = 1e-3*((float)yaw[i_frame] + factor*(
            (float)yaw[i_frame]-(float)yaw[o_frame]));

        this_pitch = 1e-3*((float)pitch[i_frame] + factor*(
            (float)pitch[i_frame]-(float)pitch[o_frame]));

        this_roll = 1e-3*((float)roll[i_frame] + factor*(
            (float)roll[i_frame]-(float)roll[o_frame]));

        // Set ephemeris
        new_meas_spot->scOrbitState.rsat.Set(
            this_pos[0]*1e-3, this_pos[1]*1e-3, this_pos[2]*1e-3);
                                              
        new_meas_spot->scOrbitState.vsat.Set(
            this_vel[0]*1e-3, this_vel[1]*1e-3, this_vel[2]*1e-3);

        // Set attitude
        new_meas_spot->scAttitude.SetRPY(
            this_roll*dtr, this_pitch*dtr, this_yaw*dtr);

        for( int i_slice = 0; i_slice < 8; ++i_slice )
        {
           // Using 1d arrays 
           int slice_ind = i_frame * 100 * 8 + i_pulse * 8 + i_slice;

           //Check that pulse is a measurement pulse.
           //(bits 0,1 in state 0,0 => meas pulse)
           if( sigma0_mode_flag[pulse_ind] & (uint16)0x3 )
             continue;
           
           //Check that egg sigma0 is useable.
           if( sigma0_qual_flag[pulse_ind] & (uint16)0x1 ) // Sigma0 useable bit
             continue;

           // Create unsigned ints for checking slice flags.
           unsigned int peak_gain_flag  = (unsigned int)(1 << (i_slice * 4 + 0) );
           unsigned int neg_sig0_flag   = (unsigned int)(1 << (i_slice * 4 + 1) );
           unsigned int low_snr_flag    = (unsigned int)(1 << (i_slice * 4 + 2) );
           unsigned int center_loc_flag = (unsigned int)(1 << (i_slice * 4 + 3) );
        
           // Skip observations with peak_gain_flag or center_loc_flag set.
           if( slice_qual_flag[pulse_ind] & peak_gain_flag  ||
               slice_qual_flag[pulse_ind] & center_loc_flag )
             continue;
           
           // Done checking flags; create memory for this Meas object
           Meas* new_meas = new Meas();           
           
           // Set XK, EnSlice
           new_meas->XK      = pow(10.0,0.01*double(x_factor[slice_ind])/10.0);
           new_meas->EnSlice = pow(10.0,0.01*double(slice_sigma0[slice_ind])/10.0)
                             * pow(10.0,0.01*double(x_factor[slice_ind])/10.0)
                             / pow(10.0,0.01*double(slice_snr[slice_ind])/10.0);
        
           // Set lon, lat of observation
           double tmp_lat = 1e-4*dtr*double(slice_lat[slice_ind]);
           double tmp_lon = 1e-4*dtr*double(slice_lon[slice_ind]) 
                          / cos( cell_lat[pulse_ind]*dtr );
           
           tmp_lon += double(cell_lon[pulse_ind])*dtr;
           tmp_lat += double(cell_lat[pulse_ind])*dtr;
           
           // make longitude, latitude to be in range.
           if( tmp_lon < 0       ) tmp_lon += two_pi;
           if( tmp_lon >= two_pi ) tmp_lon -= two_pi;
           if( tmp_lat < -pi     ) tmp_lat = -pi;
           if( tmp_lat >  pi     ) tmp_lat =  pi;
           
           new_meas->centroid.SetAltLonGDLat( 0.0, tmp_lon, tmp_lat );
           
           // Set inc angle
           new_meas->incidenceAngle = 0.01*dtr*double(slice_incidence[slice_ind]);

           // Get attenuation map value
           float atten_dB = 0;
           if( use_atten_map )
             atten_dB = attenmap.GetNadirAtten( tmp_lon, tmp_lat, sec_year )
                      / cos( new_meas->incidenceAngle );

           float atten_lin = pow(10.0,0.1*atten_dB);
           
           // Set slice azimuth angle & change for QSCATsim convention
           float northAzimuth       = 0.01*dtr*double(slice_azimuth[slice_ind]);
           new_meas->eastAzimuth    = (450.0*dtr - northAzimuth);
           if (new_meas->eastAzimuth >= two_pi) new_meas->eastAzimuth -= two_pi;
           
           // Set scanAngle
           new_meas->scanAngle = 0.01*dtr*double(antenna_azimuth[pulse_ind]);
           //if (new_meas->scanAngle < 0) new_meas->scanAngle += two_pi;

           // Set measurement type
           if (sigma0_mode_flag[pulse_ind] & 0x4) {
    	     new_meas->beamIdx = 1;
    	     new_meas->measType = Meas::VV_MEAS_TYPE;
           } 
           else {
    	     new_meas->beamIdx = 0;
		     new_meas->measType = Meas::HH_MEAS_TYPE;
    	   }

           // Get sigma-0 Mode from flags
           // Sigma0 mode is index into kpr table
           // gate width flags are bits (6,7,8).
           uint16 gatewidth = (sigma0_mode_flag[pulse_ind] & (uint16)0x1C0 ) >> 6;
           
           if( (gatewidth != (uint16)3) && (gatewidth != (uint16)4)) {
             fprintf(stderr,"%s: Gatewidth not equal to 3 or 4; %d!\n",command,gatewidth);
             exit(1);
           }
           
           if(gatewidth==(uint16)3) {
             new_meas->bandwidth = 6.929 * 1000;
           } else if ( gatewidth==(uint16)4) {
             new_meas->bandwidth = 12.933 * 1000;
           }
           // Set sigma0 + correct for attenuation if !do_composote
           if( do_composite )
             new_meas->value   = pow(10.0,0.01*double(slice_sigma0[slice_ind])/10.0);
           else
             new_meas->value   = pow(10.0,(atten_dB+0.01*double(slice_sigma0[slice_ind]))/10.0);

           // Check for negative sigma-0
           if ( slice_qual_flag[pulse_ind] & neg_sig0_flag )  
             new_meas->value *= -1;
                      
           // Set starting slice number of best 8 slices based on quality flags.
           uint16 slice_shift_bits = (sigma0_qual_flag[pulse_ind] & (uint16)0xC00) >> 10;
           if( slice_shift_bits == 0 )      // slices 0-7 of 0-9 used
             new_meas->startSliceIdx = i_slice;   
           else if( slice_shift_bits == 1 ) // slices 1-8 of 0-9 used
             new_meas->startSliceIdx = i_slice + 1; 
           else if( slice_shift_bits == 2 ) // slices 2-9 of 0-9 used
             new_meas->startSliceIdx = i_slice + 2; 
           else {
             fprintf(stderr,"Unexpected value of slice shift flags!\n");
             exit(1);
           }
           //printf("pulse_ind, slice_shift_bits: %d %x\n", pulse_ind, slice_shift_bits );
           
           new_meas->startSliceIdx -= 5;
           if( new_meas->startSliceIdx >= 0 ) new_meas->startSliceIdx += 1;
           
           // Get Land map value
           new_meas->landFlag = 0;
           if( qs_landmap.IsLand(tmp_lon,tmp_lat,1) )
             new_meas->landFlag += 1; // bit 0 for land
           
           // Get Ice map value
           if( qs_icemap.IsIce(tmp_lon,tmp_lat,new_meas->beamIdx) )
             new_meas->landFlag += 2; // bit 1 for ice
           
           // Set numSlices, A, B, and C terms depending on if USE_COMPOSITING is set or not
           if( do_composite == 0 ) {
             // Set numslices == -1 to indicate software to 
             // treat A,B,C as kp_alpha, kp_beta, kp_gamma.
             new_meas->numSlices = -1;
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
             double sos = 0.01*double(slice_sigma0[slice_ind])
                        - 0.01*double(slice_snr[slice_ind]);
             sos        = pow( 10.0, sos/10.0 ); 
             
             double kpr2      = 1 + kprs2 + kpri2;
             double kp_alpha  = (1+double(1e-4*slice_kpc_a[slice_ind])) * kpr2;
             double kp_beta   = kp_B[gatewidth*2+new_meas->beamIdx]     * kpr2 * sos;
             double kp_gamma  = kp_C[gatewidth*2+new_meas->beamIdx]     * kpr2 * sos * sos;
              
             // Set kp alpha, beta, gamma and correct for attenuation
             new_meas->A = 1 + ( kp_alpha - 1 ) * atten_lin * atten_lin;
             new_meas->B = kp_beta              * atten_lin * atten_lin;
             new_meas->C = kp_gamma             * atten_lin * atten_lin;           
           }
           else {
             // Set numslices == 1 to indicate software to treat A,B,C as kp_a, kp_b, kp_c.
             new_meas->numSlices = 1; 
           
             // Set A, B, C for later compositing.
             new_meas->A = double(1e-4*slice_kpc_a[slice_ind]);
             new_meas->B = kp_B[gatewidth*2+new_meas->beamIdx];
             new_meas->C = kp_C[gatewidth*2+new_meas->beamIdx];
           }
           
           if( fabs(new_meas->value) > 1.0e5 ) {
             delete new_meas;
             continue;
           }
           
           // Stick this meas in the measSpot
           new_meas_spot->Append(new_meas);
        }

        // Compute land fractions for various coastal_methos
        if(coastal_method != NONE) {

          float this_frequency_shift = (float)frequency_shift[pulse_ind];
          double spot_lon = double(cell_lon[pulse_ind])*dtr;
          double spot_lat = double(cell_lat[pulse_ind])*dtr;

          new_meas_spot->ComputeLandFraction( &lmap, coast_dist,
                                              &antenna, this_frequency_shift,
                                              spot_lon, spot_lat, lcres_accum);

          // Loop through Meas in this MeasSpot, decide to discard or not.
          Meas* this_meas = new_meas_spot->GetHead();
          while(this_meas) {
            int remove_it = 0;

            // Land contamination ratio
            float lcr = this_meas->bandwidth;

            if(coastal_method == LCR && lcr > land_frac_threshold)
              remove_it = 1;

            if(coastal_method == LCRES_FLAG || coastal_method == LCRES_CORR) {
              int ipol = -1;
              if(this_meas->measType == Meas::VV_MEAS_TYPE) ipol = 0;
              if(this_meas->measType == Meas::HH_MEAS_TYPE) ipol = 1;

              int is_asc = (new_meas_spot->scOrbitState.vsat.GetZ() < 0) ? 0 : 1;

              float lcres;
              if(lcr == 0 || lcr == 1) {
                lcres = lcr;

              } else {
                float land_expected_value;
                lcres_map_tiles->Get(
                    &this_meas->centroid, this_meas->eastAzimuth, ipol, is_asc,
                    &land_expected_value);
                lcres = lcr * land_expected_value;
              }

              if(coastal_method == LCRES_FLAG) {
                if(lcres > lcres_thresh_flag)
                  remove_it = 1;

              } else {
                if(lcres > lcres_thresh_corr)
                  remove_it = 1;

                else {
                  this_meas->value -= lcres;
                  this_meas->value /= (1-lcr);
                }
              }
            }

            if(remove_it) {
              this_meas = new_meas_spot->RemoveCurrent();
              delete this_meas;
              this_meas = new_meas_spot->GetCurrent();
            } else {
              // unset land flag
              this_meas->landFlag &= ~(1<<0);
              this_meas = new_meas_spot->GetNext();
            }
          }
        }

        // Stick this measSpot in the spotList.
        l1b.frame.spotList.Append(new_meas_spot);
      }
      
      // Write the ephemeris.
      if( !l1b.frame.spotList.WriteEphemeris(eph_fp) )
      {
        fprintf(stderr, "%s: writing to %s failed.\n", command, ephemeris_file );
        return (1);      
      }
      
      // Copy some stuff into l1b.frame object
      l1b.frame.frame_i              = i_frame;
      l1b.frame.num_l1b_frames       = num_l1b_frames;
      l1b.frame.num_pulses_per_frame = 100;
      l1b.frame.num_slices_per_pulse = 8;
      
      // Write this L1BFrame
      if( ! l1b.WriteDataRec() ) {
        fprintf(stderr, "%s: writing to %s failed.\n", command, l1b.GetOutputFilename() );
        return (1);
      }
      
      if( i_frame % 50 == 0 )
        printf("Wrote %d frames of %d\n", i_frame, num_l1b_frames);
        //if( i_frame == 1000 ) exit(1);
    }

    if(coastal_method == LCRES_ACCUM)
        lcres_accum->Write(config_list.Get(LCRES_ACCUM_FILE_KEYWORD));

    fclose(eph_fp);
    
    //Free the calloc-ed arrays.
    free(x_pos);
    free(y_pos);
    free(z_pos);
    free(x_vel);
    free(y_vel);
    free(z_vel);
    free(roll);
    free(pitch);
    free(yaw);
    free(antenna_azimuth);
    free(cell_lat);
    free(cell_lon);
    free(frequency_shift);
    free(slice_lat);
    free(slice_lon);
    free(slice_sigma0);
    free(x_factor);
    free(slice_azimuth);
    free(slice_incidence);
    free(slice_snr);
    free(slice_kpc_a);
    free(sigma0_mode_flag);
    free(sigma0_qual_flag);
    
    #ifdef EXTEND_EPHEM
    float extension_step=1; // units are seconds
    float extension_num=1500; // number of extra steps on each side
    if (ephemeris_file != NULL) {
          ExtendEphemerisEnds("l1bhdf_to_l1b_tmpfile",ephemeris_file,extension_step,extension_num, output_file, sd_id, command);
    }
    #endif    

    if( Hclose(h_id) || SDend( sd_id ) )
    {
      fprintf(stderr,"ERROR closing hdf file %s.\n",l1b_hdf_file);
      exit(1);
    }
    
    return 0;
    
} // main
