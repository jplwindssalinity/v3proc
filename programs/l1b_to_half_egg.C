//=========================================================//
// Copyright (C) 1998, California Institute of Technology. //
// U.S. Government sponsorship acknowledged.               //
//=========================================================//

//----------------------------------------------------------------------
// NAME
//    l1b_to_half_egg
//
// SYNOPSIS
//    l1b_to_half_egg <input_file> <output_file>
//
// DESCRIPTION
//    Reads a L1b file omits negative baseband slices and writes it
//    back out to an output_file
//
// OPTIONS
//    None.
//
// AUTHOR
//    Bryan Stiles (bstiles@acid.jpl.nasa.gov)
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
#include "Meas.h"
#include "List.h"
#include "BufferedList.h"
#include "Tracking.h"

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


const char* usage_array[] = { "<input_file>", "<output_file>",0};

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
	if (argc != 3)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* input_file = argv[clidx++];
	const char* output_file = argv[clidx++];

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

	//---------------------//
	// copy desired frames //
	//---------------------//

	while (l1b.ReadDataRec())
	{
        //------------------------//
        // Loop through spots     //
        //------------------------//

          for(MeasSpot* spot=l1b.frame.spotList.GetHead();spot;
	      spot=l1b.frame.spotList.GetNext()){


	    //------------------------------//
            // loop through slices          //
            //------------------------------//
            Meas* slice=spot->GetHead();
            while(slice){
	      if(slice->startSliceIdx<0){
		slice=spot->RemoveCurrent();
		delete slice;
                slice=spot->GetCurrent();
	      }
	      else slice=spot->GetNext();
	    }
	  }
	  l1b.WriteDataRec();
        }

        //----------------------//
        // close files and exit //
        //----------------------//

	l1b.Close();
        return(0);
}
