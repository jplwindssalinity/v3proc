//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		test_hdf_l2a.C
//
// SYNOPSIS
//		test_hdf_l2a [ -c config_file ] [ -l l2a_hdf_file ]
//			[ -o output_file ]
//
// DESCRIPTION
//		Generates output files containing ASCII output of a wind swatch
//		given a L2A HDF file.
//
// OPTIONS
//		[ -c config_file ]	Use the specified config file.
//		[ -l l2a_hdf_file ]		Use this HDF l2a file.
//		[ -o output_file ]	The name to use for output file.
//
// OPERANDS
//		None.
//
// EXAMPLES
//		An example of a command line is:
//			% test_hdf_l2a -c sally.cfg -l L2A_100.file -o l2a.out
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
#include <stdlib.h>
#include "BufferedList.h"
#include "BufferedList.C"
#include "List.h"
#include "List.C"
#include "Misc.h"
#include "Ephemeris.h"
#include "ConfigList.h"
#include "L1A.h"
#include "ConfigSim.h"
#include "L2AHdf.h"
#include "Tracking.h"
#include "Tracking.C"
#include "QscatConfig.h"


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
template class List<long>;
template class List<OffsetList>;
template class TrackerBase<unsigned char>;
template class TrackerBase<unsigned short>;


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

const char* usage_array[] = { "[ -c config_file ]", "[ -l l2a_hdf_file ]",
	"[ -o output_file ]", 0 };

// not always evil...
const char*		command = NULL;
char*			l2a_hdf_file = NULL;
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
	l2a_hdf_file = NULL;
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
			l2a_hdf_file = optarg;
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

	if (! l2a_hdf_file)
	{
		l2a_hdf_file = config_list.Get(L2A_HDF_FILE_KEYWORD);
		if (l2a_hdf_file == NULL)
		{
			fprintf(stderr, "%s: must specify HDF L2A file\n", command);
			exit(1);
		}
	}

	//-----------------------//
	// read in HDF 2A file   //
	//-----------------------//
    HdfFile::StatusE status;
    L2AHdf  l2aHdf(l2a_hdf_file, SOURCE_L2A, status);
    if (status != HdfFile::OK)
    {
        fprintf(stderr, "%s: cannot open HDF %s for input\n",
                               argv[0], l2a_hdf_file);
        exit(1);
    }
    fprintf(stdout, "%s: %s has %d records\n",
                               argv[0], l2a_hdf_file, l2aHdf.GetDataLength());

    
    if (output_file != 0)
    {
        if (l2aHdf.OpenForWriting(output_file) == 0)
        {
            fprintf(stderr, "%s: cannot open %s for output\n",
                               argv[0], output_file);
            exit(1);
        }
    }

	//-----------------------//
	// write out as SVT L2A  //
	//-----------------------//
    // return 1 if OK, 0 if no data, -1 if error or EOF
    int rc;
    while ((rc = l2aHdf.ReadL2AHdfCell()) != -1)
    {
        if (rc == 1 && ! l2aHdf.WriteDataRec())
        {
            fprintf(stderr, "%s: writing to %s failed.\n",
                               argv[0], output_file);
            exit(1);
        }
    }

    status = l2aHdf.HdfFile::GetStatus();
    if (status != HdfFile::OK && status != HdfFile::NO_MORE_DATA)
    {
        fprintf(stderr, "%s: reading HDF %s failed before EOF is reached\n",
                           argv[0], l2a_hdf_file);
            exit(1);
    }

	return (0);

} // main
