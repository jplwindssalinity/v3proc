//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		l2b_match_selected
//
// SYNOPSIS
//		l2b_match_selected source_file target_file output_file
//                        
//
// DESCRIPTION
//              Selects ambiguities in target_file which are closest to
//              the selecetd ambiguities in source file
//
// OPTIONS
//		None.
//
// OPERANDS
//		
//		self-explanatory
//
// EXAMPLES
//		An example of a command line is:
//			% l2b_match_Selected  dual_beam.l2b single_beam.l2b outfile.l2b
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
//		Bryan W. Stiles
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

const char* usage_array[] = { "selected_source_file", "<target_selected_file>", 
                              "<output_file>", "[hdf_source_flag 1=HDF 0= default]",0};


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
	if (argc != 4 && argc!=5)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* source_file = argv[clidx++];
	const char* target_file = argv[clidx++];
        const char* output_file = argv[clidx++];
        int hdf_source_flag=0;
        if(argc==5) hdf_source_flag=atoi(argv[clidx++]);

	//-------------------------------------//
	// create and configure level products //
	//-------------------------------------//

	L2B l2b_source, l2b_target;
        

	// Overwrite with new name
        l2b_target.SetOutputFilename(output_file);
        l2b_source.SetInputFilename(source_file);
        l2b_target.SetInputFilename(target_file);


        //------------//
	// open files //
	//------------//

        // Open Target Files
	if(!l2b_target.OpenForReading() ||
	   !l2b_target.OpenForWriting()){ 
		  fprintf(stderr,"%s: Error opening file\n",command);
	  exit(1);
	}
        // Read Source File
	if(hdf_source_flag){
	  if(l2b_source.ReadHDF()==0){	    
	    fprintf(stderr, "%s: error opening HDF L2B file \n", command);
	     exit(1);
	  }
	}
        else{
	  if(! l2b_source.OpenForReading()){
	    fprintf(stderr, "%s: error opening PE Source L2B file \n", command);
	     exit(1);
	  }
	  if (! l2b_source.ReadHeader())
	    {
	      fprintf(stderr, "%s: error reading Level 2B header\n", command); 
	      exit(1);
	    }
	  if (! l2b_source.ReadDataRec())
	    {
	      fprintf(stderr, "%s: error reading Level 2B data\n", command);
	      exit(1);
	    }


	}
	//---------------------------------//
	// read the headers to set up swath //
	//---------------------------------//


	if (! l2b_target.ReadHeader())
	{
		fprintf(stderr, "%s: error reading Level 2B header\n", command); 
		exit(1);
	}


	//--------------------------------//
	// read the level 2B data records //
	//--------------------------------//

	if (! l2b_target.ReadDataRec())
	  {
		fprintf(stderr, "%s: error reading Level 2B data\n", command);
		exit(1);
	  }

        l2b_target.frame.swath.MatchSelected(&(l2b_source.frame.swath));
	l2b_target.WriteHeader();
	l2b_target.WriteDataRec();
        l2b_source.Close();	
	l2b_target.Close();
	
	return(0);
}
