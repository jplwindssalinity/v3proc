//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.    //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//        l1b_isro_to_l1b.C
//
// SYNOPSIS
//        l1b_isro_to_l1b config_file
//
// DESCRIPTION
//        Generates l1b.dat file given a L1B HDF file from ISRO.
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
//        Sermsak Jaruwatanadilok (jaruwata@jpl.nasa.gov)
//----------------------------------------------------------------------


//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
    "@(#) $Id$";

//#define EXTEND_EPHEM
#define START_SLICE_INDEX    -4

#define QS_LANDMAP_FILE_KEYWORD 		"QS_LANDMAP_FILE"
#define QS_ICEMAP_FILE_KEYWORD	 		"QS_ICEMAP_FILE"


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
/* hdf5 include */
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

//-----------//
// CONSTANTS //
//-----------//

const char* usage_array[] = { "config_file", 0 };

/* get number of l1b frame  - checked */
int return_num_l1b_frames (int32 sd_id )
{
	hid_t dataspace;
	hsize_t   dims_out[2];
	int32 maxdim, rank;
	int32 sds_id = H5Dopen1(sd_id,"Scan_number");
	dataspace = H5Dget_space(sds_id);    /* dataspace handle */
	rank = H5Sget_simple_extent_dims(dataspace,dims_out, NULL);
	return(dims_out[0]);
} 

/* read data - checked */
int read_SDS( int32 sd_id, char* sds_name, void* data_buffer )
{
	hid_t dataspace, type_id;
	hsize_t dims_out[2];
	int32 maxdim, status, rank;
	int32 sds_id = H5Dopen1(sd_id,sds_name);
	
	dataspace = H5Dget_space(sds_id);    /* dataspace handle */
	rank = H5Sget_simple_extent_dims(dataspace,dims_out, NULL);
	if (rank ==1) {		dims_out[1] = 1; dims_out[2] =1;}
	if (rank ==2) {		dims_out[2] = 1;}
	printf("Reading %s  ",sds_name);
	    printf("rank %d, dimensions %lu x %lu x %lu \n", rank,
	 (unsigned long)(dims_out[0]), (unsigned long)(dims_out[1]), 
	 (unsigned long)(dims_out[2]));
	type_id = H5Dget_type(sds_id);
    status = H5Dread(sds_id, type_id, H5S_ALL, H5S_ALL,H5P_DEFAULT, data_buffer);	
	if (status<0) {fprintf(stderr,"Data reading error dataname %s.\n",sds_name);}
	H5Dclose(sds_id);
	return(0);
}

/* read attribute - checked */
int read_attr( int32 obj_id, char* attr_name, char* attr_string )
{
	hid_t attr, attr_type;
	herr_t  ret;                /* Return value */
	int32 data_type, count;
	char attr_name_[80];
	
	attr = H5Aopen(obj_id, attr_name , H5P_DEFAULT);
	attr_type = H5Aget_type(attr);
	ret  = H5Aread(attr, attr_type, &attr_string[0]);
	ret =  H5Aclose(attr); 
}

/* initialize string - from Alex */
int init_string( char* string, int length )
{
	for( int ii = 0; ii < length; ++ii )
	  string[ii] = NULL;
	return(1);
}

