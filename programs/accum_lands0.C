//==============================================================//
// Copyright (C) 2007-3007, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		l1b_hdf_to_coastal_l1b
//
// SYNOPSIS
//		l1b_hdf_to_coastal_l1b config_file l1b_hdf_file output_file 
//
// DESCRIPTION
//		Generates output files containing ASCII output of a wind swatch
//		given a L1B HDF file.
//
// OPTIONS
//		[ config_file ]	Use the specified config file.
//		[ l1b_hdf_file ]		Use this HDF l1b file.
//		[ output_file ]	The name to use for output file.
//
// OPERANDS
//		None.
//
// EXAMPLES
//		An example of a command line is:
//			% test_hdf_l1b -c sally.cfg -l L1B_100.file -o l1b.out
//
// ENVIRONMENT
//		Not environment dependent.
//
// EXIT STATUS
//		The following exit values are returned:
//		0	Program executed successfully
//		>0	Program had an error
//
// NOTES
//		None.
//
// AUTHOR
//              Bryan Stiles
//----------------------------------------------------------------------

//-----------------------//
// Configuration Control //
//-----------------------//

static const char rcs_id[] =
	"@(#) $Id$";

//----------//
// INCLUDES //
//----------//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "Misc.h"
#include "Ephemeris.h"
#include "ConfigList.h"
#include "L1A.h"
#include "ConfigSim.h"
#include "L1BHdf.h"
#include "L1AToL1B.h"
#include "Tracking.h"
#include "Tracking.C"
#include "QscatConfig.h"
#include "CoastalMaps.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"


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



//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "neastaz", "nlats","nlons", "dir1", "dir2", "..."
	, 0 };


