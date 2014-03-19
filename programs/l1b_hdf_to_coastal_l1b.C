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
//		given a L1B HDF file.
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
#include "QscatConfig.h"
#include "CoastalMaps.h"
#include "List.h"
#include "BufferedList.h"


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

        lmap.InitExtraMaps(35.5*dtr,-124.5*dtr, dtr/30.0, 5.0*dtr,5.0*dtr);

        //lmap.ReadPrecomputeLands0("debug_foreaft");

	//--------------------------------------------
	// configure Antenna objects
	//--------------------------------------------	
        Antenna ant;
	if ( ! ConfigAntenna(&ant,&config_list))
	  {
	    fprintf(stderr, "%s: config Antenna failed\n", argv[0]);
	    exit(1);
	  }

	
	if (output_file != 0)
	  {
	    if (l1bHdf.OpenForWriting(output_file) == 0)
	      {
		fprintf(stderr, "%s: cannot open %s for output\n",
			argv[0], output_file);
		exit(1);
	      }
	  }

	//-----------------------//
	// write out as SVT L1B  //
	//-----------------------//
	while (l1bHdf.ReadL1BHdfDataRecCoastal(&lmap,&ant))
	  {
	    if ( ! l1bHdf.WriteDataRec())
	      {
		fprintf(stderr, "%s: writing to %s failed.\n",
			argv[0], output_file);
		exit(1);
	      }
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
	return (0);

} // main
