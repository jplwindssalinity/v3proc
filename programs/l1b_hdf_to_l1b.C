//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		test_hdf_l1b.C
//
// SYNOPSIS
//		test_hdf_l1b [ -c config_file ] [ -l l1b_hdf_file ]
//			[ -o output_file ]
//
// DESCRIPTION
//		Generates output files containing ASCII output of a wind swatch
//		given a L1B HDF file.
//
// OPTIONS
//		[ -c config_file ]	Use the specified config file.
//		[ -l l1b_hdf_file ]		Use this HDF l1b file.
//		[ -o output_file ]	The name to use for output file.
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
//		Sally Chou
//		Sally.H.Chou@jpl.nasa.gov
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

#define OPTSTRING				"c:l:o:"


//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -c config_file ]", "[ -l l1b_hdf_file ]",
	"[ -o output_file ]", 0 };

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

	if (argc == 1)
		usage(command, usage_array, 1);

	int c;
	while ((c = getopt(argc, argv, OPTSTRING)) != -1)
	{
		switch(c)
		{
		case 'c':
			config_file = optarg;
			if (! config_list.Read(config_file))
			{
				fprintf(stderr, "%s: error reading config file %s\n",
					command, config_file);
				exit(1);
			}
			break;
		case 'l':
			l1b_hdf_file = optarg;
			break;
		case 'o':
            output_file = optarg;
			break;
		case '?':
			usage(command, usage_array, 1);
			break;
		}
	}


	//---------------------//
	// check for arguments //
	//---------------------//

	if (! l1b_hdf_file)
	{
		l1b_hdf_file = config_list.Get(L1B_HDF_FILE_KEYWORD);
		if (l1b_hdf_file == NULL)
		{
			fprintf(stderr, "%s: must specify HDF L1B file\n", command);
			exit(1);
		}
	}

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
    while (l1bHdf.ReadL1BHdfDataRec())
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

	return (0);

} // main
