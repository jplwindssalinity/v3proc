//==========================================================//
// Copyright (C) 1997, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

//----------------------------------------------------------------------
// NAME
//		l20_metrics
//
// SYNOPSIS
//		l20_metrics [ -c config_file ] [ -l l20_file ]
//			[ -t truth_type ] [ -f truth_file ] [ -s low:high ]
//			[ -o output_base ]
//
// DESCRIPTION
//		Generates output files containing wind retrieval metrics
//		as a function of cross track distance for given ranges of
//		wind speed.
//
// OPTIONS
//		[ -c config_file ]	Use the specified config file.
//		[ -l l20_file ]		Use this l20 file.
//		[ -t truth_type ]	This is the truth file type.
//		[ -f truth_file ]	This is the truth file.
//		[ -s low:high ]		The range of wind speeds.
//		[ -o output_base ]	The base name to use for output files.
//
// OPERANDS
//		None.
//
// EXAMPLES
//		An example of a command line is:
//			% l20_metrics -c qscat.cfg -s 5.0:6.0
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
#include "ConfigList.h"
#include "Misc.h"
#include "ConfigSimDefs.h"
#include "L20.h"
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

#define OPTSTRING			"c:l:t:f:s:o:"
#define ARRAY_SIZE			1024
#define DEFAULT_LOW_SPEED	0.0
#define DEFAULT_HIGH_SPEED	30.0

//--------//
// MACROS //
//--------//

//------------------//
// TYPE DEFINITIONS //
//------------------//

//-----------------------//
// FUNCTION DECLARATIONS //
//-----------------------//

int xmgr_control(FILE* ofp, const char* title, const char* subtitle,
	const char* x_label, const char* y_label);

//------------------//
// OPTION VARIABLES //
//------------------//

//------------------//
// GLOBAL VARIABLES //
//------------------//

