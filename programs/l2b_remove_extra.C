//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		l2b_medianfilter
//
// SYNOPSIS
//		l2b_medianfilter <sim_config_file>
//
// DESCRIPTION
//		Simulates the SeaWinds 1b ground processing of Level 2A to
//		Level 2B data.  This program retrieves wind from measurements.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//		<sim_config_file>		The sim_config_file needed listing
//								all input parameters, input files, and
//								output files.
//
// EXAMPLES
//		An example of a command line is:
//			% l2b_medianfilter sws1b.cfg
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

const char* usage_array[] = { "<sim_config_file>", "<output_l2b_file>", 0};

//--------------------//
// Report handler     //
// runs if SIGUSR1 is //
// recieved.          //
//--------------------//

int global_frame_number=0;

void report(int sig_num){
  fprintf(stderr,"l2b_medianfilter: Starting frame number %d\n",
	  global_frame_number);
  return;
}

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
	if (argc != 3)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* config_file = argv[clidx++];
	const char* l2b_output_file = argv[clidx++];

        //------------------------//
        // tell how far you have  //
        // gotten if you recieve  //
        // the siguser1 signal    //
        //------------------------//

        sigset(SIGUSR1,&report);

	//---------------------//
	// read in config file //
	//---------------------//

	ConfigList config_list;
	if (! config_list.Read(config_file))
	{
		fprintf(stderr, "%s: error reading sim config file %s\n",
			command, config_file);
		exit(1);
	}

	//-------------------------------------//
	// create and configure level products //
	//-------------------------------------//

	L2B l2b;
	if (! ConfigL2B(&l2b, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 2B Product\n", command);
		exit(1);
	}

	// Overwrite with new name
    l2b.SetOutputFilename(l2b_output_file);


	//------------------------------------//
	// create and configure the converter //
	//------------------------------------//

	L2AToL2B l2a_to_l2b;
	if (! ConfigL2AToL2B(&l2a_to_l2b, &config_list))
	{
		fprintf(stderr, "%s: error configuring L2AToL2B\n", command);
		exit(1);
	}

	//------------//
	// open files //
	//------------//

	l2b.OpenForReading();
	l2b.OpenForWriting();

	//---------------------------------//
	// read the header to set up swath //
	//---------------------------------//

	if (! l2b.ReadHeader())
	{
		fprintf(stderr, "%s: error reading Level 2B header\n", command); 
		exit(1);
	}

	//-----------------//
	// conversion loop //
	//-----------------//

	//-----------------------------//
	// read thelevel 2B data record //
	//-----------------------------//

	if (! l2b.ReadDataRec())
	  {
	    switch (l2b.GetStatus())
	      {
	      case L2B::OK:		// end of file
		break;
	      case L2B::ERROR_READING_FRAME:
		fprintf(stderr, "%s: error reading Level 2B data\n", command);
		exit(1);
		break;
	      case L2B::ERROR_UNKNOWN:
		fprintf(stderr, "%s: unknown error reading Level 2B data\n",
			command);
		exit(1);
		break;
	      default:
		fprintf(stderr, "%s: unknown status (???)\n", command);
		exit(1);
	      }
	  }
	//----------------------------------------------------//
	// Remove extra ambiguities (were allocated selected's)//
	//----------------------------------------------------//
	for (int cti = 0; cti < 80; cti++)
	  {   
	    for(int ati=0;ati<1603;ati++){
	      
	      WVC* wvc=l2b.frame.swath.swath[cti][ati];
	      if(!wvc) continue;
	      int na=wvc->ambiguities.NodeCount();
	      int nr=wvc->directionRanges.NodeCount();
	      if(na>nr){
		WindVectorPlus* wvp=wvc->ambiguities.GetTail();
		    wvp=wvc->ambiguities.RemoveCurrent();
		    delete wvp;
	      }
	    }
	  }
	l2b.WriteHeader();
	l2b.WriteDataRec();
	
	l2b.Close();
	
	return (0);
}
