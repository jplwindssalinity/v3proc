//==============================================================//
// Copyright (C) 2013, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    RS_GSE_merge.C
//
// SYNOPSIS
//    RS_GSE_merge -i filelist -o out_gse_file
//
// DESCRIPTION
//    Merges, sorts, and removed duplicate GSE data records.  filelist is 
//    and ASCII file containing the GSE files to process.  Optionally, one
//    can trim the GSE data to the desired time interval using the -tlims
//    option.
//
// OPTIONS
//
// OPERANDS
//
// EXAMPLES
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

static const char rcs_id[] =
    "@(#) $Id$";

#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <string>
#include "SwapEndian.h"
#include "ETime.h"

#define HEADER_BYTES 39
#define NHEAD        9
#define TT0_OFF      40
#define TT1_OFF      53
#define GPS2UTC_OFF  444

const char usage_string[] = "-i <ASCII file with list of gse files> -o <outfile> OR -od <outdir> [-tlims sim_time_min sim_time_max]";

struct GSE_TT_FILE_IDX {
  int           tt0;
  unsigned char tt1;
  double        gps_tt;
  size_t        off;
  int           packet_size;
  std::string   filename;
};

bool compare( const GSE_TT_FILE_IDX &gse_tt_idx0, const GSE_TT_FILE_IDX &gse_tt_idx1 ) {
  return( gse_tt_idx0.gps_tt < gse_tt_idx1.gps_tt );
}

bool same( const GSE_TT_FILE_IDX &gse_tt_idx0, const GSE_TT_FILE_IDX &gse_tt_idx1 ) {
  return( gse_tt_idx0.tt0==gse_tt_idx1.tt0 && gse_tt_idx0.tt1==gse_tt_idx1.tt1 );
}