const char* usage_array[] = { "[ -c config_file ]", "[ -l l20_file ]",
	"[ -t truth_type ]", "[ -f truth_file ]", "[ -s low-high ]",
	"[ -o output_base ]", 0 };

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
	char* l20_file = NULL;
	char* truth_type = NULL;
	char* truth_file = NULL;
	float low_speed = DEFAULT_LOW_SPEED;
	float high_speed = DEFAULT_HIGH_SPEED;
	char* output_base = NULL;

	//------------------------//
	// parse the command line //
	//------------------------//

	const char* command = no_path(argv[0]);

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
			l20_file = optarg;
			break;
		case 't':
			truth_type = optarg;
			break;
		case 'f':
			truth_file = optarg;
			break;
		case 's':
			if (sscanf(optarg, "%f:%f", &low_speed, &high_speed) != 2)
			{
				fprintf(stderr, "%s: error determine speed range %s\n",
					command, optarg);
				exit(1);
			}
			break;
		case 'o':
			output_base = optarg;
			break;
        case '?':
            usage(command, usage_array, 1);
            break;
        }
    }

	//---------------------//
	// check for arguments //
	//---------------------//

	if (! l20_file)
	{
		l20_file = config_list.Get(L20_FILE_KEYWORD);
		if (l20_file == NULL)
		{
			fprintf(stderr, "%s: must specify L20 file\n", command);
			exit(1);
		}
	}

	if (! truth_type)
	{
		truth_type = config_list.Get(WINDFIELD_TYPE_KEYWORD);
		if (truth_type == NULL)
		{
			fprintf(stderr, "%s: must specify truth windfield type\n",
				command);
			exit(1);
		}
	}

	if (! truth_file)
	{
		truth_file = config_list.Get(WINDFIELD_FILE_KEYWORD);
		if (truth_file == NULL)
		{
			fprintf(stderr, "%s: must specify truth windfield file\n",
				command);
			exit(1);
		}
	}

	//------------------------//
	// read in level 2.0 file //
	//------------------------//

	L20 l20;
	l20.SetFilename(l20_file);
	if (! l20.OpenForReading())
	{
		fprintf(stderr, "%s: error opening L20 file %s\n", command, l20_file);
		exit(1);
	}

	if (! l20.ReadHeader())
	{
		fprintf(stderr, "%s: error reading L20 header from file %s\n",
			command, l20_file);
		exit(1);
	}

	if (! l20.ReadDataRec())
	{
		fprintf(stderr, "%s: error reading L20 swath from file %s\n",
			command, l20_file);
		exit(1);
	}

	//----------------------------//
	// read in "truth" wind field //
	//----------------------------//

	WindField truth;
	truth.ReadType(truth_file, truth_type);

	//---------------//
	// create arrays //
	//---------------//

	int cross_track_bins = l20.frame.swath.GetCrossTrackBins();
	float* ctd_array = new float[cross_track_bins];
	float* value_array = new float[cross_track_bins];
	int* count_array = new int[cross_track_bins];

	char title[1024];
	char filename[1024];
	FILE* ofp;

	//--------------------//
	// generate ctd array //
	//--------------------//

	if (! l20.frame.swath.CtdArray(l20.header.crossTrackResolution, ctd_array))
	{
		fprintf(stderr, "%s: error generating CTD array\n", command);
		exit(1);
	}

	//-------------------------//
	// rms speed error vs. ctd //
	//-------------------------//

	if (! l20.frame.swath.RmsSpdErrVsCti(&truth, value_array, count_array,
		low_speed, high_speed))
	{
		fprintf(stderr, "%s: error calculating RMS speed error\n", command);
		exit(1);
	}

	sprintf(filename, "%s.rms_spd_err", output_base);
	ofp = fopen(filename, "w");
	if (ofp == NULL)
	{
		fprintf(stderr, "%s: error opening output file %s\n", command,
			filename);
		exit(1);
	}

	sprintf(title, "RMS Speed Error vs. CTD (%g - %g m/s)", low_speed,
		high_speed);
	xmgr_control(ofp, title, l20_file, "Cross Track Distance (km)",
		"RMS Speed Error (m/s)");

	for (int i = 0; i < cross_track_bins; i++)
	{
		if (count_array[i] > 0)
		{
			fprintf(ofp, "%g %g %d\n", ctd_array[i], value_array[i],
				count_array[i]);
		}
	}
	fclose(ofp);

	//-----------------------------//
	// rms direction error vs. ctd //
	//-----------------------------//

	if (! l20.frame.swath.RmsDirErrVsCti(&truth, value_array, count_array,
		low_speed, high_speed))
	{
		fprintf(stderr, "%s: error calculating RMS direction error\n",
			command);
		exit(1);
	}

	sprintf(filename, "%s.rms_dir_err", output_base);
	ofp = fopen(filename, "w");
	if (ofp == NULL)
	{
		fprintf(stderr, "%s: error opening output file %s\n", command,
			filename);
		exit(1);
	}

	sprintf(title, "RMS Direction Error vs. CTD (%g - %g m/s)", low_speed,
		high_speed);
	xmgr_control(ofp, title, l20_file, "Cross Track Distance (km)",
		"RMS Direction Error (deg)");

	for (int i = 0; i < cross_track_bins; i++)
	{
		if (count_array[i] > 0)
		{
			fprintf(ofp, "%g %g %d\n", ctd_array[i], value_array[i] * rtd,
				count_array[i]);
		}
	}
	fclose(ofp);

	//---------------//
	// skill vs. ctd //
	//---------------//

	if (! l20.frame.swath.SkillVsCti(&truth, value_array, count_array,
		low_speed, high_speed))
	{
		fprintf(stderr, "%s: error calculating skil\n", command);
		exit(1);
	}

	sprintf(filename, "%s.skill", output_base);
	ofp = fopen(filename, "w");
	if (ofp == NULL)
	{
		fprintf(stderr, "%s: error opening output file %s\n", command,
			filename);
		exit(1);
	}

	sprintf(title, "Skill vs. CTD (%g - %g m/s)", low_speed, high_speed);
	xmgr_control(ofp, title, l20_file, "Cross Track Distance (km)", "Skill");

	for (int i = 0; i < cross_track_bins; i++)
	{
		if (count_array[i] > 0)
		{
			fprintf(ofp, "%g %g %d\n", ctd_array[i], value_array[i],
				count_array[i]);
		}
	}
	fclose(ofp);

	//-------------//
	// free arrays //
	//-------------//

	delete[] value_array;
	delete[] ctd_array;
	delete[] count_array;

	return (0);
}

//--------------//
// xmgr_control //
//--------------//

#define QUOTE	'"'

int
xmgr_control(
	FILE*			ofp,
	const char*		title,
	const char*		subtitle,
	const char*		x_label,
	const char*		y_label)
{
	fprintf(ofp, "@ title %c%s%c\n", QUOTE, title, QUOTE);
	fprintf(ofp, "@ subtitle %c%s%c\n", QUOTE, subtitle, QUOTE);
	fprintf(ofp, "@ xaxis label %c%s%c\n", QUOTE, x_label, QUOTE);
	fprintf(ofp, "@ yaxis label %c%s%c\n", QUOTE, y_label, QUOTE);
	return(1);
}
