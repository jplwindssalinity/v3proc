//==============================================================//
// Copyright (C) 2013, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    generate_rgc
//
// SYNOPSIS
//    generate_rgc_from_ephem_quat_files [ -cf -s rev ] <config_file> <RGC_base>
//
// DESCRIPTION
//    Generates a set of Receiver Gate Constants for each beam based
//    upon the parameters in the simulation configuration file.
//
// OPTIONS
//      [ -c ]  Clean the constants.
//      [ -f ]  Make the range fixed over azimuth.
//      [ -s i ] start with rev i in ephem / quat files
//
// OPERANDS
//    The following operands are supported:
//      <config_file>  The config_file needed listing
//                     all input parameters.
//
//      <RGC_base>     The RGC output base.
//
// EXAMPLES
//    An example of a command line is:
//      % generate_constants sws1b.cfg rgc.dat
//
// ENVIRONMENT
//    Not environment dependent.
//
// EXIT STATUS
//    The following exit values are returned:
//       0   Program executed successfully
//      >0  Program had an error
//
// NOTES
//    None.
//
// AUTHOR
//    Alex Fore
//    alexander.fore@jpl.nasa.gov
//----------------------------------------------------------------------

//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
    "@(#) $Id$";

#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <list>
#include <string>
#include "Misc.h"
#include "ETime.h"
#include "Matrix3.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "EarthPosition.h"
#include "GenericGeom.h"
#include "Quat.h"
#include "SwapEndian.h"

//-----------//
// TEMPLATES //
//-----------//
template class List<EarthPosition>;
template class BufferedList<QuatRec>;
template class List<QuatRec>;
//-----------//
// CONSTANTS //
//-----------//
#define PACKET_SIZE  526
#define NHEAD        9
#define TT0_OFF      40
#define TT1_OFF      53
#define POSX_OFF     63
#define POSY_OFF     76
#define POSZ_OFF     89
#define VELX_OFF     102
#define VELY_OFF     115
#define VELZ_OFF     128
#define QUAT0_OFF    362
#define QUAT1_OFF    375
#define QUAT2_OFF    388
#define QUAT3_OFF    401
#define GPS2UTC_OFF  444

const char usage_string[] = "-i gsefile -e out_ephem_file -q out_quat_file";

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

//--------------//
// MAIN PROGRAM //
//--------------//

struct GSE_TT_IDX {
  int tt0;
  int idx;
};

bool compare( const GSE_TT_IDX &gse_tt_idx0, const GSE_TT_IDX &gse_tt_idx1 ) {
  return( gse_tt_idx0.tt0 < gse_tt_idx1.tt0 );
}

bool same( const GSE_TT_IDX &gse_tt_idx0, const GSE_TT_IDX &gse_tt_idx1 ) {
  return( gse_tt_idx0.tt0==gse_tt_idx1.tt0 );
}

