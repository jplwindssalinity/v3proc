//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l17_diagnositics
//
// SYNOPSIS
//		l17_diagnositics <cfg file> <l17_file> <output_file_base_name>
//
// DESCRIPTION
//		Reads in a Level 1.7 file and writes out diagnositic data files
//		for use in geo,heo etc.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//		<cfg file>		The simulation configuration file.
//		<l17_file>		The Level 1.7 input file.
//		<output_file_base_name>	Output file name to append suffixes to.
//
// EXAMPLES
//		An example of a command line is:
//			% l17_diagnostics qscat.cfg l17.dat l17
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
#include "Misc.h"
#include "L17.h"
#include "ConfigList.h"
#include "Ephemeris.h"
#include "ConfigSim.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"

//-----------//
// TEMPLATES //
//-----------//

template class List<StringPair>;
template class List<EarthPosition>;
template class List<Meas>;
template class List<MeasSpot>;
template class BufferedList<OrbitState>;
template class List<OrbitState>;
template class List<long>;
template class List<OffsetList>;
template class List<WindVectorPlus>;

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

const char* usage_array[] = { "<cfg_file>", "<l17_file>",
	"<output_file_base>", 0};

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
	if (argc != 4)
		usage(command, usage_array, 1);

	int clidx = 1;
	const char* config_file = argv[clidx++];
	const char* l17_file = argv[clidx++];
	const char* output_base = argv[clidx++];

	//--------------------------------//
	// read in simulation config file //
	//--------------------------------//

	ConfigList config_list;
	if (! config_list.Read(config_file))
	{
		fprintf(stderr, "%s: error reading sim config file %s\n",
			command, config_file);
		exit(1);
	}

	//-------------------------//
	// open the Level 1.7 file //
	//-------------------------//

	L17 l17;
	if (! l17.OpenForReading(l17_file))
	{
		fprintf(stderr, "%s: error opening Level 1.7 file %s\n", command,
			l17_file);
		exit(1);
	}

	//--------------------------------//
	// create and configure ephemeris //
	//--------------------------------//

	Ephemeris ephemeris;
	if (! ConfigEphemeris(&ephemeris, &config_list))
	{
		fprintf(stderr, "%s: error configuring ephemeris\n", command);
		exit(1);
	}

	//-------------------//
	// open output files //
	//-------------------//

    char filename[1024];
    sprintf(filename, "%s.%s", output_base, "grid");
    FILE* ofp_grid = fopen(filename, "w");
    if (ofp_grid == NULL)
    {
        fprintf(stderr, "%s: error opening output file %s\n", command,
            filename);
        exit(1);
    }

    char* hdr = OTLN_HEADER;
    if (fwrite((void *)hdr, 4, 1, ofp_grid) != 1)
    {
        fprintf(stderr, "%s: error writing header to output file %s\n",
            command, filename);
        exit(1);
    }


	//----------------//
	// loop and write //
	//----------------//

	Outline outline;
	double ctd,atd;
	int ii[4] = {0,0,1,1};
	int jj[4] = {0,1,1,0};

	while (l17.ReadDataRec())
	{
		//----------------------------------------------------------//
		// The 4 corners of this grid square in ctd,atd coordinates.
		//----------------------------------------------------------//

		outline.FreeContents();
		for (int k=0; k < 4; k++)
		{
			ctd = (l17.frame.cti - l17.header.zeroIndex + 0.5 + ii[k]) *
				l17.header.crossTrackResolution;
			atd = (l17.frame.ati + jj[k]) * l17.header.alongTrackResolution;
			EarthPosition* r = new EarthPosition;
			ephemeris.GetSubtrackPosition(ctd,atd,l17.header.startTime,r);
			outline.Append(r);
		}

		outline.WriteOtln(ofp_grid);

//		MeasList* ml = &(l17.frame.measList);
//		for (Meas* m = ml->GetHead(); m; m = ml->GetNext())
	}

	//-----------------//
	// close the files //
	//-----------------//

	fclose(ofp_grid);
	l17.Close();

	return (0);
}
