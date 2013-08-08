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
#include <list>
#include <string>
#include "SwapEndian.h"

#define PACKET_SIZE  526
#define NHEAD        9
#define TT0_OFF      40

const char usage_string[] = "-i <ASCII file with list of gse files> -o <outfile>";

struct GSE_TT_FILE_IDX {
  int   gps_tt;
  int   idx;
  FILE* ifp;
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
  
  std::list< GSE_TT_FILE_IDX > gse_tt_idx_list;
  
  size_t total_gse_packets=0;
  
  for( size_t ifile=0;ifile<infiles.size();++ifile) {
    ifps[ifile] = fopen(infiles[ifile].c_str(),"r");
    
    // Scan through file
    fseek(ifps[ifile],0,SEEK_END);
    size_t n_packets = ftell(ifps[ifile]) / PACKET_SIZE;
    
    for( size_t ipacket=0;ipacket<n_packets;++ipacket) {
      int   gps_tt;
      long int packet_off = ipacket*PACKET_SIZE;
      fseek(ifps[ifile],packet_off+TT0_OFF+NHEAD-1,SEEK_SET);
      fread( &gps_tt, sizeof(int), 1, ifps[ifile] ); 
      SWAP_VAR( gps_tt, int );
      
      GSE_TT_FILE_IDX this_gse_tt_file_idx;
      this_gse_tt_file_idx.gps_tt = gps_tt;
      this_gse_tt_file_idx.idx    = ipacket;
      this_gse_tt_file_idx.ifp    = ifps[ifile];
      gse_tt_idx_list.push_back( this_gse_tt_file_idx );
    }
    total_gse_packets += n_packets;
    
    // Sort and remove duplicates
    gse_tt_idx_list.sort( &compare );
    gse_tt_idx_list.unique( &same );
  }
  
  printf("Total GSE packets: %zd\n",total_gse_packets);  
  printf("Unique GSE packets: %zd\n",gse_tt_idx_list.size());
  
  // Write out unique GSE packets in all files
  FILE* ofp = fopen(outfile,"w");
  
  for( std::list<GSE_TT_FILE_IDX>::iterator it=gse_tt_idx_list.begin();
       it != gse_tt_idx_list.end(); ++it) {
    char gse_packet[PACKET_SIZE];
    long int packet_off = it->idx*PACKET_SIZE;
    fseek(it->ifp,packet_off,SEEK_SET);
    fread(&gse_packet,sizeof(char),PACKET_SIZE,it->ifp);
    fwrite(&gse_packet,sizeof(char),PACKET_SIZE,ofp);
  }
  
  fclose(ofp);
  
  for( size_t ifile=0;ifile<infiles.size();++ifile) {
    fclose(ifps[ifile]);
  }
  
  return(0);
};

