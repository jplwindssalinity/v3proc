//==============================================================//
// Copyright (C) 2001, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.                    //
//==============================================================//

//----------------------------------------------------------------------
// NAME
//     estimate_kp
//
// SYNOPSIS
//    estimate_kp [-v] [-x] [-k] [-n] <gmf_file> <l2a_hdf_file> 
//                <l2b_hdf_file> 
//      [input_kp_file] <output_kp_file>
//    estimate_kp -c [-v] [-x] [-k] [input_kp_file] <output_kp_file>
//
// DESCRIPTION
//     Estimates Kp from real L2A and L2B data
//
// OPTIONS
//    [-c] Convert kp file format only  requires either -v or -x
//         with -c only the input_kp_file and output_kp_file operands
//         are supported
//    [-v] Write output in verbose ASCII format (default output is BINARY)
//    [-x] Write output in xmgr ASCII format (default output is BINARY)
//    [-k] Write Kpmu output to Kpm file (same format as class Kpm)
//    [-n] Use NCEP winds rather than retrieved wind to compute Kp  
//  
//
// OPERANDS
//    The following operands are supported:
//      <gmf_file>     Geophysical Model Function file
//      <l2a_hdf_file> The GDS L2A HDF file
//      <l2b_hdf_file> The GDS L2B HDF file
//      [input_kp_file] (Optional) Kp file including previously calculated
//                      Kp statistics to be updated from the input L2A, L2B
//                      files
//      <output_kp_file> Output file for Kp statistics
// EXAMPLES
//    An example of a command line is:
//      % estimate_kp qscat1.dat QS_S2A01067.19992001232 
//      %             QS_S2B01067.19992001250 est_kp.430-1066 est_kp.430-1067
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
//   Bryan W. STiles (Bryan.W.Stiles@jpl.nasa.gov)
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
#include <unistd.h>
#include "List.h"
#include "BufferedList.h"
#include "Misc.h"
#include "ConfigList.h"
#include "L2AH.h"
#include "ConfigSim.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "GMF.h"
#include "Kp.h"
#include "KpStatistics.h"
#include "Index.h"
#include "Tracking.h"
#include "WindGrad.h"

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
template class List<long>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class List<AngleInterval>;

//-----------//
// CONSTANTS //
//-----------//

// Dimensions of L2A and L2B arrays

#define OPTSTRING "cvxnk"
#define NUM_AT_CELLS 1624
#define NUM_CT_CELLS 76

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -v ]", "[ -x ]", "[ -n ]","[ -k ]",
			      "<GMF_file>",
			      "<l2a_hdf_file>","<l2b_hdf_file>",
			      "[input_kp_file]","<output_kp_file>", 0};
