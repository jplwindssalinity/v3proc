//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l17_to_ascii
//
// SYNOPSIS
//		l17_to_ascii <sim_config_file> <ascii_output_file>
//
// DESCRIPTION
//		Extracts data from a level 1.7 data file and writes to a column
//		oriented ascii data file.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//		<sim_config_file>		The sim_config_file needed listing
//								all input parameters, input files, and
//								output files.
//		<ascii_output_file>
//
// EXAMPLES
//		An example of a command line is:
//			% l17_to_ascii sws1b.cfg l17ascii.dat
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
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Misc.h"
#include "ConfigList.h"
#include "L17.h"
#include "ConfigSim.h"

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

const char* usage_array[] = { "<sim_config_file>", "<ascii_output_file>", 0};

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
	const char* ascii_output_file = argv[clidx++];

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

	L17 l17;
	if (! ConfigL17(&l17, &config_list))
	{
		fprintf(stderr, "%s: error configuring Level 1.7 Product\n", command);
		exit(1);
	}

	//------------//
	// open files //
	//------------//

	l17.file.OpenForInput();
	FILE* ascii_file = fopen(ascii_output_file,"w");
	if (ascii_file == NULL)
	{
		printf("Error opening %s\n",ascii_output_file);
		exit(-1);
	}

	//---------------------------------//
	// read the header to set up swath //
	//---------------------------------//

	if (! l17.ReadHeader())
	{
		fprintf(stderr, "%s: error reading Level 1.7 header\n", command); 
		exit(1);
	}

	printf("cross track resolution (km) = %g\n",
		l17.header.crossTrackResolution);
	printf("along track resolution (km) = %g\n",
		l17.header.alongTrackResolution);
	printf("cross track bins = %d\n",
		l17.header.crossTrackBins);
	printf("along track bins = %d\n",
		l17.header.alongTrackBins);
	printf("cti zero index = %d\n",
		l17.header.zeroIndex);
	printf("along track start time = %g\n",
		l17.header.startTime);

	//-------------//
	// output loop //
	//-------------//

	long count = 0;

	for (;;)
	{
		//------------------------------//
		// read a level 1.7 data record //
		//------------------------------//

		if (! l17.ReadDataRec())
		{
			switch (l17.GetStatus())
			{
			case L17::OK:		// end of file
				break;
			case L17::ERROR_READING_FRAME:
				fprintf(stderr, "%s: error reading Level 1.7 data\n", command);
				exit(1);
				break;
			case L17::ERROR_UNKNOWN:
				fprintf(stderr, "%s: unknown error reading Level 1.7 data\n",
					command);
				exit(1);
				break;
			default:
				fprintf(stderr, "%s: unknown status (???)\n", command);
				exit(1);
			}
			break;		// done, exit do loop
		}

		//--------//
		// output //
		//--------//

		int ati = l17.frame.ati;
		int cti = l17.frame.cti;
		int Nmeas = l17.frame.measList.NodeCount();
		for (Meas* meas = l17.frame.measList.GetHead();
			 meas;
			 meas = l17.frame.measList.GetNext()
			)
		{
			float sigma0_dB = 10*log(meas->value)/log(10);
			float XK_dB = 10*log(meas->XK)/log(10);
			fprintf(ascii_file,"%d %d %d %g %g %g %g %g %g %g %g %g\n",
				Nmeas,ati,cti,
				meas->value,meas->XK,meas->Pn_slice,meas->bandwidth,
				meas->eastAzimuth,
				meas->incidenceAngle,meas->A,meas->B,meas->C);
			count++;
		}
	
		if (count > 10000) break;	
	}

	l17.file.Close();
	fclose(ascii_file);

	return (0);
}