int
main(
    int    argc,
    char*  argv[]) {

  const char* command        = no_path(argv[0]);
  char*       gse_file       = NULL; // GSE telemetry file
  char*       out_ephem_file = NULL;
  char*       out_quat_file  = NULL;
  
  int optind = 1;
  while ( (optind < argc) && (argv[optind][0]=='-') ) {
    std::string sw = argv[optind];
    if( sw=="-i" ) {
      gse_file=argv[++optind];
    } else if ( sw=="-e" ) {
      out_ephem_file=argv[++optind];
    } else if ( sw=="-q" ) {
      out_quat_file=argv[++optind];
    } else {
      fprintf(stderr,"%s: %s\n",command,&usage_string);
      exit(1);
    }
    ++optind;
  }
  
  if( !gse_file || !out_ephem_file || !out_quat_file ) {
    fprintf(stderr,"%s: %s\n",command,&usage_string);
    exit(1);
  }
  
  char quat_geocfile[1000];
  sprintf(quat_geocfile,"%s_geoc",out_quat_file);
  
  FILE* ifp    = fopen(gse_file,"r");
  FILE* ofp_e  = fopen(out_ephem_file,"w");
  FILE* ofp_q  = fopen(out_quat_file,"w");
  FILE* ofp_qc = fopen(quat_geocfile,"w");
  
  fseek(ifp,0,SEEK_END);
  long int gse_size  = ftell(ifp);
  long int n_packets = gse_size / PACKET_SIZE;
  
  if( n_packets*PACKET_SIZE != gse_size ) {
    fprintf(stderr,"%s: something is not right with GSE file: %s\n",command,gse_file);
    exit(1);
  }
  
  // Read in all GPS second time-tags fron GSE file
  std::list< GSE_TT_IDX > gse_tt_idx_list;
  for( int ipacket=0;ipacket<n_packets;++ipacket) {
    int   tt0;
    long int packet_off = ipacket*PACKET_SIZE;
    fseek(ifp,packet_off+TT0_OFF+NHEAD-1,SEEK_SET);
    fread( &tt0, sizeof(int), 1, ifp ); 
    SWAP_VAR( tt0, int );
    
    GSE_TT_IDX this_gse_tt_idx;
    this_gse_tt_idx.tt0 = tt0;
    this_gse_tt_idx.idx = ipacket;
    
    gse_tt_idx_list.push_back( this_gse_tt_idx );
  }
  
  // Sort them and remove duplicates
  gse_tt_idx_list.sort( &compare );
  gse_tt_idx_list.unique( &same );
  
  ETime etime;
  etime.FromCodeB("1970-001T00:00:00.000");
  double sim_time_base = (double)etime.GetSec() + (double)etime.GetMs()/1000;
  
  etime.FromCodeB("1980-006T00:00:00.000");
  double gps_time_base = (double)etime.GetSec() + (double)etime.GetMs()/1000;
  
  // Loop over the unique, sorted GPS time-tags in GSE file
  for( std::list<GSE_TT_IDX>::iterator it=gse_tt_idx_list.begin();
       it != gse_tt_idx_list.end(); ++it) {
    
    // Get the index in the GSE file for this time-tag
    int this_packet_index = it->idx;
    
    int   tt0;
    char  tt1;
    float pos[3];
    float vel[3];
    float quat[4];
    short gps2utc;
    
    long int packet_off = this_packet_index*PACKET_SIZE;
    
    fseek(ifp,packet_off+TT0_OFF+NHEAD-1,SEEK_SET);
    fread( &tt0, sizeof(int), 1, ifp ); 
    SWAP_VAR( tt0, int );
    
    fseek(ifp,packet_off+TT1_OFF+NHEAD-1,SEEK_SET);
    fread( &tt1, sizeof(char), 1, ifp );
    
    fseek(ifp,packet_off+POSX_OFF+NHEAD-1,SEEK_SET);
    fread( &pos[0], sizeof(float), 1, ifp );
    SWAP_VAR( pos[0], float );

    fseek(ifp,packet_off+POSY_OFF+NHEAD-1,SEEK_SET);
    fread( &pos[1], sizeof(float), 1, ifp );
    SWAP_VAR( pos[1], float );

    fseek(ifp,packet_off+POSZ_OFF+NHEAD-1,SEEK_SET);
    fread( &pos[2], sizeof(float), 1, ifp );
    SWAP_VAR( pos[2], float );

    fseek(ifp,packet_off+VELX_OFF+NHEAD-1,SEEK_SET);
    fread( &vel[0], sizeof(float), 1, ifp );
    SWAP_VAR( vel[0], float );

    fseek(ifp,packet_off+VELY_OFF+NHEAD-1,SEEK_SET);
    fread( &vel[1], sizeof(float), 1, ifp );
    SWAP_VAR( vel[1], float );

    fseek(ifp,packet_off+VELZ_OFF+NHEAD-1,SEEK_SET);
    fread( &vel[2], sizeof(float), 1, ifp );
    SWAP_VAR( vel[2], float );
    
    fseek(ifp,packet_off+QUAT0_OFF+NHEAD-1,SEEK_SET);
    fread( &quat[0], sizeof(float), 1, ifp );
    SWAP_VAR( quat[0], float );

    fseek(ifp,packet_off+QUAT1_OFF+NHEAD-1,SEEK_SET);
    fread( &quat[1], sizeof(float), 1, ifp );
    SWAP_VAR( quat[1], float );

    fseek(ifp,packet_off+QUAT2_OFF+NHEAD-1,SEEK_SET);
    fread( &quat[2], sizeof(float), 1, ifp );
    SWAP_VAR( quat[2], float );

    fseek(ifp,packet_off+QUAT3_OFF+NHEAD-1,SEEK_SET);
    fread( &quat[3], sizeof(float), 1, ifp );
    SWAP_VAR( quat[3], float );

    fseek(ifp,packet_off+GPS2UTC_OFF+NHEAD-1,SEEK_SET);
    fread( &gps2utc, sizeof(short), 1, ifp );
    SWAP_VAR( gps2utc, short );

    double time = double(tt0); //+ double(tt1)/255.0;
    time       += double(gps2utc) + gps_time_base - sim_time_base;
    
    // Convert to meters, meters / second from feet, feet / second.
    for( int ii=0;ii<3;++ii) {
      pos[ii] *= 0.3048;
      vel[ii] *= 0.3048;
    }
    
    // Still need to adjust for epoch for the GSE inertial reference frame!
    // EarthPosition objects use kilometers, kilometers / second units.
    EarthPosition rsat;
    Vector3       vsat;
    rsat.Set( (double)pos[0]*1E-3, (double)pos[1]*1E-3, (double)pos[2]*1E-3 );
    vsat.Set( (double)vel[0]*1E-3, (double)vel[1]*1E-3, (double)vel[2]*1E-3 );
    
    Vector3 xscvel_geoc, yscvel_geoc, zscvel_geoc;
    Vector3 xscvel_geod, yscvel_geod, zscvel_geod;
    
    velocity_frame_geocentric( rsat, vsat, &xscvel_geoc, &yscvel_geoc, &zscvel_geoc, time );
    velocity_frame_geodetic(   rsat, vsat, &xscvel_geod, &yscvel_geod, &zscvel_geod, time );
    
    // Make them unit vectors
    xscvel_geoc.Scale(1.0);
    yscvel_geoc.Scale(1.0);
    zscvel_geoc.Scale(1.0);
    
    xscvel_geod.Scale(1.0);
    yscvel_geod.Scale(1.0);
    zscvel_geod.Scale(1.0);
    
    // m Transforms from geocentric to geodetic local reference frames
    double m[3][3];
    
    m[0][0] = xscvel_geod % xscvel_geoc;
    m[0][1] = xscvel_geod % yscvel_geoc;
    m[0][2] = xscvel_geod % zscvel_geoc;
    
    m[1][0] = yscvel_geod % xscvel_geoc;
    m[1][1] = yscvel_geod % yscvel_geoc;
    m[1][2] = yscvel_geod % zscvel_geoc;
    
    m[2][0] = zscvel_geod % xscvel_geoc;
    m[2][1] = zscvel_geod % yscvel_geoc;
    m[2][2] = zscvel_geod % zscvel_geoc;
    
//    printf("matrix: \n");
//    for( int ii=0;ii<3;++ii) {
//      printf("%12.6e %12.6e %12.6e\n",m[ii][0],m[ii][1],m[ii][2]);
//    }
    
    Quat q_geoc_to_geod;
    q_geoc_to_geod.QuatFromMatrix( &m[0][0] );
    
//    printf("Quat: %f %f %f %f\n",
//           q_geoc_to_geod.w, q_geoc_to_geod.x, 
//           q_geoc_to_geod.y, q_geoc_to_geod.z );
    
    
    // contructor wants x,y,z,w order (different from QuatFile and GSE data...).
    Quat q_att_geoc( (double)quat[1], (double)quat[2], (double)quat[3], (double)quat[0] );
    Quat q_all = q_geoc_to_geod * q_att_geoc;
    
    q_all.Normalize();
    
    double out_pos[3], out_vel[3];
    for( int ii=0;ii<3;++ii) {
      out_pos[ii] = pos[ii];
      out_vel[ii] = vel[ii];
    }
    
    fwrite(&time,sizeof(double),1,ofp_e);
    fwrite(&out_pos[0],sizeof(double),3,ofp_e);
    fwrite(&out_vel[0],sizeof(double),3,ofp_e);
    
    fwrite(&time,sizeof(double),1,ofp_q);
    fwrite(&(q_all.w),sizeof(double),1,ofp_q);
    fwrite(&(q_all.x),sizeof(double),1,ofp_q);
    fwrite(&(q_all.y),sizeof(double),1,ofp_q);
    fwrite(&(q_all.z),sizeof(double),1,ofp_q);
    
    fwrite(&time,sizeof(double),1,ofp_qc);
    fwrite(&(q_att_geoc.w),sizeof(double),1,ofp_qc);
    fwrite(&(q_att_geoc.x),sizeof(double),1,ofp_qc);
    fwrite(&(q_att_geoc.y),sizeof(double),1,ofp_qc);
    fwrite(&(q_att_geoc.z),sizeof(double),1,ofp_qc);
  }
  
  fclose(ifp);
  fclose(ofp_e);
  fclose(ofp_q);
  
  return(0);
}

