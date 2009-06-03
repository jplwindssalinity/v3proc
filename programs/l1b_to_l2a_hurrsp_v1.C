//==============================================================//
// Copyright (C) 2007-3007, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		l1b_hdf_to_coastal_l1b
//
// SYNOPSIS
//		l1b_hdf_to_coastal_l1b config_file l1b_hdf_file output_file 
//
// DESCRIPTION
//		Generates output files containing ASCII output of a wind swatch
//		given a L2A HDF file.
//
// OPTIONS
//		[ config_file ]	Use the specified config file.
//		[ l1b_hdf_file ]		Use this HDF l1b file.
//		[ output_file ]	The name to use for output file.
//
// OPERANDS
//		None.
//
// EXAMPLES
//		An example of a command line is:
//			% test_hdf_l1b -c sally.cfg -l L1B_100.file -o l1b.out
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
//              Bryan Stiles
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
#include <unistd.h>
#include <stdlib.h>
#include "Misc.h"
#include "Ephemeris.h"
#include "ConfigList.h"
#include "L1A.h"
#include "ConfigSim.h"
#include "L1BHdf.h"
#include "L1AToL1B.h"
#include "Tracking.h"
#include "Tracking.C"
#include "QscatConfig.h"
#include "CoastalMaps.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"


using std::list;
using std::map;

//-----------//
// TEMPLATES //
//-----------//

class AngleInterval;

template class List<AngleInterval>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<StringPair>;
template class List<Meas>;
template class List<EarthPosition>;
template class List<MeasSpot>;
template class List<WindVectorPlus>;
template class List<off_t>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;
template class std::list<string>;
template class std::map<string,string,Options::ltstr>;

//-----------//
// CONSTANTS //
//-----------//



//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "config_file", "l1b_hdf_file ",
	" output_file ", 0 };

// not always evil...
const char*		command = NULL;
char*			l1b_hdf_file = NULL;
char*			output_file = NULL;
FILE*           output_fp = stdout;

//--------------//
// MAIN PROGRAM //
//--------------//

int
main(
	int		argc,
	char*	argv[])
{
	//-----------//
	// variables //
	//-----------//

	char* config_file = NULL;
	ConfigList config_list;
	l1b_hdf_file = NULL;
	output_file = NULL;

	//------------------------//
	// parse the command line //
	//------------------------//

	command = no_path(argv[0]);

	if (argc !=4)
		usage(command, usage_array, 1);

        
        int clidx=1;
        config_file=argv[clidx++];
        if(! config_list.Read(config_file))
	  {
	    fprintf(stderr,"%s Error reading config_file %s\n",command,config_file);
	    exit(1);
	  }
	l1b_hdf_file=argv[clidx++];
	output_file=argv[clidx++];



	//-----------------------//
	// read in HDF 1B file   //
	//-----------------------//
	HdfFile::StatusE rc;
	L1BHdf  l1bHdf(l1b_hdf_file, rc);
	if (rc != HdfFile::OK)
	  {
	    fprintf(stderr, "%s: cannot open HDF %s for input\n",
		    argv[0], l1b_hdf_file);
	    exit(1);
	  }
	fprintf(stdout, "%s: %s has %d records\n",
		argv[0], l1b_hdf_file, l1bHdf.GetDataLength());

	//--------------------------------------------
	// configure L1B HDF object from config list
	//--------------------------------------------
	if ( ! ConfigL1BHdf(&l1bHdf, &config_list))
	  {
	    fprintf(stderr, "%s: config L1B HDF failed\n", argv[0]);
	    exit(1);
	  }


	//--------------------------------------------
	// configure LandMap object from config list
	//--------------------------------------------
        CoastalMaps lmap;
	if ( ! ConfigLandMap(&lmap, &config_list))
	  {
	    fprintf(stderr, "%s: config LandMap failed\n", argv[0]);
	    exit(1);
	  }

        lmap.InitExtraMaps(23*dtr,-93*dtr, dtr/30.0, 9.0*dtr,9.0*dtr,1);

        // lmap.ReadPrecomputeLands0("precomputed");

	//--------------------------------------------
	// configure Antenna objects
	//--------------------------------------------	
        Antenna ant;
	if ( ! ConfigAntenna(&ant,&config_list))
	  {
	    fprintf(stderr, "%s: config Antenna failed\n", argv[0]);
	    exit(1);
	  }

	

	//-----------------------//
	//       convert SVT L1B  //
	//-----------------------//

        int num_badframes=0;
        int found_goodframes=0;
	while (l1bHdf.ReadL1BHdfDataRecCoastal(&lmap,&ant))
	  {
	    if(l1bHdf.frame.spotList.NodeCount()==0){
	      num_badframes++;
	    }
	    else{
	      found_goodframes=1;
	      num_badframes=0;
	    }
	    if(num_badframes>100 && found_goodframes) break;
	  }

	rc = l1bHdf.HdfFile::GetStatus();
	if (rc != HdfFile::OK && rc != HdfFile::NO_MORE_DATA)
	  {
	    fprintf(stderr, "%s: reading HDF %s failed before EOF is reached\n",
		    argv[0], l1b_hdf_file);
            exit(1);
	  }

	lmap.Normalize();
        lmap.Write("debug");
        lmap.WriteL2A(output_file);
	return (0);
} // main
