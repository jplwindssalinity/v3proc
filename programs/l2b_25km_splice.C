//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		l2b_splice
//
// SYNOPSIS
//		l2b_splice inner_swath_file outer_swath_file output_file
//                         threshold_distance
//
// DESCRIPTION
//              Puts together two l2b files taking form one for the
//              inner swath and the other for the outer swath
//
// OPTIONS
//		None.
//
// OPERANDS
//		
//		First three are self-explanatory.
//              threshold_distance: swath is broken into inner between
//              -threshold_distance and + threshold distance and outer
//              everything else
//
// EXAMPLES
//		An example of a command line is:
//			% l2b_splice  in.l2b.dat out.l2b.dat l2b.dat 500
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
//		James N. Huddleston
//		hudd@acid.jpl.nasa.gov
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
#include "BufferedList.h"
#include "Misc.h"
#include "ConfigList.h"
#include "L2A.h"
#include "ConfigSim.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Tracking.h"


//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<AngleInterval>;
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

const char* usage_array[] = { "<inner_swath_file>", "<output_swath_file>", 
                              "<output_file>","<threshold_distance>"};


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
	if (argc != 5)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* inner_file = argv[clidx++];
	const char* outer_file = argv[clidx++];
        const char* output_file = argv[clidx++];
        float threshold=atof(argv[clidx++]);


	//-------------------------------------//
	// create and configure level products //
	//-------------------------------------//

	L2B l2b_in, l2b_out;
        

	// Overwrite with new name
        l2b_out.SetOutputFilename(output_file);
        l2b_in.SetInputFilename(inner_file);
        l2b_out.SetInputFilename(outer_file);


        //------------//
	// open files //
	//------------//

	if(!l2b_out.OpenForReading() ||
	   !l2b_out.OpenForWriting() ||
	   !l2b_in.OpenForReading()){
	  fprintf(stderr,"%s: Error opening file\n",command);
	  exit(1);
	}

	//---------------------------------//
	// read the headers to set up swath //
	//---------------------------------//

	if (! l2b_in.ReadHeader())
	{
		fprintf(stderr, "%s: error reading Level 2B header\n", command); 
		exit(1);
	}

	if (! l2b_out.ReadHeader())
	{
		fprintf(stderr, "%s: error reading Level 2B header\n", command); 
		exit(1);
	}


	//--------------------------------//
	// read the level 2B data records //
	//--------------------------------//

	if (! l2b_in.ReadDataRec())
	  {
		fprintf(stderr, "%s: error reading Level 2B data\n", command);
		exit(1);
	  }

	if (! l2b_out.ReadDataRec())
	  {
		fprintf(stderr, "%s: error reading Level 2B data\n", command);
		exit(1);
	  }

	//----------------------------------------------------//
	// splice  Right now only works for 25km resolution   //
	//----------------------------------------------------//
        // Determine edge_indices
        float radius=threshold/25.0;
        int left=(int)ceil(39.5-radius);
        int right=(int)floor(39.5+radius);
 	for (int cti = left; cti <= right; cti++)
	  {   
	    for(int ati=0;ati<1603;ati++){
              WVC* wvc=l2b_out.frame.swath.Remove(cti,ati);
              if(wvc) delete wvc;
              wvc=l2b_in.frame.swath.Remove(cti,ati);
              if(wvc) l2b_out.frame.swath.Add(cti,ati,wvc);
	    }
	  }
       
	l2b_out.WriteHeader();
	l2b_out.WriteDataRec();
        l2b_in.Close();	
	l2b_out.Close();
	
	return(0);
}
