//=========================================================//
// Copyright (C) 2009, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//


static const char rcs_id[] =
    "@(#) $Id$";

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include "Misc.h"
#include "L2A.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"
#include "Qscat.h"

//-----------//
// TEMPLATES //
//-----------//

template class List<EarthPosition>;
template class List<Meas>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;

int main( int argc, char* argv[] )
{
  const char* command = no_path(argv[0]);
  
  std::string l2a_input_file;
  std::string output_file;
  std::string satellite_tag;
  
  int l2a_input_file_entered = 0;
  int output_file_entered    = 0;
  int satellite_tag_entered  = 0;
  
  int optind = 1;
  while ( (optind < argc) && (argv[optind][0]=='-') )
  {
    std::string sw        = argv[optind];
    
    if( sw == "-i" )
    {
      ++optind;
      l2a_input_file         = argv[optind];
      l2a_input_file_entered = 1;
    }
    else if( sw == "-o" )
    {
      ++optind;
      output_file         = argv[optind];
      output_file_entered = 1;
    }
    else if( sw == "-s" )
    {
      ++optind;
      satellite_tag         = argv[optind];
      satellite_tag_entered = 1;
    }    
    else
    {
      printf("%s: unknown option\n",command);
      return (1);
    }
    ++optind;
  }
  
  if( l2a_input_file_entered == 0 || output_file_entered == 0 || satellite_tag_entered == 0 )
  {
    fprintf(stderr,"Usage: %s -i l2a_file -o output_file -s ( QSCAT || ASCAT || DFS )\n",
            command);
    exit(1);
  }
  
  
  if( satellite_tag != "ASCAT" && 
      satellite_tag != "QSCAT" &&
      satellite_tag != "DFS"      )
  {
    fprintf(stderr,"%s: ERROR Unknown satellite tag entered: %s !\n",
            command,satellite_tag.c_str());
    exit(1);
  }
  
  //------------------------//
  // open the Level 2A file //
  //------------------------//

  L2A l2a;
  if (! l2a.OpenForReading(l2a_input_file.c_str()))
  {
    fprintf(stderr, "%s: ERROR opening Level 2A file %s\n", command,
            l2a_input_file.c_str());
    exit(1);
  }
  
  FILE* output_fp = fopen(output_file.c_str(), "w");
  if (output_fp == NULL)
  {
    fprintf(stderr, "%s: ERROR opening output file %s\n", command,
            output_file.c_str());
    exit(1);
  }
  
  int NUM_BEAMS;
  if( satellite_tag == "ASCAT" ) NUM_BEAMS = 6;
  if( satellite_tag == "QSCAT" ) NUM_BEAMS = 2;
  if( satellite_tag == "DFS" )   NUM_BEAMS = 4;

//--Loop over the data records in the l2a file
  int i_rec = 0;
  while (l2a.ReadDataRec())
  {
    ++i_rec;
    MeasList* ml     = &(l2a.frame.measList);
    LonLat    lonlat = ml->AverageLonLat();
        
    //--Loop over the measlist and computes mean sigma0, var(sigma0),
    //-- And other stuff we may want.
    int    num_obs_beam[NUM_BEAMS];    
    double avg_sig0[NUM_BEAMS];
    double avg_sig0sq[NUM_BEAMS];
    double land_flag[NUM_BEAMS];
    double inc_ang[NUM_BEAMS];
    double along_beam_idx[NUM_BEAMS];
    
    // initialize these quantites
    for( int i_beam = 0; i_beam < NUM_BEAMS; ++i_beam )
    { 
      num_obs_beam[i_beam]   = 0;
      avg_sig0[i_beam]       = 0;
      avg_sig0sq[i_beam]     = 0;
      land_flag[i_beam]      = 0;
      inc_ang[i_beam]        = 0;
      along_beam_idx[i_beam] = 0;
    } 
    
    // Loop over the # of measurements at this wvc.
    for ( Meas* m = ml->GetHead(); m; m = ml->GetNext() )
    {
      int i_beam             = m->beamIdx;
      num_obs_beam[i_beam]   = num_obs_beam[i_beam]   + 1;
      avg_sig0[i_beam]       = avg_sig0[i_beam]       + m->value;
      avg_sig0sq[i_beam]     = avg_sig0sq[i_beam]     + pow(m->value,2);
      land_flag[i_beam]      = land_flag[i_beam]      + m->landFlag;
      inc_ang[i_beam]        = inc_ang[i_beam]        + m->incidenceAngle;
      along_beam_idx[i_beam] = along_beam_idx[i_beam] + m->startSliceIdx;
    }
    
    // Compute averages and variance of sigma0.
    for( int i_beam = 0; i_beam < NUM_BEAMS; ++i_beam )
    { 
      if( num_obs_beam[i_beam] > 0 )
      {
        avg_sig0[i_beam]       /= double( num_obs_beam[i_beam] );
        avg_sig0sq[i_beam]     /= double( num_obs_beam[i_beam] );
        land_flag[i_beam]      /= double( num_obs_beam[i_beam] );
        inc_ang[i_beam]        /= double( num_obs_beam[i_beam] );
        along_beam_idx[i_beam] /= double( num_obs_beam[i_beam] );
        
        double var_sig0 = avg_sig0sq[i_beam] - pow(avg_sig0[i_beam],2);
        
        fprintf(output_fp,"%d %d %8.5f %10.7f %20.17f %f %f\n",
                i_beam, num_obs_beam[i_beam],
                inc_ang[i_beam]*rtd, 10*log10(avg_sig0[i_beam]), 
                var_sig0, land_flag[i_beam], along_beam_idx[i_beam] );
      }
    }    
  }
  
  return 0;
}