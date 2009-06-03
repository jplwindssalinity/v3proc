//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l2a_to_ascii
//
// SYNOPSIS
//	      l2a_to_ascii <input_file> <output_file>  <start_frame> <end_frame>
//
// DESCRIPTION
//          Reads frames start_frame through end_frame from a L1b file and
//          writes them to an ASCII file
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
#include "L2A.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"

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
	L2A l2a;

	//------------------------//
	// open the input file    //
	//------------------------//

 	if (! l2a.OpenForReading(input_file))
	{
		fprintf(stderr, "%s: error opening input file %s\n", command,
			input_file);
		exit(1);
	}

	//------------------------//
	// open the output file   //
	//------------------------//

 	if (! l2a.OpenForWriting(output_file))
	{
		fprintf(stderr, "%s: error creating output file %s\n", command,
			input_file);
		exit(1);
	}       



        int frame_number=0;

	//---------------------//
	// copy desired frames //
	//---------------------//
        FILE* ifp=l2a.GetInputFp();
        l2a.ReadHeader();

        int cell_header_size=16;

        int valid=1, nm=0;
        off_t meas_size;
        while(nm==0){
          off_t tmp=ftello(ifp);
	  valid=l2a.ReadDataRec();
          if(start_frame>0) frame_number++;
	  nm=l2a.frame.measList.NodeCount();
	  if(nm>0) meas_size=(ftello(ifp)-tmp-cell_header_size)/nm;
        }
	
	while (valid && frame_number <= end_frame)
	{
	  if(frame_number >= start_frame){
	    l2a.WriteDataRecAscii();
	    valid=l2a.ReadDataRec();
	  }
          else if(frame_number==start_frame-1){
	    valid=l2a.ReadDataRec();
	  }
	  else{
	    fseeko(ifp,12,SEEK_CUR);
	    int nm;
            valid=fread(&nm,sizeof(int),1,ifp);
            valid=(valid==1);
	    if(valid){
	      fseeko(ifp,nm*meas_size,SEEK_CUR);
	      valid=!(feof(ifp));
	    }
	  }
          if(start_frame>=0) frame_number++;
	  if(frame_number%1000==0) fprintf(stderr,"%d frames read\n",frame_number-1);
        }
        if(start_frame>=0 && end_frame>frame_number){
	  fprintf(stderr,"Maximum frame number was %d\n",frame_number-1);
	}

        //----------------------//
        // close files and exit //
        //----------------------//

	l2a.Close();
        return(0);
}
