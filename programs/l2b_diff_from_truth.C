//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		l2b_diff_from_truth
//
// SYNOPSIS
//		l2b_diff_from_truth <config_file> [ vctr_base ]
//
// DESCRIPTION
//	        Computes difference between wind vectors in two l2b files and
//              convert it to vector               
//		files for plotting in IDL.  Output filenames are created by
//		adding the rank number (0 for selected) to the base name.
//		If vctr_base is not provided, l2b_file is used as the base name.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operands are supported:
//		<config_file>	Simulation Configuration file
//		[ vctr_base ]	The output vctr file basename
//
// EXAMPLES
//		An example of a command line is:
//			% l2b_to_vctr quikscat.cfg diff.vctr
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
#include "ConfigSimDefs.h"
#include "L2B.h"
#include "L2AToL2B.h"
#include "Tracking.h"
#include "Tracking.C"


//-----------//
// TEMPLATES //
//-----------//

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

const char* usage_array[] = { "<config_file>", "<vctr_base>", 0};

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
	const char* vctr_base = argv[clidx++];

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

	//--------------------------------------//
	// read in  truth windfield             //
	//--------------------------------------//
	char* truth_type = config_list.Get(WINDFIELD_TYPE_KEYWORD);
	if (truth_type == NULL)
	  {
	    fprintf(stderr, "%s: must specify truth windfield type\n",
				command);
	    exit(1);
	  }
	
        char* truth_file = config_list.Get(WINDFIELD_FILE_KEYWORD);
	if (truth_file == NULL)
	  {
	    fprintf(stderr, "%s: must specify truth windfield file\n",
		    command);
	    exit(1);
	  }
	WindField truth;
	truth.ReadType(truth_file, truth_type);


	//------------------//
	// read in l2b file //
	//------------------//

	if (! l2b.OpenForReading())
	{
		fprintf(stderr, "%s: error opening L2B file\n",command);
		exit(1);
	}
	if (! l2b.ReadHeader())
	{
		fprintf(stderr, "%s: error reading L2B header from file \n",
			command);
		exit(1);
	}

	if (! l2b.ReadDataRec())
	{
		fprintf(stderr, "%s: error reading L2B data record from file \n",
			command);
		exit(1);
	}

	//----------------------//
        // compute difference & //
	// write out vctr files //
	//----------------------//
        l2b.frame.swath.DifferenceFromTruth(&truth);
	int max_rank = l2b.frame.swath.GetMaxAmbiguityCount();
	for (int i = 0; i <= max_rank; i++)
	{
		char filename[1024];
		sprintf(filename, "%s.%d", vctr_base, i);
		if (! l2b.WriteVctr(filename, i))
		{
			fprintf(stderr, "%s: error writing vctr file %s\n", command,
				filename);
			exit(1);
		}
	}

	return (0);
}