const char* usage_array2[] = { "-c", "[ -v ]", "[ -x ]", "[ -k ]"
			       "<input_kp_file>",
			       "<output_kp_file>", 0};



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

     extern int optind;
     int c;
     windTypeE wind_type = DIRTH;
     int convert_only_flag = 0;
     enum Out_E {BINARY,VERBOSE,XMGR, KPMU};
     Out_E out_type = BINARY;
     while ((c = getopt(argc, argv, OPTSTRING)) != -1)
       {
	 switch(c)
	   {
	   case 'c':
	     convert_only_flag=1;
             break;
           case 'v':
	     if(out_type==BINARY) out_type=VERBOSE;
	     else{
               fprintf(stderr,"%s: Output file type options incorrectly specified\n",command);
	       exit(1);
	     }
	     break;
	     
	   case 'x':
	     if(out_type==BINARY) out_type=XMGR;
	     else{
               fprintf(stderr,"%s: Output file type option incorrectly specified\n",command);
	       exit(1);
	     }
	     break;	     

	   case 'k':
	     if(out_type==BINARY) out_type=KPMU;
	     else{
               fprintf(stderr,"%s: Output file type option incorrectly specified\n",command);
	       exit(1);
	     }
	     break;	     
           
	   case 'n':
	     wind_type=NCEP;
	     break;

	   case '?':
	     usage(command, usage_array, 1);
	     break;
	   }
       }
      
     char* in_file=NULL;
     char* out_file=NULL, *gmf_file=NULL, *l2a_file=NULL, *l2b_file=NULL;
     if (convert_only_flag){
       if (argc !=optind+2 )
	 usage(command, usage_array2, 1);       
       in_file = argv[optind++]; 
       out_file = argv[optind++];     
     }

     else{
       if (argc !=optind+4 && argc !=optind+5 )
	 usage(command, usage_array, 1);
    
       gmf_file = argv[optind++];
       l2a_file = argv[optind++];    
       l2b_file = argv[optind++]; 
       if(argc==optind+2) in_file = argv[optind++]; 
       out_file = argv[optind++];     
     }

    //-------------------------------------------------//  
    // Set up dimensions of KpStatistics tables        //
    // This is where you edit bin numbers etc          //
    //-------------------------------------------------//
    KpStatistics kpstat;
    kpstat.beamIdx.SpecifyCenters(0,1,2);
    kpstat.aTIdx.SpecifyEdges(0,NUM_AT_CELLS-1,1);
    kpstat.cTIdx.SpecifyEdges(0,NUM_CT_CELLS-1,6);
    kpstat.scanAngleIdx.SpecifyWrappedCenters(0,two_pi,2);
    kpstat.spdIdx.SpecifyEdges(1,20,19);
    kpstat.chiIdx.SpecifyWrappedCenters(-pi,pi,1);

    //-------------------------------------------------------//
    // Allocate kpstat, reading from file if necessary       //
    //-------------------------------------------------------//

    if(in_file){
      int read_flag = kpstat.Read(in_file);
      if(read_flag==0){
	fprintf(stderr,"%s:I/O Error reading from %s.\n",command,in_file);
        exit(1);
      }
      else if(read_flag==-1){
	fprintf(stderr,"%s:Index mismatch reading from %s.\n",command,in_file);
        exit(1);
      }
    }
    else{
      if(!kpstat.Allocate()){
	fprintf(stderr,"%s:Error allocating kpstat.\n",command);
        exit(1);
      }
    }
    if(!convert_only_flag){
      //-----------------//
      // Set up GMF      //
      //-----------------//

      GMF gmf;
      if(! gmf.ReadOldStyle(gmf_file)){
	fprintf(stderr,"%s: Error reading GMF file %s\n",command,gmf_file);
	exit(1);
      }
      
      //-----------------//
      // Set up L2A      //
      //-----------------//

      L2AH l2ah;
      if(! l2ah.OpenForReading(l2a_file)){
	fprintf(stderr,"%s: Error opening L2A file %s.\n",command,l2a_file);
	exit(1);
      }
      
      //-----------------//
      // Set up L2B      //
      //-----------------// 

      L2B l2b;
      if(! l2b.ReadPureHdf(l2b_file)){
	fprintf(stderr,"%s: Error reading L2B file %s.\n",command,l2b_file);
	exit(1);
      }   
      l2b.frame.swath.DeleteFlaggedData(); // omit rainy cells    

      WindGrad windgrad(&(l2b.frame.swath),wind_type);
 
      //---------------------------------------------//
      // Loop through along track and cross track    //
      // indices.                                    //
      //---------------------------------------------//

      for(int ati=0;ati<NUM_AT_CELLS;ati++){
	for(int cti=0;cti<NUM_CT_CELLS;cti++){
	  // read l2a cell
	  MeasList * meas_list=l2ah.GetWVC(cti,ati,L2AH::OCEAN_ONLY);

	  // skip empty cells
	  if(meas_list==NULL) continue;
	  if(meas_list->NodeCount()==0) continue;

	  // read l2b cell
	  WVC* wvc=l2b.frame.swath.GetWVC(cti,ati);
          WGC* wgc=windgrad.GetWGC(cti,ati);

	  // skip empty cell or uncalculatable gradient
	  if(wvc==NULL || wgc==NULL) continue;
	  if(wvc->ambiguities.NodeCount()==0) continue;

	  // Update Kp statistics
	  if(!kpstat.Update(ati,cti,meas_list,wvc,wgc,&gmf,wind_type)){
	    fprintf(stderr,"%s: Error Kpstat:Update Failed\n",command);
	    exit(1);
	  }

	}
      }
    }
    switch(out_type){
    case BINARY :
      kpstat.Write(out_file);
      break;
    case VERBOSE :
      kpstat.WriteAscii(out_file);
      break;
    case XMGR :
      kpstat.WriteXmgr(out_file);
      break;
    case KPMU :
      kpstat.WriteKpmu(out_file);
      break;
    }
    return(0);
}



