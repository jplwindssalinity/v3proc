//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		test_hdf_l2b.C
//
// SYNOPSIS
//		test_hdf_l2b [ -c config_file ] [ -l l2b_hdf_file ]
//			[ -o output_file ]
//
// DESCRIPTION
//		Generates output files containing ASCII output of a wind swatch
//		given a L2B HDF file.
//
// OPTIONS
//		[ -c config_file ]	Use the specified config file.
//		[ -l l2b_hdf_file ]		Use this HDF l2b file.
//		[ -o output_file ]	The name to use for output file.
//
// OPERANDS
//		None.
//
// EXAMPLES
//		An example of a command line is:
//			% test_hdf_l2b -c sally.cfg -l L2B_100.file -o l2b.out
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
#include "ConfigList.h"
#include "Misc.h"
#include "ConfigSimDefs.h"
#include "L2B.h"
#include "Constants.h"
#include "List.h"
#include "List.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<EarthPosition>;
template class List<WindVectorPlus>;

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

const char* usage_array[] = { "[ -c config_file ]", "[ -l l2b_hdf_file ]",
	"[ -o output_file ]", 0 };

// not always evil...
const char*		command = NULL;
char*			l2b_hdf_file = NULL;
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
	l2b_hdf_file = NULL;
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
			l2b_hdf_file = optarg;
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

	if (! l2b_hdf_file)
	{
		l2b_hdf_file = config_list.Get(L2B_HDF_FILE_KEYWORD);
		if (l2b_hdf_file == NULL)
		{
			fprintf(stderr, "%s: must specify HDF L2B file\n", command);
			exit(1);
		}
	}

	//-----------------------//
	// read in level 2B file //
	//-----------------------//

    WindSwath swath;
    if (swath.ReadHdfL2B(l2b_hdf_file) == 0)
    {
        fprintf(stderr, "%s: cannot open HDF %s for input\n",
                               argv[0], l2b_hdf_file);
        exit(1);
    }
    if (output_file != 0)
    {
        if ((output_fp = fopen(output_file, "w")) == NULL)
        {
            fprintf(stderr, "%s: cannot open %s for output\n",
                               argv[0], output_file);
            exit(1);
        }
    }

	//-----------------------//
	// write out as ASCII    //
	//-----------------------//
    if (swath.WriteAscii(output_fp) == 0)
    {
        fprintf(stderr, "%s: cannot write ASCII output to %s\n",
                               argv[0], output_file);
        exit(1);
    }

	return (0);

} // main