int main( int argc, char* argv[] ) {
  
  // Really want these to be declared const, but I dunno how??
  // Valid GSE packet sizes (version 1 526 bytes; version 2 869 byte)
  // Version 2 for data after 2014-Feb-3rd (140203)
  std::vector<int> valid_packet_sizes;
  valid_packet_sizes.push_back(526);
  valid_packet_sizes.push_back(869);
  
  const char* command  = argv[0];
  char*       outfile  = NULL;
  char*       outdir   = NULL;
  char*       listfile = NULL;
  double      t_min    = -1;          // little number
  double      t_max    = pow(10,300); // big number
  
  int optind = 1;
  while ( (optind < argc) && (argv[optind][0]=='-') ) {
    std::string sw = argv[optind];
    if( sw=="-i" ) {
      listfile=argv[++optind];
    } else if ( sw=="-o" ) {
      outfile=argv[++optind];
    } else if ( sw=="-od" ) {
      outdir=argv[++optind];
    } else if ( sw=="-tlims" ) {
      t_min = atof(argv[++optind]);
      t_max = atof(argv[++optind]);
    } else {
      fprintf(stderr,"%s: %s\n",command,&usage_string[0]);
      exit(1);
    }
    ++optind;
  }
  
  // Only one of outfile and outdir may be commanded
  if( !listfile || ( !outfile == !outdir ) ) {
    fprintf(stderr,"%s: %s\n",command,&usage_string[0]);
    exit(1);
  }
  
  ETime etime;
  etime.FromCodeB("1970-001T00:00:00.000");
  double sim_time_base = (double)etime.GetSec() + (double)etime.GetMs()/1000;
  
  etime.FromCodeB("1980-006T00:00:00.000");
  double gps_time_base = (double)etime.GetSec() + (double)etime.GetMs()/1000;
  
  std::vector< std::string > infiles;
  
  FILE* ifp_list = fopen(listfile,"r");
  char line[1024], filename[1024];
  while(fgets(line,sizeof(line),ifp_list)!=NULL) {
    sscanf( line, "%s", filename );
    std::string this_infile = filename;
    infiles.push_back( this_infile );
    //printf("%s\n",this_infile.c_str());
  }
  fclose(ifp_list);
  
  //printf("%f %f\n",t_min,t_max);
  
  //printf("%d\n",infiles.size());
  
  std::vector< GSE_TT_FILE_IDX > gse_tt_idx_list;
  
  size_t total_gse_packets=0;
  
  for( size_t ifile=0;ifile<infiles.size();++ifile) {
    FILE* ifp = fopen(infiles[ifile].c_str(),"r");
    fseek(ifp,0,SEEK_END);
    size_t file_size = ftell(ifp);
    size_t this_off  = 0;
    
    // Scan through file
    while( this_off+valid_packet_sizes[0] <= file_size ) {
      unsigned char gse_headers[HEADER_BYTES];
      
      fseek(ifp,this_off,SEEK_SET);
      fread(&gse_headers,sizeof(unsigned char),HEADER_BYTES,ifp);
      
      // Extract packet length header, should always be PACKET_SIZE-HEADER_BYTES
      // Table 8.6.3.1-1 in Document 50305D
      short packet_length = (short)gse_headers[36]*256 + (short)gse_headers[37];
      
      // Is this a valid packet size (compare to known sizes)
      int valid_length = 0;
      for(int ii=0; ii<valid_packet_sizes.size(); ++ii) {
        if(packet_length==valid_packet_sizes[ii]-HEADER_BYTES)
          valid_length=1;
      }
      
      int tt0;
      fseek(ifp,this_off+TT0_OFF+NHEAD-1,SEEK_SET);
      fread( &tt0, sizeof(int), 1, ifp ); 
      SWAP_VAR( tt0, int );
      
      unsigned char tt1;
      fseek(ifp,this_off+TT1_OFF+NHEAD-1,SEEK_SET);
      fread( &tt1, sizeof(unsigned char), 1, ifp ); 
      
      short gps2utc;
      fseek(ifp,this_off+GPS2UTC_OFF+NHEAD-1,SEEK_SET);
      fread( &gps2utc, sizeof(short), 1, ifp );
      SWAP_VAR( gps2utc, short );
      
      double gps_tt = (double)tt0 + (double)tt1/255.0;
      
      // Check if this is a valid GSE packet starting at this byte offset
      // See Document 50305D, Table 8.1.1-1 Primary EHS protocol header format
      // Byte 0 indicates version of EHS protocol and project ID
      // Byte 3 indicates content of EHS protocol data (GSE==6).
      if( gse_headers[0]==35 && gse_headers[3]==6  && 
          valid_length==1 && gps_tt>0 && gps2utc < 0 ) {
        // Add to list of packets
        GSE_TT_FILE_IDX this_gse_tt_file_idx;
        this_gse_tt_file_idx.tt0         = tt0;
        this_gse_tt_file_idx.tt1         = tt1;
        this_gse_tt_file_idx.gps_tt      = gps_tt;
        this_gse_tt_file_idx.off         = this_off;
        this_gse_tt_file_idx.packet_size = packet_length+HEADER_BYTES;
        this_gse_tt_file_idx.filename    = infiles[ifile];
        
        // check if this packet is within the time bounds
        double t_packet = (double)(gps_tt+gps2utc) + gps_time_base - sim_time_base;
        
        if( t_packet >= t_min && t_packet<=t_max ) 
          gse_tt_idx_list.push_back( this_gse_tt_file_idx );
        
        total_gse_packets += 1;
        this_off          += this_gse_tt_file_idx.packet_size;
      } else {
        // shift one byte and try again.
        this_off++;
      }
    }
    fclose(ifp);
    //printf("%s: GSE packets: %zd\n",infiles[ifile].c_str(),gse_tt_idx_list.size());
  }

  std::vector<GSE_TT_FILE_IDX>::iterator it;
  // Sort and remove duplicates 
  // (http://www.cplusplus.com/reference/algorithm/)
  std::sort(        gse_tt_idx_list.begin(), gse_tt_idx_list.end(), compare );
  it = std::unique( gse_tt_idx_list.begin(), gse_tt_idx_list.end(), same    );
  
  gse_tt_idx_list.resize( std::distance(gse_tt_idx_list.begin(),it) );
  
  //printf("Total GSE packets: %zd\n",total_gse_packets);
  //printf("Unique GSE packets: %zd\n",gse_tt_idx_list.size());
  
  if( gse_tt_idx_list.size()==0) {
    printf("No packets to write, quitting\n");
    exit(0);
  }
  
  // Determine time strings for 1st, last GSE packet in file, and current UTC time.
  short gps2utc;
  
  // get iterator to first element in vector
  it = gse_tt_idx_list.begin();
  FILE* ifp = fopen(it->filename.c_str(),"r");
  fseek(ifp,it->off+GPS2UTC_OFF+NHEAD-1,SEEK_SET);
  fread( &gps2utc, sizeof(short), 1, ifp );
  fclose(ifp);
  SWAP_VAR( gps2utc, short );
  
  double t_start = (double)(it->gps_tt+gps2utc) + gps_time_base - sim_time_base;
  
  // get iterator to last element in vector
  it = gse_tt_idx_list.end(); --it;
  ifp = fopen(it->filename.c_str(),"r");
  fseek(ifp,it->off+GPS2UTC_OFF+NHEAD-1,SEEK_SET);
  fread( &gps2utc, sizeof(short), 1, ifp );
  fclose(ifp);
  SWAP_VAR( gps2utc, short );
  double t_end = (double)(it->gps_tt+gps2utc) + gps_time_base - sim_time_base;
  
  char str_t_start[BLOCK_B_TIME_LENGTH];
  char str_t_end[BLOCK_B_TIME_LENGTH];
  char str_t_now[BLOCK_B_TIME_LENGTH];
  
  etime.SetTime(t_start);
  etime.ToBlockB(str_t_start);

  etime.SetTime(t_end);
  etime.ToBlockB(str_t_end);

  etime.CurrentTime();
  etime.ToBlockB(str_t_now);
  
//   printf("%s to %s; %s\n",str_t_start,str_t_end,str_t_now);
  
  // Generate the output file name dynmaically if only supplied the output directory
  if ( outdir ) {
    outfile = new char[1024];
    sprintf(outfile,"%s/RS_GSE_%s-%s.%s",outdir,str_t_start,str_t_end,str_t_now);
  }
  
  // Write out unique GSE packets in all files
  FILE* ofp = fopen(outfile,"w");
  for( it=gse_tt_idx_list.begin(); it != gse_tt_idx_list.end(); ++it) {
    char gse_packet[it->packet_size];
    ifp = fopen(it->filename.c_str(),"r");
    fseek(ifp,it->off,SEEK_SET);
    fread(&gse_packet,sizeof(char),it->packet_size,ifp);
    fwrite(&gse_packet,sizeof(char),it->packet_size,ofp);
    fclose(ifp);
  }
  fclose(ofp);
  
  if ( outdir != NULL ) delete[] outfile;
  
  return(0);
};

