//==============================================================//
// Copyright (C) 1997-2000, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//    l2a_subsample
//
// SYNOPSIS
//    l2a_subsample <sim_config_file> <output_l2a> <ss_factor> [ignore_bad_L2A]
//
// DESCRIPTION
//    Subsamples a l2a file by ss_factor in both dimension 
//    wind up with ss_factor*ss_factor fewer cells and the same number
//    of measurements
//
// OPTIONS
//    None.
//
// OPERANDS
//    The following operand is supported:
//      <sim_config_file>  The sim_config_file needed listing
//                         all input parameters, input files, and
//                         output files.
//
// EXAMPLES
//    An example of a command line is:
//      % l2a_to_l2b sws1b.cfg
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
//    None.
//
// AUTHORS
//    Bryan W. Stiles
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
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "ConfigList.h"
#include "L2A.h"
#include "ConfigSim.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Tracking.h"
#include "Tracking.C"

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<AngleInterval>;
template list<string>;
template map<string,string,Options::ltstr>;

//-----------//
// CONSTANTS //
//-----------//

#define MAX_ALONG_TRACK_BINS  1624

//-------//
// HACKS //
//-------//

//#define LATLON_LIMIT_HACK

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

const char* usage_array[] = { "<l2a_input_file>", "<l2a_output_file>",
"<subsample_factor>",0};


//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
    int    argc,
    char*  argv[])
{
    //------------------------//
    // parse the command line //
    //------------------------//

    const char* command = no_path(argv[0]);
    if (argc != 4 )
        usage(command, usage_array, 1);

    int clidx = 1;
    const char* in_file = argv[clidx++];
    const char* out_file = argv[clidx++];
    int ssf=atoi(argv[clidx++]);





    //-------------------------------------//
    // create and open      L2A files      //
    //-------------------------------------//

    L2A l2a_in, l2a_out;


    //------------//
    // open files //
    //------------//

    l2a_in.OpenForReading(in_file);
    l2a_out.OpenForWriting(out_file);
    FILE* ifp=l2a_in.GetInputFp();
    FILE* ofp=l2a_out.GetOutputFp();

   

    //---------------------------------//
    // read the header to set up swath //
    //---------------------------------//

    if (! l2a_in.ReadHeader())
    {
        fprintf(stderr, "%s: error reading Level 2A header\n", command);
        exit(1);
    }

    off_t header_size=ftello(ifp);
    int cell_header_size=16; 
    int along_track_bins =
        (int)(two_pi * r1_earth / l2a_in.header.alongTrackResolution + 0.5);

    int ** nummeas=(int**)make_array(sizeof(int),2,
				     along_track_bins,
				     l2a_in.header.crossTrackBins);
    off_t ** offset=(off_t**)make_array(sizeof(off_t),2,
					along_track_bins,
					l2a_in.header.crossTrackBins);
    int nm;
    off_t tmp;
    while(nm==0){
      tmp=ftello(ifp);
      if (! l2a_in.ReadDataRec())
        {
	  fprintf(stderr, "%s: error reading Level 2A data\n", command);
	  exit(1);
	}

      nm=l2a_in.frame.measList.NodeCount();
      
    }

    // assumes all measurements in the file are the same length

    int meas_length=(int)((ftello(ifp)-tmp-cell_header_size)/nm);

    // get number of bytes in files
    fseeko(ifp,0,SEEK_END);
    off_t end_byte = ftello(ifp);
    fseeko(ifp,header_size,SEEK_SET);

    // initialize num_meas
    for(int a=0;a<along_track_bins;a++){
      for(int c=0;c<l2a_in.header.crossTrackBins;c++){
	nummeas[a][c]=0;
        offset[a][c]=0;
      }
    }


    //---------------------------------//
    // compute nummeas loop            //
    //---------------------------------//
    int frame_number=0;
    for (;;)
    {

        frame_number++;
        if(frame_number%1000==0) fprintf(stderr,"%d frames sorted\n",
					      frame_number);
        // skip rev number
        fseeko(ifp,4,SEEK_CUR);
        int cti,ati,nm;
        if( fread(&ati,sizeof(int),1,ifp)!=1 ||
	    fread(&cti,sizeof(int),1,ifp)!=1 ||
	    fread(&nm,sizeof(int),1,ifp)!=1){
	  if(feof(ifp)) break;
	  fprintf(stderr,"Error read L2A cell header\n");
	  exit(1);
	}
        nummeas[ati][cti]=nm;
        offset[ati][cti]=ftello(ifp);
        fseeko(ifp,nm*meas_length,SEEK_CUR);
        if(ftello(ifp)>end_byte) break;
    }
 
    char buffer[500000];
    //-----------------------------------//
    // write output file                 //
    //-----------------------------------//
    int along_track_bins_out=along_track_bins/ssf;
    l2a_out.header.alongTrackResolution=l2a_in.header.alongTrackResolution*ssf;
    l2a_out.header.crossTrackResolution=l2a_in.header.crossTrackResolution*ssf;
    l2a_out.header.alongTrackBins=l2a_in.header.alongTrackBins/ssf;
    l2a_out.header.crossTrackBins=l2a_in.header.crossTrackBins/ssf;
    l2a_out.header.zeroIndex=(int)(l2a_out.header.crossTrackBins/2.0 + 0.5);
    l2a_out.header.startTime=l2a_in.header.startTime;
    l2a_out.WriteHeader();

    int rev=0;
    for(int a=0;a<along_track_bins_out;a++){
      int a2start=a*ssf;
      int a2end=a2start+ssf;
      for(int c=0;c<l2a_out.header.crossTrackBins;c++){
	int c2start=c*ssf;
	int c2end=c2start+ssf;
	// compute number of meas and output rev,ati,cti,nummeas
	int nm2=0;
	for(int a2=a2start;a2<a2end;a2++){
	  for(int c2=c2start;c2<c2end;c2++){
	    nm2+=nummeas[a2][c2];
	  } 
	}
        if(nm2==0) continue;
        if(fwrite(&rev,sizeof(int),1,ofp)!=1 ||
	   fwrite(&a,sizeof(int),1,ofp)!=1 ||
	   fwrite(&c,sizeof(int),1,ofp)!=1 ||
	   fwrite(&nm2,sizeof(int),1,ofp)!=1)
	{
	  fprintf(stderr,"Cannot write cell header to %s\n",out_file);
	  exit(1);
	}
	// read and write out measurements
	for(int a2=a2start;a2<a2end;a2++){
	  for(int c2=c2start;c2<c2end;c2++){
	    if(nummeas[a2][c2]==0) continue;
	    unsigned int N=(unsigned int)nummeas[a2][c2]*meas_length;
	    fseeko(ifp,offset[a2][c2],SEEK_SET);
	    if(fread(buffer,sizeof(char),N,ifp)!=N){
	      fprintf(stderr,"Cannot read buffer from %s\n",in_file);
	      exit(1);
	    }
	    if(fwrite(buffer,sizeof(char),N,ofp)!=N){
	      fprintf(stderr,"Cannot write buffer to %s\n",out_file);
	      exit(1);
	    }
	  } 
	}
       }
       fprintf(stderr,"%d rows written\n",a+1);
    }
    free_array(nummeas,2,along_track_bins,
	       l2a_in.header.crossTrackBins);
    free_array(offset,2,along_track_bins,
	       l2a_in.header.crossTrackBins);
    l2a_in.Close();
    l2a_out.Close();
    return(0);
}
