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
//#define EXTEND_EPHEM
//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
    "@(#) $Id$";

#define START_SLICE_INDEX    -4
#define EXTEND_EPHEM


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

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//
void landFlagL1BFrame(L1BFrame* f, LandMap* l){
  MeasSpotList* msl= &(f->spotList);
  for(MeasSpot* spot=msl->GetHead();spot;spot=msl->GetNext()){
    for(Meas* meas=spot->GetHead();meas;meas=spot->GetNext()){
      double alt,gdlat,lon;
      meas->centroid.GetAltLonGDLat(&alt,&lon,&gdlat);
      meas->landFlag=l->IsLand(lon,gdlat);
    }
  }
}

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
    const char*  command        = NULL;
    char*        l1b_hdf_file   = NULL;
    char*        output_file    = NULL;
    char*        ephemeris_file = NULL;
    char*        landmapfile    = NULL;
    char*        config_file    = NULL;
    
    ConfigList   config_list;
    LandMap      landmap;
    
    landmap.Initialize(NULL,0);

    //------------------------//
    // parse the command line //
    //------------------------//

    command = no_path(argv[0]);

    if (argc != 2)
        usage(command, usage_array, 1);
        
    config_file = argv[1];
    if (! config_list.Read(config_file))
    {
        fprintf(stderr, "%s: error reading config file %s\n",
            command, config_file);
        exit(1);
    }

    //---------------------//
    // check for config file parameters //
    //---------------------//

    config_list.DoNothingForMissingKeywords();

    l1b_hdf_file = config_list.Get(L1B_HDF_FILE_KEYWORD);
    if (l1b_hdf_file == NULL) {
        fprintf(stderr, "%s: config file must specify L1B_HDF_FILE\n", command);
        exit(1);
    } else {
       printf("Using l1b HDF file: %s\n", l1b_hdf_file);
    }
    
    output_file = config_list.Get(L1B_FILE_KEYWORD);
    if (output_file == NULL) {
        fprintf(stderr, "%s: config file must specify L1B_FILE\n", command);
        exit(1);
    } else {
       printf("Using l1b file: %s\n", output_file);
    }
    
    landmapfile = config_list.Get(LANDMAP_FILE_KEYWORD);
    if (landmapfile != NULL) {
       landmap.Initialize(landmapfile,1);
       printf("Using landmap file: %s\n", landmapfile);
    }

    ephemeris_file = config_list.Get(EPHEMERIS_FILE_KEYWORD);
    if (ephemeris_file != NULL) {
        printf("Using ephemeris file: %s\n", ephemeris_file);
    }
    
    L1B l1b;
    
    // Prepare to write
    if (l1b.OpenForWriting(output_file) == 0) {
        fprintf(stderr, "%s: cannot open l1b file %s for output\n",
                            argv[0], output_file);
        exit(1);
    }

	if (ephemeris_file == NULL)
	{
	  fprintf(stderr, "%s: ERROR getting ephemeris filename from config file.\n",
              command);
      exit(1);
	}
	
	// open ephem file for writing.
	#ifdef EXTEND_EPHEM
	FILE* eph_fp = fopen("l1bhdf_to_l1b_tmpfile", "w");
	#else
	FILE* eph_fp = fopen(ephemeris_file, "w");
	#endif
	if (eph_fp == NULL)
	{
	  fprintf(stderr, "%s: ERROR opening ephemeris file %s\n", command,
              ephemeris_file);
	  exit(1);
	}  
    
    int32 sd_id;
	int32 h_id;
	
	h_id = Hopen(l1b_hdf_file, DFACC_READ, 0);
	Vstart(h_id);
    
    sd_id = SDstart(l1b_hdf_file,DFACC_READ);    
    
    if( sd_id < 0 )
    {
      fprintf(stderr,"ERROR opening hdf file %s.\n",l1b_hdf_file);
      exit(1);
    }
    
    int num_l1b_frames = return_num_l1b_frames( sd_id );
    printf("num_l1b_frames: %d\n",num_l1b_frames);
    
    if( num_l1b_frames <= 0 )
    {
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
    
    int16* slice_lat       = (int16*)calloc(sizeof(int16),num_l1b_frames*100*8);
    int16* slice_lon       = (int16*)calloc(sizeof(int16),num_l1b_frames*100*8);
    int16* slice_sigma0    = (int16*)calloc(sizeof(int16),num_l1b_frames*100*8);
    int16* x_factor        = (int16*)calloc(sizeof(int16),num_l1b_frames*100*8);
    uint16* slice_azimuth  = (uint16*)calloc(sizeof(uint16),num_l1b_frames*100*8);
    int16* slice_incidence = (int16*)calloc(sizeof(int16),num_l1b_frames*100*8);
    int16* slice_snr       = (int16*)calloc(sizeof(int16),num_l1b_frames*100*8);
    
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
        !read_SDS( sd_id, "antenna_azimuth", &antenna_azimuth[0] ) )  // uint16
    {
      fprintf(stderr,"Error reading geolocation from HDF file!\n");
      exit(1);
    }     
    
    if( !read_SDS( sd_id, "slice_sigma0",     &slice_sigma0[0]    )  ||
        !read_SDS( sd_id, "x_factor",         &x_factor[0]        )  ||
        !read_SDS( sd_id, "slice_azimuth",    &slice_azimuth[0]   )  ||
        !read_SDS( sd_id, "slice_incidence",  &slice_incidence[0] )  ||
        !read_SDS( sd_id, "slice_snr",        &slice_snr[0]       )  ||
        !read_SDS( sd_id, "slice_lat",        &slice_lat[0]       )  || // uint16
        !read_SDS( sd_id, "slice_lon",        &slice_lon[0]       )  || // uint16        
        !read_SDS( sd_id, "slice_qual_flag",  &slice_qual_flag[0] ) )
    {
      fprintf(stderr,"Error reading slice quantities from HDF file!\n");
      exit(1);
    }  
    
    for( int i_frame = 0; i_frame < num_l1b_frames; ++i_frame )
    {
      // Read Frame times (put null in all chars since VSread won't null terminate
      // the chars that it reads).
      for ( int ii=0;ii<64;++ii)
        frame_time_str[ii] = NULL;
      
      if(!read_frame_time( h_id, i_frame, &frame_time_str[0] ))
      {
        fprintf(stderr,"error reading frame time at frame %d\n",i_frame);
        exit(1);
      }

      // Flush whatever is in the spotList object.
      l1b.frame.spotList.FreeContents();
      
      // Unix time is the epoch time for QSCATsim software.
      ETime etime;
      etime.FromCodeB("1970-001T00:00:00.000");
      double time_base = (double)etime.GetSec() + (double)etime.GetMs()/1000;
      
      if(!etime.FromCodeB(frame_time_str)) 
      {
        fprintf(stderr, "l1b_hdf_to_l1b: Error: could not parse time string: %s\n",
    	        frame_time_str);
    	exit(1);
      }
      
      double frame_time = (double)etime.GetSec() + (double)etime.GetMs()/1000 
                        - time_base;
      
      //printf("i_frame: %6d; frame_time: %s \n",i_frame,frame_time_str);
      
      for( int i_pulse = 0; i_pulse < 100; ++i_pulse )
      {
        
        MeasSpot* new_meas_spot = new MeasSpot();
        
        // Set time in meas_spot and in orbit state
        new_meas_spot->time              = frame_time;
        new_meas_spot->scOrbitState.time = frame_time;
        
        // Set ephemeris
        new_meas_spot->scOrbitState.rsat.Set( double(x_pos[i_frame]*1e-3), // convert to km
                                              double(y_pos[i_frame]*1e-3),
                                              double(z_pos[i_frame]*1e-3) );
                                              
        new_meas_spot->scOrbitState.vsat.Set( double(x_vel[i_frame]*1e-3), // convert to km/s
                                              double(y_vel[i_frame]*1e-3),
                                              double(z_vel[i_frame]*1e-3) );
        // Set attitude
        new_meas_spot->scAttitude.SetRPY( float(roll[i_frame] )*1e-3*dtr,
                                          float(pitch[i_frame])*1e-3*dtr,
                                          float(yaw[i_frame]  )*1e-3*dtr );
        
        for( int i_slice = 0; i_slice < 8; ++i_slice )
        {
           // Using 1d arrays 
           int pulse_ind = i_frame * 100     + i_pulse;
           int slice_ind = i_frame * 100 * 8 + i_pulse * 8 + i_slice;
           
           // Create unsigned ints for checking slice flags.
           unsigned int peak_gain_flag  = (unsigned int)(1 << (i_slice * 4 + 0) );
           unsigned int neg_sig0_flag   = (unsigned int)(1 << (i_slice * 4 + 1) );
           unsigned int low_snr_flag    = (unsigned int)(1 << (i_slice * 4 + 2) );
           unsigned int center_loc_flag = (unsigned int)(1 << (i_slice * 4 + 3) );
        
           // Skip observations with peak_gain_flag or center_loc_flag set.
           if( slice_qual_flag[pulse_ind] & peak_gain_flag ||
               slice_qual_flag[pulse_ind] & center_loc_flag )
             continue;
           
           Meas* new_meas = new Meas();
           
           // Set sigma0
           new_meas->value   = pow(10.0,0.01*double(slice_sigma0[slice_ind])/10.0);

           // Check for negative sigma-0
           if ( slice_qual_flag[pulse_ind] & neg_sig0_flag )  
             new_meas->value *= -1;

           // Set XK, EnSlice
           new_meas->XK      = pow(10.0,0.01*double(x_factor[slice_ind])/10.0);
           float tmp_snr     = pow(10.0,0.01*double(slice_snr[slice_ind])/10.0);
           new_meas->EnSlice = new_meas->value * new_meas->XK / tmp_snr;
        
           // Set lon, lat of observation
           double tmp_lat = 1e-4*dtr*double(slice_lat[slice_ind]);
           double tmp_lon = 1e-4*dtr*double(slice_lon[slice_ind]) 
                         / cos( cell_lat[pulse_ind]*dtr );
           
           tmp_lon += double(cell_lon[pulse_ind])*dtr;
           tmp_lat += double(cell_lat[pulse_ind])*dtr;
           
           new_meas->centroid.SetAltLonGDLat( 0.0, tmp_lon, tmp_lat );
           
           // Set inc angle
           new_meas->incidenceAngle = 0.01*dtr*double(slice_incidence[slice_ind]);
           
           // Set slice azimuth angle & change for QSCATsim convention
           float northAzimuth       = 0.01*dtr*double(slice_azimuth[slice_ind]);
           new_meas->eastAzimuth    = (450.0*dtr - northAzimuth);           
           if (new_meas->eastAzimuth >= two_pi) new_meas->eastAzimuth -= two_pi;
           
           // Set scanAngle
           new_meas->scanAngle = 0.01*dtr*double(antenna_azimuth[pulse_ind]);
           //if (new_meas->scanAngle < 0) new_meas->scanAngle += two_pi;
           
           // Set A,B,C kpc coef
           double nLooks = 10.0;
           double s0NE   = new_meas->EnSlice / new_meas->XK;
           
           new_meas->A = 1.0+1.0/nLooks;
           new_meas->B = fabs(2.0*s0NE/nLooks);
           new_meas->C = fabs(s0NE*s0NE/nLooks);
           
           // Set measurement type depending on inc angle
           if (new_meas->incidenceAngle < 50*dtr) 
           {
    	     new_meas->beamIdx = 0;
		     new_meas->measType = Meas::HH_MEAS_TYPE;
           }
           else
           {
    	     new_meas->beamIdx = 1;
    	     new_meas->measType = Meas::VV_MEAS_TYPE;
    	   }
           
           // Set startslice, numslices
           new_meas->numSlices     = -1; // as per BWS discussion.
           new_meas->startSliceIdx = START_SLICE_INDEX + i_slice;
           
           // startSliceIdx == [-4, -3, -2, -1, 1, 2, 3, 4] (no zero).
           // as per discussion with BWS.  Thus add one if(i_slice >= 0).
           if( new_meas->startSliceIdx >= 0 ) new_meas->startSliceIdx += 1;
           
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
      l1b.frame.frame_i              = i_frame;
      l1b.frame.num_l1b_frames       = num_l1b_frames;
      l1b.frame.num_pulses_per_frame = 100;
      l1b.frame.num_slices_per_pulse = 8;
      
      // Put in the land flag.
      landFlagL1BFrame(&(l1b.frame),&landmap);

      // Write this L1BFrame
      if( ! l1b.WriteDataRec() )
      {
        fprintf(stderr, "%s: writing to %s failed.\n", command, ephemeris_file );
        return (1);
      }
      
      if( i_frame % 100 == 0 )
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
    free(antenna_azimuth);
    free(cell_lat);
    free(cell_lon);
    free(slice_lat);
    free(slice_lon);
    free(slice_sigma0);
    free(x_factor);
    free(slice_azimuth);
    free(slice_incidence);
    free(slice_snr);
    
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
