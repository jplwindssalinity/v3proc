//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l15_to_l17
//
// SYNOPSIS
//		l15_to_l17 <sim_config_file>
//
// DESCRIPTION
//		Simulates the SeaWinds 1b ground processing of Level 1.5 to
//		Level 1.7 data.  This program grids the sigma-0 measurments.
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
//			% l15_to_l17 sws1b.cfg
//
// ENVIRONMENT
//		Not environment dependent.
//
// EXIT STATUS
//		The following exit values are returned:
//		1	Program executed successfully
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

const char* usage_array[] = { "<sim_config_file>", 0};

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
	if (argc != 2)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* config_file = argv[clidx++];

	//---------------------//
	// read in config file //
	//---------------------//

	ConfigList config_list;
	config_list.LogErrors();
	if (! config_list.Read(config_file))
	{
		fprintf(stderr, "%s: error reading sim config file %s\n",
			command, config_file);
		exit(1);
	}

	//--------------------------------------//
	// create and configure product objects //
	//--------------------------------------//

	L15 l15;
	if (! ConfigL15(&l15, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 1 Product\n", command);
		exit(1);
	}

	Grid grid;
	if (! ConfigL17Grid(&grid, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 1.7 Grid\n", command);
		exit(1);
	}

	//-----------------//
	// conversion loop //
	//-----------------//

	do
	{
		//------------------------------//
		// read a level 1.5 data record //
		//------------------------------//

		if (! l15.ReadDataRec())
		{
			switch (l15.GetStatus())
			{
			case Product::ERROR_READING_FRAME:
				fprintf(stderr, "%s: error reading Level 1.5 data\n", command);
				exit(1);
				break;
			case Product::ERROR_UNKNOWN:
				fprintf(stderr, "%s: unknown error reading Level 1.5 data\n",
					command);
				exit(1);
				break;
			case Product::ERROR_NO_MORE_DATA:
				break;
			default:
				fprintf(stderr, "%s: unknown status (???)\n", command);
				exit(1);
			}
			break;		// done, exit do loop
		}

		//------//
		// grid //
		//------//

		if (! l15_to_l17.Convert(&l1, &l15))
		{
			fprintf(stderr, "%s: error converting Level 1 to Level 1.5\n",
				command);
			exit(1);
		}

		//-------------------------------//
		// write a level 1.5 data record //
		//-------------------------------//

		if (! l15.WriteDataRec())
		{
			fprintf(stderr, "%s: error writing Level 1.5 data\n", command);
			exit(1);
		}

	} while (1);

	l15.CloseCurrentFile();
		
	return (0);
}
