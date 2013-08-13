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
//    and ASCII file containing the GSE files to process.
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
#include <vector>
#include <algorithm>
#include <list>
#include <string>
#include "SwapEndian.h"

#define PACKET_SIZE  526
#define NHEAD        9
#define TT0_OFF      40
#define GPS2UTC_OFF  444

const char usage_string[] = "-i <ASCII file with list of gse files> -o <outfile>";

struct GSE_TT_FILE_IDX {
  int    gps_tt;
  size_t off;
  FILE*  ifp;
};

bool compare( const GSE_TT_FILE_IDX &gse_tt_idx0, const GSE_TT_FILE_IDX &gse_tt_idx1 ) {
  return( gse_tt_idx0.gps_tt < gse_tt_idx1.gps_tt );
}

bool same( const GSE_TT_FILE_IDX &gse_tt_idx0, const GSE_TT_FILE_IDX &gse_tt_idx1 ) {
  return( gse_tt_idx0.gps_tt==gse_tt_idx1.gps_tt );
}


int main( int argc, char* argv[] ) {
  
  const char* command  = argv[0];
  char*       outfile  = NULL;
  char*       listfile = NULL;
  
  int optind = 1;
  while ( (optind < argc) && (argv[optind][0]=='-') ) {
    std::string sw = argv[optind];
    if( sw=="-i" ) {
      listfile=argv[++optind];
    } else if ( sw=="-o" ) {
      outfile=argv[++optind];
    } else {
      fprintf(stderr,"%s: %s\n",command,&usage_string[0]);
      exit(1);
    }
    ++optind;
  }
  
  if( !listfile || !outfile ) {
    fprintf(stderr,"%s: %s\n",command,&usage_string[0]);
    exit(1);
  }
  
  std::vector< std::string > infiles;
  
  FILE* ifp = fopen(listfile,"r");
  while( !feof(ifp) ) {
    char line[1024];
    fscanf( ifp, "%s", line );
    std::string this_infile = line;
    infiles.push_back( this_infile );
    //printf("%s\n",this_infile.c_str());
  }
  fclose(ifp);
  
  FILE* ifps[infiles.size()];
  
  //printf("%d\n",infiles.size());
  
  std::vector< GSE_TT_FILE_IDX > gse_tt_idx_list;
  
  size_t total_gse_packets=0;
  
  for( size_t ifile=0;ifile<infiles.size();++ifile) {
    ifps[ifile] = fopen(infiles[ifile].c_str(),"r");
    
    fseek(ifps[ifile],0,SEEK_END);
    size_t file_size = ftell(ifps[ifile]);
    size_t this_off  = 0;
    
    // Scan through file
    while( this_off+PACKET_SIZE <= file_size ) {
      char first_four_bytes[4];
      
      fseek(ifps[ifile],this_off,SEEK_SET);
      fread(&first_four_bytes,sizeof(char),4,ifps[ifile]);
      
      int gps_tt;
      fseek(ifps[ifile],this_off+TT0_OFF+NHEAD-1,SEEK_SET);
      fread( &gps_tt, sizeof(int), 1, ifps[ifile] ); 
      SWAP_VAR( gps_tt, int );
      
      short gps2utc;
      fseek(ifps[ifile],this_off+GPS2UTC_OFF+NHEAD-1,SEEK_SET);
      fread( &gps2utc, sizeof(short), 1, ifps[ifile] );
      SWAP_VAR( gps2utc, short );
      
      // Check if this is a valid GSE packet starting at this byte offset
      // See Document 50305D, Table 8.1.1-1 Primary EHS protocol header format
      // Byte 0 indicates version of EHS protocol and project ID
      // Byte 3 indicates content of EHS protocol data (GSE==6).
      if( first_four_bytes[0]==35 && first_four_bytes[3]==6  &&
          gps_tt>0 && gps2utc < 0) {
        // if valid, get gps time-tag for sorting and extracting unique packets.
        
        
        // Add to list of packets
        GSE_TT_FILE_IDX this_gse_tt_file_idx;
        this_gse_tt_file_idx.gps_tt = gps_tt;
        this_gse_tt_file_idx.off    = this_off;
        this_gse_tt_file_idx.ifp    = ifps[ifile];
        gse_tt_idx_list.push_back( this_gse_tt_file_idx );
        
        total_gse_packets += 1;
        this_off          += PACKET_SIZE;
      } else {
        // shift one byte and try again.
        this_off++;
      }
    }
  }
  
  std::vector<GSE_TT_FILE_IDX>::iterator it;
  
  // Sort and remove duplicates
  std::stable_sort( gse_tt_idx_list.begin(), gse_tt_idx_list.end(), compare );
  it = std::unique( gse_tt_idx_list.begin(), gse_tt_idx_list.end(), same );
  
  gse_tt_idx_list.resize( std::distance(gse_tt_idx_list.begin(),it) );
  
  printf("Total GSE packets: %zd\n",total_gse_packets);  
  printf("Unique GSE packets: %zd\n",gse_tt_idx_list.size());
  
  // Write out unique GSE packets in all files
  FILE* ofp = fopen(outfile,"w");
  
  for( it=gse_tt_idx_list.begin(); it != gse_tt_idx_list.end(); ++it) {
    char gse_packet[PACKET_SIZE];    
    fseek(it->ifp,it->off,SEEK_SET);
    fread(&gse_packet,sizeof(char),PACKET_SIZE,it->ifp);
    fwrite(&gse_packet,sizeof(char),PACKET_SIZE,ofp);
  }
  
  fclose(ofp);
  
  for( size_t ifile=0;ifile<infiles.size();++ifile) {
    fclose(ifps[ifile]);
  }
  
  return(0);
};

