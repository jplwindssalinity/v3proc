//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l1b_to_slicebal
//
// SYNOPSIS
//	      l1b_to_slicebal <input_file> <output_file>  <start_frame> <end_frame>
//
// DESCRIPTION
//          Reads frames start_frame through end_frame from a L1b file and
//          extracts data to an ASCII file for use in slice balancing
//      OPTIONS
//		Last two arguments are optional
// AUTHOR
//		Bryan Stiles
//		bstiles@acid.jpl.nasa.gov
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
#include "Misc.h"
#include "L1B.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"
#define  TOTAL_NUM_SLICES 12
#define  DATA_UNAVAILABLE -999
//-----------//
// TEMPLATES //
//-----------//

template class List<EarthPosition>;
template class List<Meas>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<long>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;


const char* usage_array[] = { "<input_file>", "<output_file>",
			      "<start_frame>(OPT)",
			      "<end_frame>(OPT)",0};

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
	int		argc,
	char*	argv[])
{
	//------------------------//
	// parse the command line //
	//------------------------//

	const char* command = no_path(argv[0]);
	if (argc != 5 && argc!=3)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* input_file = argv[clidx++];
	const char* output_file = argv[clidx++];
        int start_frame=-1, end_frame=2;
        if(argc==5){
	  start_frame=atoi(argv[clidx++]);
	  end_frame=atoi(argv[clidx++]);
	}

	//------------------------//
	// create L2A object      //
	//------------------------//
	L1B l1b;

	//------------------------//
	// open the input file    //
	//------------------------//

 	if (! l1b.OpenForReading(input_file))
	{
		fprintf(stderr, "%s: error opening input file %s\n", command,
			input_file);
		exit(1);
	}

	//------------------------//
	// open the output file   //
	//------------------------//

        FILE* ofp=fopen(output_file,"w");
        if(!ofp){
	  fprintf(stderr,"%s:Cannot create file %s\n",command,output_file);
	  exit(1);
	}



        int frame_number=1;
      
	//---------------------//
	// copy desired frames //
	//---------------------//

	while (l1b.ReadDataRec() && frame_number <= end_frame)
	{
	  if(frame_number < start_frame){
	    frame_number++;
	    continue;
	  }

	  for(MeasSpot*spot=l1b.frame.spotList.GetHead();spot;
	      spot=l1b.frame.spotList.GetNext()){
	      Meas* meas=spot->GetHead();
	      double x,y,z;
              spot->scOrbitState.rsat.Get(&x,&y,&z);
	      fprintf(ofp,"%d %g %g ",meas->beamIdx,spot->time,meas->scanAngle*rtd);
	      

	      //   Output  Land flags for each slice

	      int count=0;
	      for(meas=spot->GetHead();meas;meas=spot->GetNext()){
		int idx=0;
                rel_to_abs_idx(meas->startSliceIdx,TOTAL_NUM_SLICES,&idx);
		while(count< idx){
		  fprintf(ofp,"%d ", DATA_UNAVAILABLE);
		  count++;
		}
		fprintf(ofp,"%d ",meas->landFlag);
		count++;
	      } 
	      while(count< TOTAL_NUM_SLICES){
		fprintf(ofp,"%d ", DATA_UNAVAILABLE);
		count++;
	      }

	      //   Output Incidence Angles for each slice
	      count=0;    
	      for(meas=spot->GetHead();meas;meas=spot->GetNext()){
		int idx=0;
                rel_to_abs_idx(meas->startSliceIdx,TOTAL_NUM_SLICES,&idx);
		while(count< idx){
		  fprintf(ofp,"%d ", DATA_UNAVAILABLE);
		  count++;
		}
		fprintf(ofp,"%g ",meas->incidenceAngle*rtd);
		count++;
	      }   
	      while(count< TOTAL_NUM_SLICES){
		fprintf(ofp,"%d ", DATA_UNAVAILABLE);
		count++;
	      }

              //   Output Sigma0 Values for each slice
	      count=0;  
	      for(meas=spot->GetHead();meas;meas=spot->GetNext()){
		int idx=0;
                rel_to_abs_idx(meas->startSliceIdx,TOTAL_NUM_SLICES,&idx);
		while(count< idx){
		  fprintf(ofp,"%d ", DATA_UNAVAILABLE);
		  count++;
		}
		fprintf(ofp,"%g ",meas->value);    
		count++;
	      } 
	      while(count< TOTAL_NUM_SLICES){
		fprintf(ofp,"%d ", DATA_UNAVAILABLE);
		count++;
	      }
              //---------------------------------------//
              // Output Egg Sigma0                     //
              //---------------------------------------//
	      Meas egg;
	      count=spot->NodeCount();

	      int first_idx, last_idx;
	      meas=spot->GetHead();              
              rel_to_abs_idx(meas->startSliceIdx,TOTAL_NUM_SLICES,&first_idx);
	      meas=spot->GetTail();              
              rel_to_abs_idx(meas->startSliceIdx,TOTAL_NUM_SLICES,&first_idx);

	      int num_missing_guards=0;
              if(first_idx>0) num_missing_guards++;
              if(last_idx<TOTAL_NUM_SLICES-1) num_missing_guards++;
	      if(count == TOTAL_NUM_SLICES){
		spot->GetHead();
		spot->GetNext();
		egg.Composite(spot,TOTAL_NUM_SLICES-2);
		fprintf(ofp,"%g\n",egg.value);
	      }
	      else if(count+num_missing_guards==TOTAL_NUM_SLICES){
		spot->GetHead();
		if(first_idx==0)  spot->GetNext();
		egg.Composite(spot,TOTAL_NUM_SLICES-2);
		fprintf(ofp,"%g\n",egg.value);		
	      }
	      else{
		fprintf(ofp,"%d\n",DATA_UNAVAILABLE);
	      }
	  }
	  if(start_frame>=0) frame_number++;
        }

        //----------------------//
        // close files and exit //
        //----------------------//

	l1b.Close();
        return(0);
}