/* read orbit element from attribute - checked */
int read_orbit_elements_from_attr( int32 sd_id, double &lon_asc_node,
		double &orbit_period, double &orbit_inc, double &orbit_semi_major,
		double &orbit_ecc, double &t_asc_node)
{
    char attr_string[1024];
  	char attr_asc_node_t_str[CODE_B_TIME_LENGTH];
  	
	read_attr( sd_id, "Equator Crossing Longitude", attr_string );
	lon_asc_node = atof( attr_string );
	
	read_attr( sd_id, "Orbit Period", attr_string );
	orbit_period = atof( attr_string );
	
	read_attr( sd_id, "Orbit Inclination", attr_string );
	orbit_inc = atof( attr_string );
	
	read_attr( sd_id, "Orbit Semi-major Axis", attr_string );
	orbit_semi_major = atof( attr_string );
	
	read_attr( sd_id, "Orbit Eccentricity", attr_string );
	orbit_ecc = atof( attr_string );
	
	read_attr( sd_id, "Equator Crossing Date", attr_string );
    ETime eTime_asc_node;
    eTime_asc_node.FromCodeB( attr_string );
	
	ETime etime;
	etime.FromCodeB("1970-001T00:00:00.000");
	double time_base = (double)etime.GetSec() + (double)etime.GetMs()/1000;	
    
    t_asc_node = (double)eTime_asc_node.GetSec() 
	+ (double)eTime_asc_node.GetMs()/1000 - time_base;
	
	return(0);
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
    //QSKpr        qs_kpr;

    //------------------------//
    // parse the command line //
    //------------------------//

    command = no_path(argv[0]);

    if( !( argc == 2) )
        usage(command, usage_array, 1);
        
    config_file = argv[1];
    
    if (! config_list.Read(config_file)) {
        fprintf(stderr, "%s: error reading config file %s\n",
            command, config_file);
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
    
    // Check for QS_LANDMAP and QS_ICEMAP keywords
    qslandmap_file = config_list.Get(QS_LANDMAP_FILE_KEYWORD);
    qsicemap_file  = config_list.Get(QS_ICEMAP_FILE_KEYWORD);
    
    if( qslandmap_file == NULL || qsicemap_file == NULL ) {
      fprintf(stderr,"%s: Must specify QS_LANDMAP_FILE and QS_ICEMAP_FILE in config!\n",command);
      exit(1);
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
    
	int32 h_id, sd_id;
	
	h_id   = H5Fopen( l1b_hdf_file, H5F_ACC_RDONLY, H5P_DEFAULT );   
	sd_id  = H5Gopen1(h_id,"science_data");
    
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

	uint16* antenna_azimuthI = (uint16*)calloc(sizeof(uint16),num_l1b_frames*281);
	uint16* antenna_azimuthO = (uint16*)calloc(sizeof(uint16),num_l1b_frames*282);

    int16* cell_latI = (int16*)calloc(sizeof(int16),num_l1b_frames*281);
    int16* cell_lonI = (int16*)calloc(sizeof(int16),num_l1b_frames*281);   
    int16* cell_latO = (int16*)calloc(sizeof(int16),num_l1b_frames*282);
    int16* cell_lonO = (int16*)calloc(sizeof(int16),num_l1b_frames*282);  

    int16* slice_latI       = (int16*)calloc(sizeof(int16),num_l1b_frames*281*7);
    int16* slice_latO       = (int16*)calloc(sizeof(int16),num_l1b_frames*282*12);
    int16* slice_lonI       = (int16*)calloc(sizeof(int16),num_l1b_frames*281*7);
    int16* slice_lonO       = (int16*)calloc(sizeof(int16),num_l1b_frames*282*12);
    int16* slice_sigma0I    = (int16*)calloc(sizeof(int16),num_l1b_frames*281*7);
    int16* slice_sigma0O    = (int16*)calloc(sizeof(int16),num_l1b_frames*282*12);
    int16* x_factorI        = (int16*)calloc(sizeof(int16),num_l1b_frames*281*7);
    int16* x_factorO        = (int16*)calloc(sizeof(int16),num_l1b_frames*282*12);
    uint16* slice_azimuthI  = (uint16*)calloc(sizeof(uint16),num_l1b_frames*281*7);
    uint16* slice_azimuthO  = (uint16*)calloc(sizeof(uint16),num_l1b_frames*282*12);
    int16* slice_incidenceI = (int16*)calloc(sizeof(int16),num_l1b_frames*281*7);
    int16* slice_incidenceO = (int16*)calloc(sizeof(int16),num_l1b_frames*282*12);
    int16* slice_snrI       = (int16*)calloc(sizeof(int16),num_l1b_frames*281*7);
    int16* slice_snrO       = (int16*)calloc(sizeof(int16),num_l1b_frames*282*12);
    float* slice_kpI     = (float*)calloc(sizeof(float),num_l1b_frames*281*7);    
    float* slice_kpO     = (float*)calloc(sizeof(float),num_l1b_frames*282*12);    
		
	// !! flags !!

	uint16* Inner_beam_footprint_sigma0_flag = (uint16*)calloc(sizeof(uint16),num_l1b_frames*281);
	uint16* Outer_beam_footprint_sigma0_flag = (uint16*)calloc(sizeof(uint16),num_l1b_frames*282);
	uint16* Inner_beam_slice_sigma0_flag = (uint16*)calloc(sizeof(uint16),num_l1b_frames*281*7);
	uint16* Outer_beam_slice_sigma0_flag = (uint16*)calloc(sizeof(uint16),num_l1b_frames*282*12);

    // Start reading data from hdf file //
	read_SDS( sd_id, "Inner_beam_footprint_latitude",        &cell_latI[0]  );
	read_SDS( sd_id, "Outer_beam_footprint_latitude",        &cell_latO[0]  );
	read_SDS( sd_id, "Inner_beam_footprint_longitude",        &cell_lonI[0] );
	read_SDS( sd_id, "Outer_beam_footprint_longitude",        &cell_lonO[0] );
	read_SDS( sd_id, "Inner_beam_footprint_azimuth_angle", &antenna_azimuthI[0] );  
	read_SDS( sd_id, "Outer_beam_footprint_azimuth_angle", &antenna_azimuthO[0] ); 
    read_SDS( sd_id, "Inner_beam_slice_sigma0",     &slice_sigma0I[0]    );
	read_SDS( sd_id, "Outer_beam_slice_sigma0",     &slice_sigma0O[0]    );  


	read_SDS( sd_id, "Inner_beam_slice_Xfactor",         &x_factorI[0] );
	read_SDS( sd_id, "Outer_beam_slice_Xfactor",         &x_factorO[0]        );
	read_SDS( sd_id, "Inner_beam_slice_azimuth_angle",    &slice_azimuthI[0]   );  
	read_SDS( sd_id, "Outer_beam_slice_azimuth_angle",    &slice_azimuthO[0]   ); 
	read_SDS( sd_id, "Inner_beam_slice_incidence_angle",  &slice_incidenceI[0] );  
	read_SDS( sd_id, "Outer_beam_slice_incidence_angle",  &slice_incidenceO[0] );
	read_SDS( sd_id, "Inner_beam_slice_SNR",        &slice_snrI[0]       );
	read_SDS( sd_id, "Outer_beam_slice_SNR",        &slice_snrO[0]       );  
	   /* Kp ??  still need to resolve */
	read_SDS( sd_id, "Inner_beam_slice_Kp",      &slice_kpI[0]     );  
	read_SDS( sd_id, "Outer_beam_slice_Kp",      &slice_kpO[0]     ); 
	   
	read_SDS( sd_id, "Inner_beam_slice_latitude",        &slice_latI[0]       ); 
	read_SDS( sd_id, "Outer_beam_slice_latitude",        &slice_latO[0]       ); 
	read_SDS( sd_id, "Inner_beam_slice_longitude",        &slice_lonI[0]       );         
	read_SDS( sd_id, "Outer_beam_slice_longitude",        &slice_lonO[0]       );         

	// read flag 
    read_SDS( sd_id, "Inner_beam_footprint_sigma0_flag", &Inner_beam_footprint_sigma0_flag[0] );
	read_SDS( sd_id, "Outer_beam_footprint_sigma0_flag", &Outer_beam_footprint_sigma0_flag[0] ); 
    read_SDS( sd_id, "Inner_beam_slice_sigma0_flag", &Inner_beam_slice_sigma0_flag[0] );
	read_SDS( sd_id, "Outer_beam_slice_sigma0_flag", &Outer_beam_slice_sigma0_flag[0] ); 


	// read scan start time => for frame_time parameter
	char frame_time_str[64];	
	char sst[num_l1b_frames][22];
	read_SDS(sd_id,"Scan_start_time",&sst[0]);
	
        
    for( int i_frame = 0; i_frame < num_l1b_frames; ++i_frame ) {

		for ( int ii=0;ii<64;++ii)
        frame_time_str[ii] = NULL;
		strcpy(frame_time_str,sst[i_frame]);

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

      printf("i_frame: %6d; frame_time: %s \n",i_frame,frame_time_str);
      
      // Get seconds of year for attenuation map.  Note that strtok replaces the 
      // tokens with NULL; frame_time_str is basically unusable after this point.
      int year      = atoi( strtok( &frame_time_str[0], "-"  ) );
      int doy       = atoi( strtok( NULL,               "T"  ) );
      int hour      = atoi( strtok( NULL,               ":"  ) );
      int minute    = atoi( strtok( NULL,               ":"  ) );
      float seconds = atof( strtok( NULL,               "\0" ) );
      
      double sec_year = seconds+60.0*(float(minute)+60.0*(float(hour)+24.0*float(doy - 1)));
            
      for( int i_pulse = 0; i_pulse < 281; ++i_pulse ) {
        
        MeasSpot* new_meas_spot = new MeasSpot();
        
        // Set time in meas_spot and in orbit state
        new_meas_spot->time              = frame_time;
        new_meas_spot->scOrbitState.time = frame_time;

		// Set ephemeris to zero
		new_meas_spot->scOrbitState.rsat.Set( double(0.0*1e-3), // convert to km
											   double(0.0*1e-3),
											   double(0.0*1e-3) );
		  
		new_meas_spot->scOrbitState.vsat.Set( double(0.0*1e-3), // convert to km/s
											   double(0.0*1e-3),
											   double(0.0*1e-3) );
		// Set attitude
		new_meas_spot->scAttitude.SetRPY( float(0.0)*1e-3*dtr,
										   float(0.0)*1e-3*dtr,
										   float(0.0)*1e-3*dtr );
		  
        for( int i_slice = 0; i_slice < 7; ++i_slice )
        {
			// Using 1d arrays 
			int pulse_ind = i_frame * 281 + i_pulse;
			int slice_ind = i_frame *281*7 +i_pulse*7+ i_slice;

			// check footprint flag
			if( Inner_beam_footprint_sigma0_flag[pulse_ind] & (uint16)0x6070 )
				continue;
			// check slice flag
			if( Inner_beam_slice_sigma0_flag[slice_ind] & (uint16)0x6070 )
				continue;
			//printf("slice_ind is %d \n",slice_ind);
           // Done checking flags; create memory for this Meas object
           Meas* new_meas = new Meas();           
           
           // Set XK, EnSlice
           new_meas->XK      = pow(10.0,0.01*double(x_factorI[slice_ind])/10.0);
           new_meas->EnSlice = pow(10.0,0.01*double(slice_sigma0I[slice_ind])/10.0)
                             * pow(10.0,0.01*double(x_factorI[slice_ind])/10.0)
                             / pow(10.0,0.01*double(slice_snrI[slice_ind])/10.0);
			

        
           // Set lon, lat of observation
           /*double tmp_lat = 1e-4*dtr*double(slice_lat[slice_ind]);
           double tmp_lon = 1e-4*dtr*double(slice_lon[slice_ind]) 
                          / cos( cell_latI[pulse_ind]*dtr );*/
			
           double tmp_lon = 0.01*double(slice_lonI[slice_ind])*dtr;
           double tmp_lat = 0.01*double(slice_latI[slice_ind])*dtr;
           
           // make longitude, latitude to be in range.
           if( tmp_lon < 0       ) tmp_lon += two_pi;
           if( tmp_lon >= two_pi ) tmp_lon -= two_pi;
           if( tmp_lat < -pi/2     ) tmp_lat = -pi/2;
           if( tmp_lat >  pi/2     ) tmp_lat =  pi/2;
 
			
           new_meas->centroid.SetAltLonGDLat( 0.0, tmp_lon, tmp_lat );
           
           // Set inc angle
           new_meas->incidenceAngle = 0.01*dtr*double(slice_incidenceI[slice_ind]);

           // Get attenuation map value
           float atten_dB = 0;
           if( use_atten_map )
             atten_dB = attenmap.GetNadirAtten( tmp_lon, tmp_lat, sec_year )
                      / cos( new_meas->incidenceAngle );

           float atten_lin = pow(10.0,0.1*atten_dB);
           //printf("atten_lin is %f \n",atten_lin);
			
           // Set slice azimuth angle & change for QSCATsim convention
           float northAzimuth       = 0.01*dtr*double(slice_azimuthI[slice_ind]);
           new_meas->eastAzimuth    = (450.0*dtr - northAzimuth);
           if (new_meas->eastAzimuth >= two_pi) new_meas->eastAzimuth -= two_pi;
           
           // Set scanAngle
           new_meas->scanAngle = 0.01*dtr*double(antenna_azimuthI[pulse_ind]);

			new_meas->measType = Meas::HH_MEAS_TYPE;         

           
           // Set sigma0 + correct for attenuation if !do_composote
           if( do_composite )
             new_meas->value   = pow(10.0,0.01*double(slice_sigma0I[slice_ind])/10.0);
           else
             new_meas->value   = pow(10.0,(atten_dB+0.01*double(slice_sigma0I[slice_ind]))/10.0);


           // Check for negative sigma-0
			if( Inner_beam_slice_sigma0_flag[slice_ind] & (uint16)0x0200 )
             new_meas->value *= -1;
                      
           // Set starting slice number of best 8 slices based on quality flags.
			/* This definitely need to be changed since quality flags are different ?? */

			new_meas->startSliceIdx = i_slice;   

           //printf("pulse_ind, slice_shift_bits: %d %x\n", pulse_ind, slice_shift_bits );
           
           new_meas->startSliceIdx -= 5;
           if( new_meas->startSliceIdx >= 0 ) new_meas->startSliceIdx += 1;
           
           // Get Land map value
           new_meas->landFlag = 0;
           if( qs_landmap.IsLand(tmp_lon,tmp_lat,1) )
             new_meas->landFlag += 1; // bit 0 for land
           
           // Get Ice map value
           if( Inner_beam_slice_sigma0_flag[slice_ind] & (uint16)0xE000 )
             new_meas->landFlag += 2; // bit 1 for ice
           
           // Set numSlices, A, B, and C terms depending on if USE_COMPOSITING is set or not
			if( do_composite == 0 ) {             new_meas->numSlices = -1;}
             // Set numslices == -1 to indicate software to 
             // treat A,B,C as kp_alpha, kp_beta, kp_gamma.
           else {			   new_meas->numSlices = 1; }
             // Set numslices == 1 to indicate software to treat A,B,C as kp_a, kp_b, kp_c.

			double snrfac, NLinv;
			snrfac = 1+2/double(slice_snrI[slice_ind])
				+1/(double(slice_snrI[slice_ind])*double(slice_snrI[slice_ind]));
			NLinv = (double(slice_kpI[slice_ind])*double(slice_kpI[slice_ind]))/snrfac;
             // Set A, B, C for later compositing.
             new_meas->A = NLinv;
             new_meas->B = 2*NLinv;
             new_meas->C = NLinv;

           
           // Stick this meas in the measSpot
           new_meas_spot->Append(new_meas);
        }
        
        // Stick this measSpot in the spotList.
        l1b.frame.spotList.Append(new_meas_spot);
      }

	  for( int i_pulse = 0; i_pulse < 282; ++i_pulse ) {
			
			MeasSpot* new_meas_spot = new MeasSpot();
			
			// Set time in meas_spot and in orbit state
			new_meas_spot->time              = frame_time;
			new_meas_spot->scOrbitState.time = frame_time;
			
			
			/* set ephemeris to zero ?? */  
			new_meas_spot->scOrbitState.rsat.Set( double(0.0*1e-3), // convert to km
												 double(0.0*1e-3),
												 double(0.0*1e-3) );
			
			new_meas_spot->scOrbitState.vsat.Set( double(0.0*1e-3), // convert to km/s
												 double(0.0*1e-3),
												 double(0.0*1e-3) );
			// Set attitude
			new_meas_spot->scAttitude.SetRPY( float(0.0)*1e-3*dtr,
											 float(0.0)*1e-3*dtr,
											 float(0.0)*1e-3*dtr );
			
			
			
			for( int i_slice = 0; i_slice < 12; ++i_slice )
			{
				// Using 1d arrays 
				/*int pulse_ind = i_frame * 100     + i_pulse; */
				int pulse_ind = i_frame * 282 + i_pulse;
				/*int slice_ind = i_frame * 100 * 8 + i_pulse * 8 + i_slice;*/
				int slice_ind = i_frame *282*12 +i_pulse*12+ i_slice;
				
				// check footprint flag
				if( Outer_beam_footprint_sigma0_flag[pulse_ind] & (uint16)0x6070 )
					continue;
				// check slice flag
				if( Outer_beam_slice_sigma0_flag[slice_ind] & (uint16)0x6070 )
					continue;
				
				
				// Done checking flags; create memory for this Meas object
				Meas* new_meas = new Meas();           
				
				// Set XK, EnSlice
				new_meas->XK      = pow(10.0,0.01*double(x_factorO[slice_ind])/10.0);
				new_meas->EnSlice = pow(10.0,0.01*double(slice_sigma0O[slice_ind])/10.0)
				* pow(10.0,0.01*double(x_factorO[slice_ind])/10.0)
				/ pow(10.0,0.01*double(slice_snrO[slice_ind])/10.0);
				
				// Set lon, lat of observation
				/*double tmp_lat = 1e-4*dtr*double(slice_latO[slice_ind]);
				double tmp_lon = 1e-4*dtr*double(slice_lonO[slice_ind]) 
				/ cos( cell_latO[pulse_ind]*dtr );*/
				
				double tmp_lon = 0.01*double(slice_lonO[slice_ind])*dtr;
				double tmp_lat = 0.01*double(slice_latO[slice_ind])*dtr;
				
				// make longitude, latitude to be in range.
				if( tmp_lon < 0       ) tmp_lon += two_pi;
				if( tmp_lon >= two_pi ) tmp_lon -= two_pi;
				if( tmp_lat < -pi/2     ) tmp_lat = -pi/2;
				if( tmp_lat >  pi/2     ) tmp_lat =  pi/2;
				
				new_meas->centroid.SetAltLonGDLat( 0.0, tmp_lon, tmp_lat );
				
				// Set inc angle
				new_meas->incidenceAngle = 0.01*dtr*double(slice_incidenceO[slice_ind]);
				
				// Get attenuation map value
				float atten_dB = 0;
				if( use_atten_map )
					atten_dB = attenmap.GetNadirAtten( tmp_lon, tmp_lat, sec_year )
					/ cos( new_meas->incidenceAngle );
				
				float atten_lin = pow(10.0,0.1*atten_dB);
				
				// Set slice azimuth angle & change for QSCATsim convention
				/* check azimuth angle convention ?? */
				float northAzimuth       = 0.01*dtr*double(slice_azimuthO[slice_ind]);
				new_meas->eastAzimuth    = (450.0*dtr - northAzimuth);
				if (new_meas->eastAzimuth >= two_pi) new_meas->eastAzimuth -= two_pi;
				
				// Set scanAngle
				new_meas->scanAngle = 0.01*dtr*double(antenna_azimuthO[pulse_ind]);
				//if (new_meas->scanAngle < 0) new_meas->scanAngle += two_pi;
				
				// Set measurement type depending on inc angle
				/* this is outer beam so VV */
				new_meas->measType = Meas::VV_MEAS_TYPE;         
				
				
				// Set sigma0 + correct for attenuation if !do_composote
				if( do_composite )
					new_meas->value   = pow(10.0,0.01*double(slice_sigma0O[slice_ind])/10.0);
				else
					new_meas->value   = pow(10.0,(atten_dB+0.01*double(slice_sigma0O[slice_ind]))/10.0);
				
				// Check for negative sigma-0
				if( Outer_beam_slice_sigma0_flag[slice_ind] & (uint16)0x0200 )
					new_meas->value *= -1;
				
				// Set starting slice number of best 8 slices based on quality flags.
				/* This definitely need to be changed since quality flags are different ?? */

				new_meas->startSliceIdx = i_slice;
				//printf("pulse_ind, slice_shift_bits: %d %x\n", pulse_ind, slice_shift_bits );
				
				new_meas->startSliceIdx -= 5;
				if( new_meas->startSliceIdx >= 0 ) new_meas->startSliceIdx += 1;
				
				// Get Land map value
				new_meas->landFlag = 0;
				if( qs_landmap.IsLand(tmp_lon,tmp_lat,1) )
					new_meas->landFlag += 1; // bit 0 for land
				
				// Get Ice map value
				if( Outer_beam_slice_sigma0_flag[slice_ind] & (uint16)0xE000 )
					new_meas->landFlag += 2; // bit 1 for ice

				
				// Set numSlices, A, B, and C terms depending on if USE_COMPOSITING is set or not
				if( do_composite == 0 ) {					new_meas->numSlices = -1;}
					// Set numslices == -1 to indicate software to 
					// treat A,B,C as kp_alpha, kp_beta, kp_gamma.

				else {					new_meas->numSlices = 1; }
					// Set numslices == 1 to indicate software to treat A,B,C as kp_a, kp_b, kp_c.

					
				double snrfac, NLinv;
				snrfac = 1+2/double(slice_snrO[slice_ind])
					+1/(double(slice_snrO[slice_ind])*double(slice_snrO[slice_ind]));
				NLinv = (double(slice_kpO[slice_ind])*double(slice_kpO[slice_ind]))/snrfac;
				// Set A, B, C for later compositing.
				new_meas->A = NLinv;
				new_meas->B = 2*NLinv;
				new_meas->C = NLinv;
	
				
				// Stick this meas in the measSpot
				new_meas_spot->Append(new_meas);
			}
			
			// Stick this measSpot in the spotList.
			l1b.frame.spotList.Append(new_meas_spot);
		}
		
		
      // Write the ephemeris.
      if( !l1b.frame.spotList.WriteEphemeris(eph_fp) )
      {
        fprintf(stderr, "%s: writing to %s failed.\n", command, l1b.GetOutputFilename() );
        return (1);      
      }
      
      // Copy some stuff into l1b.frame object
		/* This need to change */
      l1b.frame.frame_i              = i_frame;
      l1b.frame.num_l1b_frames       = num_l1b_frames;
      l1b.frame.num_pulses_per_frame = 563;
      l1b.frame.num_slices_per_pulse = 12;
      
      // Write this L1BFrame
      if( ! l1b.WriteDataRec() ) {
        fprintf(stderr, "%s: writing to %s failed.\n", command, ephemeris_file );
        return (1);
      }
      
      if( i_frame % 1024 == 0 )
        printf("Wrote %d frames of %d\n", i_frame, num_l1b_frames);
    }
    
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
    free(antenna_azimuthI);
    free(antenna_azimuthO);
    free(cell_latI);
	free(cell_latO);
    free(cell_lonI);
    free(cell_lonO);
    free(slice_latI);
	free(slice_latO);
    free(slice_lonI);
    free(slice_lonO);	
    free(slice_sigma0I);
    free(slice_sigma0O);
    free(x_factorI);
	free(x_factorO);
    free(slice_azimuthI);
	free(slice_azimuthO);
    free(slice_incidenceI);
    free(slice_incidenceO);
    free(slice_snrI);
	free(slice_snrO);
    free(slice_kpI);
	free(slice_kpO);
    
    #ifdef EXTEND_EPHEM
    float extension_step=1; // units are seconds
    float extension_num=1500; // number of extra steps on each side
    if (ephemeris_file != NULL) {
          ExtendEphemerisEnds("l1bhdf_to_l1b_tmpfile",ephemeris_file,extension_step,extension_num, output_file, sd_id, command);
    }
    #endif    

	H5Gclose(sd_id);
	H5Fclose(h_id);

    
    
    return 0;
    
} // main
