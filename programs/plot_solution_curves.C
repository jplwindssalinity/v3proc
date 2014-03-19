//==============================================================//
// Copyright (C) 1997-1998, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.					//
//==============================================================//

//----------------------------------------------------------------------
// NAME
//		plot_solution_curves
//
// SYNOPSIS
//		plot_solution_curves <sim_config_file> <output_base>
//
// DESCRIPTION
//		Plots the solution curves for a given set of measurements.
//		Input values are prompted from standard input and are
//		polarization (1=V), incidence angle (deg), azimuth angle (deg),
//			and sigma-0 (dB)
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
//		<output_base>			The base for the output filenames.
//
// EXAMPLES
//		An example of a command line is:
//			% plot_solution_curves sws1b.cfg solcur
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
#include "Misc.h"
#include "ConfigList.h"
#include "GMF.h"
#include "ConfigSim.h"
#include "List.h"
#include "BufferedList.h"
#include "Tracking.h"

//-----------//
// TEMPLATES //
//-----------//

// Class declarations needed for templates
// eliminates need to include the entire header file
class AngleInterval;

template class List<AngleInterval>;
template class List<EarthPosition>;
template class List<StringPair>;
template class List<WindVectorPlus>;
template class List<Meas>;
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

const char* usage_array[] = { "<sim_config_file>", "<output_base>", 0};

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
	const char* output_base = argv[clidx++];

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
	// read the geophysical model function //
	//-------------------------------------//
 
	GMF gmf;
	if (! ConfigGMF(&gmf, &config_list))
	{
		fprintf(stderr, "%s: error configuring GMF\n", command);
		exit(1);
	}

	//--------------//
	// configure Kp //
	//--------------//

	// Kp is NOT USED because the data needed is not available in this program.

	Kp kp;
	if (! ConfigKp(&kp, &config_list))
	{
		fprintf(stderr, "%s: error configuring Kp\n", command);
		exit(1);
	}

	//------//
	// loop //
	//------//

	MeasList meas_list;
	char filename[1024];
	char line[1024];
	char polchar;
	float inc, azi, s0;
	for (int file_idx = 1; ; file_idx++)
	{
		meas_list.FreeContents();

		sprintf(filename, "%s.%d", output_base, file_idx);
		printf("\nEnter information for output file %s\n", filename);
		for (int meas_idx = 1; ; meas_idx++)
		{
			if (fgets(line, 1024, stdin) != line)
				break;

			if (line[0]=='#') continue; // skip comments
			if (sscanf(line, " %c %f %f %f", &polchar, &inc, &azi, &s0) != 4)
			{
				if (meas_idx == 1)
					return(0);
				else
					break;
			}

			Meas* new_meas = new Meas();
			if (polchar == 'V' || polchar == 'v')
				new_meas->measType = Meas::VV_MEAS_TYPE;
			else if (polchar == 'H' || polchar == 'h')
				new_meas->measType = Meas::HH_MEAS_TYPE;
			new_meas->incidenceAngle = inc * dtr;
			new_meas->eastAzimuth = azi * dtr;
			new_meas->value = s0;

			//-----------------//
			// add measurement //
			//-----------------//

			meas_list.Append(new_meas);
		}

		printf("Writing to file %s\n", filename);

		//------------------//
		// open output file //
		//------------------//

		FILE* ofp = fopen(filename, "w");
		if (ofp == NULL)
		{
			fprintf(stderr, "%s: error opening output file %s\n", command,
				filename);
			exit(1);
		}

		//-----------------------//
		// write solution curves //
		//-----------------------//

		gmf.WriteSolutionCurves(ofp, &meas_list, NULL);

		//-------------------//
		// close output file //
		//-------------------//

		fclose(ofp);
	}

	return (0);
}
