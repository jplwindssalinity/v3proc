//==========================================================//
// Copyright (C) 1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l2a_diagnositics
//
// SYNOPSIS
//		l2a_diagnositics <cfg file> <l2A_file> <output_file_base_name>
//
// DESCRIPTION
//		Reads in a Level 2a file and writes out diagnositic data files
//		for use in geo,heo etc.
//
// OPTIONS
//		None.
//
// OPERANDS
//		The following operand is supported:
//		<cfg file>		The simulation configuration file.
//		<l2A_file>		The Level 2a input file.
//		<output_file_base_name>	Output file name to append suffixes to.
//
// EXAMPLES
//		An example of a command line is:
//			% l2a_diagnostics qscat.cfg l2A.dat l2A
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
#include "L2A.h"
#include "ConfigList.h"
#include "Ephemeris.h"
#include "ConfigSim.h"
#include "List.h"
#include "List.C"
#include "BufferedList.h"
#include "BufferedList.C"
#include "Tracking.h"
#include "Tracking.C"

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

const char* usage_array[] = { "<cfg_file>", "<l2A_file>",
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
	const char* l2a_file = argv[clidx++];
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
	// open the Level 2a file //
	//-------------------------//

	L2A l2a;
	if (! l2a.OpenForReading(l2a_file))
	{
		fprintf(stderr, "%s: error opening Level 2a file %s\n", command,
			l2a_file);
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
	double sum_c_diff = 0.0;
	double sum_a_diff = 0.0;
	long count = 0;
	int ii[4] = {0,0,1,1};
	int jj[4] = {0,1,1,0};

	while (l2a.ReadDataRec())
	{
		EarthPosition start_position;
		ephemeris.GetPosition(l2a.header.startTime,EPHEMERIS_INTERP_ORDER,
			&start_position);
		start_position = start_position.Nadir();

		MeasList* ml = &(l2a.frame.measList);
		int N = ml->NodeCount();
		float sum_sigma0 = 0.0;
		for (Meas* m = ml->GetHead(); m; m = ml->GetNext())
		{
			sum_sigma0 += m->value;
		}
		
		//----------------------------------------------------------//
		// The 4 corners of this grid square in ctd,atd coordinates.
		//----------------------------------------------------------//

		outline.FreeContents();
		EarthPosition *r;
		for (int k=0; k < 4; k++)
		{
			ctd = (l2a.frame.cti - l2a.header.zeroIndex - 0.5 + ii[k]) *
				l2a.header.crossTrackResolution;
			atd = (l2a.frame.ati + jj[k]) * l2a.header.alongTrackResolution;
			r = new EarthPosition;
			ephemeris.GetSubtrackPosition(ctd,atd,l2a.header.startTime,r);
			outline.Append(r);
/*
			float cd,ad;
			ephemeris.GetSubtrackCoordinates(*r,start_position,
				l2a.header.startTime,l2a.header.startTime,&cd,&ad);
			printf("%g %g %g %g\n",ctd,atd,ctd-cd,atd-ad);
*/
		}

		float cd,ad;
		ephemeris.GetSubtrackCoordinates(*r,start_position,
			l2a.header.startTime,l2a.header.startTime+atd/6.5,&cd,&ad);
		sum_c_diff += fabs(ctd - cd);
		sum_a_diff += fabs(atd - ad);
		count++;
		printf("%g %g %g %g\n",ctd,atd,ctd-cd,atd-ad);

		outline.WriteOtln(ofp_grid,sum_sigma0/N);

	}

	printf("Average absolute position error (km): ct: %g at: %g\n",
		sum_c_diff/count,sum_a_diff/count);

	//-----------------//
	// close the files //
	//-----------------//

	fclose(ofp_grid);
	l2a.Close();

	return (0);
}