//--------------//
// MAIN PROGRAM //
//--------------//
int
main( int argc, char* argv[]){
  const char* command = no_path(argv[0]);

  if(argc==1) usage(command, usage_array, 1);

  int nlats,nlons,naz,nlooks=4;
  int clidx=1;
  naz=atoi(argv[clidx++]);
  nlats=atoi(argv[clidx++]);
  nlons=atoi(argv[clidx++]);
  float *** sumg=(float***)make_array(sizeof(float),3,nlooks,nlats,nlons);
  float *** totsumg=(float***)make_array(sizeof(float),3,naz,nlats,nlons);
  float *** inlands0=(float***)make_array(sizeof(float),3,nlooks,nlats,nlons);
  float *** outlands0=(float***)make_array(sizeof(float),3,naz,nlats,nlons);
  float *** eastaz=(float***)make_array(sizeof(float),3,nlooks,nlats,nlons);
  int *** n=(int***)make_array(sizeof(int),3,nlooks,nlats,nlons);

  // loop through directories
  char pref1[200], pref2[200],filename[200];
  char* dir1=argv[clidx];
  char* dir2=argv[argc-1];


  // open output file
  char outfile[200];
  sprintf(outfile,"CM%s_%s_eastaz.lands0",dir1,dir2);
  FILE* ofp=fopen(outfile,"w");
  if(ofp==NULL){
    fprintf(stderr,"accum_lands0 could not create output file %s/n",
	    outfile);
    exit(1);
  }
  while(clidx<argc){
    char* dir=argv[clidx++];
    int skip_this_dir=0;    
    sprintf(pref1,"%s/CM%s_total",dir,dir);
    fprintf(stderr,"processing %s ..\n",dir);
    // read input files and accumulate
    for(int l=0;l<nlooks;l++){
      switch(l){
      case 0:
	sprintf (pref2,"%s_innerfore",pref1);
	break;
      case 1:
	sprintf (pref2,"%s_inneraft",pref1);
	break;
      case 2:
	sprintf (pref2,"%s_outerfore",pref1);
	break;
      case 3:
	sprintf (pref2,"%s_outeraft",pref1);
	break;
      default:
	fprintf(stderr,"accum_lands0: Bad look set %d\n",l);
	exit(1);
      }

      // read sumgfile
      sprintf(filename,"%s.gain",pref2);
      FILE* ifp=fopen(filename,"r");
      if(ifp==NULL){
	fprintf(stderr,"accum_lands0 error opening file %s\n",filename);
	fprintf(stderr,"skipping directory %s\n",dir);
        skip_this_dir=1;
	break;
      }
      for(int i=0;i<nlats;i++){
	if(fread((void*)&sumg[l][i][0],sizeof(float),nlons,ifp)!=(unsigned int)nlons){
	  fprintf(stderr,"accum_lands0 error reading file %s\n",filename);
	  fprintf(stderr,"skipping directory %s\n",dir);
	  skip_this_dir=1;
	  break;
	}
      }
      fclose(ifp);
      if(skip_this_dir) break;


      // read lands0 file
      sprintf(filename,"%s.lands0",pref2);
      ifp=fopen(filename,"r");
      if(ifp==NULL){
	fprintf(stderr,"accum_lands0 error opening file %s\n",filename);
	fprintf(stderr,"skipping directory %s\n",dir);
        skip_this_dir=1;
	break;
      }
      for(int i=0;i<nlats;i++){
	if(fread((void*)&inlands0[l][i][0],sizeof(float),nlons,ifp)!=(unsigned int)nlons){
	  fprintf(stderr,"accum_lands0 error reading file %s\n",filename);
	  fprintf(stderr,"skipping directory %s\n",dir);
	  skip_this_dir=1;
	  break;
	}
      }
      fclose(ifp);
      if(skip_this_dir) break;


      // read eastaz file
      sprintf(filename,"%s.eastaz",pref2);
      ifp=fopen(filename,"r");
      if(ifp==NULL){
	fprintf(stderr,"accum_lands0 error opening file %s\n",filename);
	fprintf(stderr,"skipping directory %s\n",dir);
        skip_this_dir=1;
	break;
      }
      for(int i=0;i<nlats;i++){
	if(fread((void*)&eastaz[l][i][0],sizeof(float),nlons,ifp)!=(unsigned int)nlons){
	  fprintf(stderr,"accum_lands0 error reading file %s\n",filename);
	  fprintf(stderr,"skipping directory %s\n",dir);
	  skip_this_dir=1;
	  break;
	}
      }
      fclose(ifp);
      if(skip_this_dir) break;


      // read n file
      sprintf(filename,"%s.numslices",pref2);
      ifp=fopen(filename,"r");
      if(ifp==NULL){
	fprintf(stderr,"accum_lands0 error opening file %s\n",filename);
	fprintf(stderr,"skipping directory %s\n",dir);
        skip_this_dir=1;
	break;
      }
      for(int i=0;i<nlats;i++){
	if(fread((void*)&n[l][i][0],sizeof(int),nlons,ifp)!=(unsigned int)nlons){
	  fprintf(stderr,"accum_lands0 error reading file %s\n",filename);
	  fprintf(stderr,"skipping directory %s\n",dir);
	  skip_this_dir=1;
	  break;
	}
      }
      fclose(ifp);
      if(skip_this_dir) break;

      // accumulate
      for(int i=0;i<nlats;i++){
	for(int j=0;j<nlons;j++){
	  float azstep=two_pi/naz;
	  int az=(int)floor(eastaz[l][i][j]/azstep);
          az%=naz;
          float w=sumg[l][i][j]*n[l][i][j];
	  totsumg[az][i][j]+=w;
	  outlands0[az][i][j]+=w*inlands0[l][i][j];
	}
      }
    }
  }

  // Normalize and output to file

  for(int az=0;az<naz;az++){
    for(int i=0;i<nlats;i++){
      for(int j=0;j<nlons;j++){
	if(totsumg[az][i][j]>0)
	  outlands0[az][i][j]/=totsumg[az][i][j];
	else outlands0[az][i][j]=0;
      }
      if(fwrite((void*)&outlands0[az][i][0],sizeof(float),nlons,ofp)!=(unsigned int)nlons){
	  fprintf(stderr,"accum_lands0 error writing file %s\n",outfile);
	  exit(1);
	}

    }
  }




  fclose(ofp);
  free_array(sumg,3,nlooks,nlats,nlons);
  free_array(totsumg,3,nlooks,nlats,nlons);
  free_array(inlands0,3,nlooks,nlats,nlons);
  free_array(outlands0,3,naz,nlats,nlons);
  free_array(eastaz,3,nlooks,nlats,nlons);
  free_array(n,3,nlooks,nlats,nlons);
  return(1);
}	
