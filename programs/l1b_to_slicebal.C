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

 	if (! l1b.OpenForWriting(output_file))
	{
		fprintf(stderr, "%s: error creating output file %s\n", command,
			input_file);
		exit(1);
	}       



        int frame_number=1;

	//---------------------//
	// copy desired frames //
	//---------------------//

	while (l1b.ReadDataRec() && frame_number <= end_frame)
	{
	  for(MeasSpot*spot=l1b.frame.spotList.GetHead();spot;
	      spot=l1b.frame.spotList.GetNext()){
	      Meas* meas=spot->GetHead();
	      double x,y,z;
              spot->scOrbitState.rsat.Get(&x,&y,&z);
	      printf("%d %g %g ",meas->beamIdx,spot->time,meas->scanAngle*rtd);
	      for(meas=spot->GetHead();meas;meas=spot->GetNext()){
		printf("%d ",meas->landFlag);
	      }     
	      for(meas=spot->GetHead();meas;meas=spot->GetNext()){
		printf("%g ",meas->incidenceAngle);
	      }     
	      for(meas=spot->GetHead();meas;meas=spot->GetNext()){
		printf("%g ",meas->value);
	      } 
	      Meas egg;
              spot->GetHead();
              spot->GetNext();
	      egg.Composite(spot,10);
	      printf("%g\n",egg.value);
	  }
        }

        //----------------------//
        // close files and exit //
        //----------------------//

	l1b.Close();
        return(0);
}
