//==============================================================//
// Copyright (C) 2013, California Institute of Technology.      //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    xtable_to_gs.C
//
// SYNOPSIS
//    xtable_to_gs -i sim_xtable -o gs_xtable -m mode_index
//
// DESCRIPTION
//    Converts a sim Xtable to the GS format, assuming user enteres mode_index correctly
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
//       0  Program executed successfully
//      >0  Program had an error
//
// NOTES
//
// AUTHORS
//    Alex Fore
//----------------------------------------------------------------------

//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] = "@(#) $Id$";

//----------//
// INCLUDES //
//----------//
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <string>
#include "XTable.h"
#include "Misc.h"
#include "Constants.h"
//-----------//
// TEMPLATES //
//-----------//

//-----------//
// CONSTANTS //
//-----------//

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
const char usage_string[] = "-i sim_xtable -o gs_xtable -m mode_index";

//--------------//
// MAIN PROGRAM //
//--------------//

int main( int argc, char* argv[] ) {
  //------------------------//
  // parse the command line //
  //------------------------//
  
  const char* command    = no_path(argv[0]);
  char*       infile     = NULL;
  char*       outfile    = NULL;
  int         mode_index = 0;
  
  int optind    = 1;
  while( (optind < argc) && (argv[optind][0]=='-') ) {
    std::string sw = argv[optind];
    if( sw == "-i" ) {
      infile = argv[++optind];
    } else if( sw == "-o" ) {
      outfile = argv[++optind];
    } else if( sw == "-m" ) {
      mode_index = atoi(argv[++optind]);
    } else {
      fprintf(stderr,"%s: %s\n",command,&usage_string[0]);
      exit(1);
    }
    ++optind;
  }
  
  if( !infile || !outfile || mode_index <1 || mode_index >8 ) {
    fprintf(stderr,"%s: %s\n",command,&usage_string[0]);
    exit(1);
  }
  
  XTable x_table;
  if( !x_table.SetFilename( infile ) || !x_table.Read() ) {
    fprintf(stderr,"%s: Error reading XTable from: %s\n",command,infile);
    exit(1);
  }
  
  int n_orb = x_table.numOrbitPositionBins;
  int n_azi = x_table.numAzimuthBins;
  
  if (x_table.numScienceSlices!=10 || n_orb!=32 || n_azi!=36) {
    fprintf(stderr,"%s: GS format requires 10 sci slices, n_azi==36, and n_orb=32!\n",command);
    fprintf(stderr,"%d %d %d\n",x_table.numScienceSlices,x_table.numAzimuthBins, 
            x_table.numOrbitPositionBins);
    exit(1);
  }
  
  FILE* ofp = fopen(outfile,"w");
  if(!ofp) {
    fprintf(stderr,"%s: Error opening file: %s\n",command,outfile);
    exit(1);
  }
  
  // Write fortran header 
  int nbytes = 6414464; 
  if( fwrite(&nbytes,sizeof(int),1,ofp) != 1 ) {
    fprintf(stderr,"%s: Error writing x table to file: %s\n",command,outfile);
    fclose(ofp);
    exit(1);
  }
  
  // Slice shift threshold table
  float slice_shift_thresh[8][2][2];
    
  slice_shift_thresh[0][0][0] = -1425.000000;
  slice_shift_thresh[0][0][1] = -975.000000;
  slice_shift_thresh[0][1][0] = -800.000000;
  slice_shift_thresh[0][1][1] = -800.000000;

  slice_shift_thresh[1][0][0] = -2325.000000;
  slice_shift_thresh[1][0][1] = -25.000000;
  slice_shift_thresh[1][1][0] = -1925.000000;
  slice_shift_thresh[1][1][1] = 425.000000;

  slice_shift_thresh[2][0][0] = -3525.000000;
  slice_shift_thresh[2][0][1] = 1125.000000;
  slice_shift_thresh[2][1][0] = -2625.000000;
  slice_shift_thresh[2][1][1] = 1975.000000;

  slice_shift_thresh[3][0][0] = -4675.000000;
  slice_shift_thresh[3][0][1] = 2225.000000;
  slice_shift_thresh[3][1][0] = -2925.000000;
  slice_shift_thresh[3][1][1] = 3925.000000;
  
  slice_shift_thresh[4][0][0] = -8075.000000;
  slice_shift_thresh[4][0][1] = 4825.000000;
  slice_shift_thresh[4][1][0] = -3625.000000;
  slice_shift_thresh[4][1][1] = 8925.000000;

  slice_shift_thresh[5][0][0] = -5525.000000;
  slice_shift_thresh[5][0][1] = 2975.000000;
  slice_shift_thresh[5][1][0] = -3225.000000;
  slice_shift_thresh[5][1][1] = 5075.000000;

  slice_shift_thresh[6][0][0] = -6625.000000;
  slice_shift_thresh[6][0][1] = 3875.000000;
  slice_shift_thresh[6][1][0] = -3675.000000;
  slice_shift_thresh[6][1][1] = 6825.000000;

  slice_shift_thresh[7][0][0] = -5525.000000;
  slice_shift_thresh[7][0][1] = 2975.000000;
  slice_shift_thresh[7][1][0] = -3225.000000;
  slice_shift_thresh[7][1][1] = 5075.000000;
  
  
  if( fwrite(&slice_shift_thresh[0][0][0],sizeof(float),8*2*2,ofp) != 8*2*2 ) {
    fprintf(stderr,"%s: Error writing x table to file: %s\n",command,outfile);
    fclose(ofp);
    exit(1);  
  }
  
  // Write X nom
  for( int i_mode = 1; i_mode <= 8; ++i_mode ) {
    for( int i_orb = 0; i_orb < n_orb; ++i_orb ) {
      float this_orb = ((double)i_orb+0.5)/(double)n_orb;
      for( int i_azi = 0; i_azi < n_azi; ++i_azi ) {
        float this_azi = (double)i_azi/(double)n_azi * two_pi;
        for( int i_beam = 0; i_beam < 2; ++i_beam ) {
          
          float slice_x_nom[10];
          float egg_x_nom = 0;
          
          for( int i_slice = 0; i_slice < 10; ++i_slice )
            slice_x_nom[i_slice] = 0;
          
          if( i_mode == mode_index ) {
            // Copy from sim Xtable
            for( int i_slice = 0; i_slice < 10; ++i_slice ) {
              
              slice_x_nom[i_slice] = x_table.RetrieveBySliceNumber( i_beam, this_azi, 
                                                                    this_orb, i_slice);
              egg_x_nom += slice_x_nom[i_slice];
            }
          } 
          if( fwrite(&slice_x_nom[0],sizeof(float),10,ofp) != 10 ||
              fwrite(&egg_x_nom,sizeof(float),1,ofp)       != 1 ) {
            fprintf(stderr,"%s: Error writing x table to file: %s\n",command,outfile);
            fclose(ofp);
            exit(1);
          }
        }
      }
    }
  }
  
  // Write Xterms (all zeros)
  int NumTerms = 6;
  for( int i_mode = 1; i_mode <= 8; ++i_mode ) {
    for( int i_orb = 0; i_orb < n_orb; ++i_orb ) {
      for( int i_azi = 0; i_azi < n_azi; ++i_azi ) {
        for( int i_beam = 0; i_beam < 2; ++i_beam ) {
          float x_terms[NumTerms];
          for ( int i_term = 0; i_term < NumTerms; ++i_term )
            x_terms[i_term] = 0;
          // Write slice x terms
          for( int i_slice = 0; i_slice < 10; ++i_slice ) {
            if( fwrite(&x_terms[0],sizeof(float),NumTerms,ofp)!=(size_t)NumTerms) {
              fprintf(stderr,"%s: Error writing x table to file: %s\n",command,outfile);
              fclose(ofp);
              exit(1);
            }
          }
          // Write egg x terms
          if( fwrite(&x_terms[0],sizeof(float),NumTerms,ofp)!=(size_t)NumTerms) {
            fprintf(stderr,"%s: Error writing x table to file: %s\n",command,outfile);
            fclose(ofp);
            exit(1);
          }
        }
      }
    }
  }
  
  // Write Gfactors (range clipping)
  for( int i_mode = 1; i_mode <= 8; ++i_mode ) {
    for( int i_orb = 0; i_orb < n_orb; ++i_orb ) {
      for( int i_azi = 0; i_azi < n_azi; ++i_azi ) {
        for( int i_beam = 0; i_beam < 2; ++i_beam ) {
          float Gfactor[10];
          for( int i_slice = 0; i_slice < 10; ++i_slice )
            Gfactor[i_slice] = 1;
          if( fwrite(&Gfactor[0],sizeof(float),10,ofp)!=10) {
            fprintf(stderr,"%s: Error writing x table to file: %s\n",command,outfile);
            fclose(ofp);
            exit(1);
          }
        }
      }
    }
  }
  // Write fortran footer 
  if( fwrite(&nbytes,sizeof(int),1,ofp) != 1 ) {
    fprintf(stderr,"%s: Error writing x table to file: %s\n",command,outfile);
    fclose(ofp);
    exit(1);
  }
  fclose(ofp);
  return (0);
}

